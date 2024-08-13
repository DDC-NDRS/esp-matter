// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <esp_log.h>
#include <esp_matter_controller_client.h>
#include <esp_matter_controller_pairing_command.h>

static const char *TAG = "pairing_command";

using namespace chip;
using namespace chip::Controller;

namespace esp_matter {
namespace controller {

void pairing_command::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status) {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ESP_LOGI(TAG, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ESP_LOGI(TAG, "Secure Pairing Failed");
        break;
    }
}

void pairing_command::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR) {
        ESP_LOGI(TAG, "Pairing Success");
    } else {
        ESP_LOGI(TAG, "Pairing Failure: Matter-%s", ErrorStr(err));
    }
}

void pairing_command::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR) {
        ESP_LOGI(TAG, "Pairing Deleted Success");
    } else {
        ESP_LOGI(TAG, "Pairing Deleted Failure: Matter-%s", ErrorStr(err));
    }
}

void pairing_command::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR) {
        ESP_LOGI(TAG, "Device commissioning completed with success - getting OperationalDeviceProxy");
        auto &controller_instance = esp_matter::controller::matter_controller_client::get_instance();
        controller_instance.get_commissioner()->GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback,
                                                                   &mOnDeviceConnectionFailureCallback);
    } else {
        ESP_LOGI(TAG, "Device commissioning Failure: Matter%s", ErrorStr(err));
    }
}

void pairing_command::OnDeviceConnectedFn(void *context, ExchangeManager &exchangeMgr,
                                          const SessionHandle &sessionHandle)
{
    ESP_LOGI(TAG, "OnDeviceConnectedFn");
}

void pairing_command::OnDeviceConnectionFailureFn(void *context, const ScopedNodeId &peerId, CHIP_ERROR err)
{
    ESP_LOGI(TAG, "OnDeviceConnectionFailureFn - attempt to get OperationalDeviceProxy failed");
}

CommissioningParameters pairing_command::get_commissioning_params()
{
    switch (m_pairing_network_type) {
    case NETWORK_TYPE_ETHERNET:
    case NETWORK_TYPE_NONE:
        return CommissioningParameters();
        break;
    default:
        ESP_LOGE(TAG, "Unsuppoted pairing network type");
        break;
    }
    return CommissioningParameters();
}

void pairing_command::OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData &nodeData)
{
    auto &controller_instance = esp_matter::controller::matter_controller_client::get_instance();
    // Ignore nodes with closed comissioning window
    VerifyOrReturn(nodeData.commissioningMode != 0);
    const uint16_t port = nodeData.port;
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    nodeData.ipAddress[0].ToString(buf);
    ESP_LOGI(TAG, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery. TODO: Check whether it is a right method
    controller_instance.get_commissioner()->RegisterDeviceDiscoveryDelegate(nullptr);

    Inet::InterfaceId interfaceId = nodeData.ipAddress[0].IsIPv6LinkLocal()
        ? nodeData.interfaceId
        : Inet::InterfaceId::Null();
    PeerAddress peerAddress = PeerAddress::UDP(nodeData.ipAddress[0], port, interfaceId);
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(m_setup_pincode).SetPeerAddress(peerAddress);
    CommissioningParameters commissioning_params = get_commissioning_params();
    controller_instance.get_commissioner()->PairDevice(m_remote_node_id, params, commissioning_params);
}

esp_err_t pairing_on_network(NodeId node_id, uint32_t pincode)
{
    Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kNone);
    auto &controller_instance = esp_matter::controller::matter_controller_client::get_instance();
    controller_instance.get_commissioner()->RegisterDeviceDiscoveryDelegate(&pairing_command::get_instance());
    pairing_command::get_instance().m_pairing_mode = PAIRING_MODE_ONNETWORK;
    pairing_command::get_instance().m_setup_pincode = pincode;
    pairing_command::get_instance().m_remote_node_id = node_id;
    pairing_command::get_instance().m_pairing_network_type = NETWORK_TYPE_NONE;
    if (CHIP_NO_ERROR != controller_instance.get_commissioner()->DiscoverCommissionableNodes(filter)) {
        ESP_LOGE(TAG, "Failed to discover commissionable nodes");
        return ESP_FAIL;
    }
    return ESP_OK;
}

#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
esp_err_t pairing_ble_wifi(NodeId node_id, uint32_t pincode, uint16_t disc, const char *ssid, const char *pwd)
{
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(pincode).SetDiscriminator(disc).SetPeerAddress(
        chip::Transport::PeerAddress::BLE());

    chip::ByteSpan nameSpan(reinterpret_cast<const uint8_t *>(ssid), strlen(ssid));
    chip::ByteSpan pwdSpan(reinterpret_cast<const uint8_t *>(pwd), strlen(pwd));
    CommissioningParameters commissioning_params =
        CommissioningParameters().SetWiFiCredentials(Controller::WiFiCredentials(nameSpan, pwdSpan));
    auto &controller_instance = esp_matter::controller::matter_controller_client::get_instance();
    controller_instance.get_commissioner()->PairDevice(node_id, params, commissioning_params);
    return ESP_OK;
}

esp_err_t pairing_ble_thread(NodeId node_id, uint32_t pincode, uint16_t disc, uint8_t *dataset_tlvs,
                             uint8_t dataset_len)
{
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(pincode).SetDiscriminator(disc).SetPeerAddress(
        chip::Transport::PeerAddress::BLE());

    chip::ByteSpan dataset_span(dataset_tlvs, dataset_len);
    CommissioningParameters commissioning_params = CommissioningParameters().SetThreadOperationalDataset(dataset_span);
    auto &controller_instance = esp_matter::controller::matter_controller_client::get_instance();
    controller_instance.get_commissioner()->PairDevice(node_id, params, commissioning_params);
    return ESP_OK;
}
#endif
} // namespace controller
} // namespace esp_matter
