// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
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

/**
 * Since we will not use ember data model anymore, but the upstream code still uses ember APIs so we define
 * these stub functions to make upstream code access our esp_matter data model instead of ember data model.
 */

#include <esp_matter_attribute_utils.h>
#include <esp_matter_data_model.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <app/util/AttributesChangedListener.h>
#include <app/util/MarkAttributeDirty.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/binding-table.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

using chip::Protocols::InteractionModel::Status;

namespace {

esp_matter::endpoint_t *get_endpoint_at_index(uint16_t index)
{
    esp_matter::endpoint_t *ep = esp_matter::endpoint::get_first(esp_matter::node::get());
    uint16_t idx = 0;
    while (idx < index && ep) {
        ep = esp_matter::endpoint::get_next(ep);
        idx++;
    }
    return ep;
}

Status get_raw_data_buffer_from_attr_val(const esp_matter_attr_val_t &val, uint8_t *dataPtr, uint16_t readLength)
{
    switch (val.type) {
    case ESP_MATTER_VAL_TYPE_BOOLEAN:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BOOLEAN: {
        if (readLength < sizeof(bool) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<bool>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(*(uint8_t *)(&(val.val.b)))) {
            Traits::SetNull(*(uint8_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.b, *dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_FLOAT:
    case ESP_MATTER_VAL_TYPE_NULLABLE_FLOAT: {
        if (readLength < sizeof(float) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<float>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.f)) {
            Traits::SetNull(*(float *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.f, *(float *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_OCTET_STRING:
    case ESP_MATTER_VAL_TYPE_CHAR_STRING: {
        if (readLength < val.val.a.t || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        uint8_t len = val.val.a.s;
        memcpy(dataPtr, &len, sizeof(len));
        // UINT8_MAX is reserved for null value
        if (len < UINT8_MAX) {
            memcpy(dataPtr + sizeof(len), val.val.a.b, len);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_LONG_OCTET_STRING:
    case ESP_MATTER_VAL_TYPE_LONG_CHAR_STRING: {
        if (readLength < val.val.a.t || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        memcpy(dataPtr, &val.val.a.s, sizeof(val.val.a.s));
        // UINT16_MAX is reserved for null value
        if (val.val.a.s < UINT16_MAX) {
            memcpy(dataPtr + sizeof(val.val.a.s), val.val.a.b, val.val.a.s);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_INT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT8: {
        if (readLength < sizeof(int8_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<int8_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.i8)) {
            Traits::SetNull(*(int8_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.i8, *(int8_t *)dataPtr);
        }
        break;
    }
    case ESP_MATTER_VAL_TYPE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_ENUM8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM8:
    case ESP_MATTER_VAL_TYPE_UINT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT8: {
        if (readLength < sizeof(uint8_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<uint8_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.u8)) {
            Traits::SetNull(*dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.u8, *dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_INT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT16: {
        if (readLength < sizeof(int16_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<int16_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.i16)) {
            Traits::SetNull(*(int16_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.i16, *(int16_t *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_ENUM16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM16:
    case ESP_MATTER_VAL_TYPE_UINT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT16: {
        if (readLength < sizeof(uint16_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<uint16_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.u16)) {
            Traits::SetNull(*(uint16_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.u16, *(uint16_t *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_INT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT32: {
        if (readLength < sizeof(int32_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<int32_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.i32)) {
            Traits::SetNull(*(int32_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.i32, *(int32_t *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_UINT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT32: {
        if (readLength < sizeof(uint32_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<uint32_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.u32)) {
            Traits::SetNull(*(uint32_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.u32, *(uint32_t *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_INT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT64: {
        if (readLength < sizeof(int64_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<int64_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.i64)) {
            Traits::SetNull(*(int64_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.i64, *(int64_t *)dataPtr);
        }
        break;
    }

    case ESP_MATTER_VAL_TYPE_UINT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT64: {
        if (readLength < sizeof(uint8_t) || !dataPtr) {
            return chip::Protocols::InteractionModel::Status::ResourceExhausted;
        }
        using Traits = chip::app::NumericAttributeTraits<uint64_t>;
        if ((val.type & ESP_MATTER_VAL_NULLABLE_BASE) && Traits::IsNullValue(val.val.u64)) {
            Traits::SetNull(*(uint64_t *)dataPtr);
        } else {
            Traits::WorkingToStorage(val.val.u64, *(uint64_t *)dataPtr);
        }
        break;
    }

    default:
        return chip::Protocols::InteractionModel::Status::InvalidDataType;
    }
    return chip::Protocols::InteractionModel::Status::Success;
}

Status get_attr_val_from_raw_data_buffer(uint8_t *value, EmberAfAttributeType dataType, esp_matter_attr_val_t &val,
                                         bool is_nullable)
{
    switch (dataType) {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<bool>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        val = esp_matter_bool(attribute_value);
        break;
    }

    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE: {
        uint8_t data_count = 0;
        memcpy(&data_count, &value[0], sizeof(uint8_t));
        if (is_nullable && data_count == UINT8_MAX) {
            val = esp_matter_char_str(nullptr, data_count);
        } else if (data_count < UINT8_MAX) {
            val = esp_matter_char_str((char *)(value + sizeof(uint8_t)), data_count);
        }
        break;
    }

    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE: {
        uint16_t data_count = 0;
        memcpy(&data_count, &value[0], sizeof(uint16_t));
        if (is_nullable && data_count == UINT16_MAX) {
            val = esp_matter_char_str(nullptr, data_count);
        } else if (data_count < UINT16_MAX) {
            val = esp_matter_long_char_str((char *)(value + sizeof(uint16_t)), data_count);
        }
        break;
    }

    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_IPADR_ATTRIBUTE_TYPE:
    case ZCL_IPV4ADR_ATTRIBUTE_TYPE:
    case ZCL_IPV6ADR_ATTRIBUTE_TYPE:
    case ZCL_IPV6PRE_ATTRIBUTE_TYPE:
    case ZCL_HWADR_ATTRIBUTE_TYPE: {
        uint8_t data_count = 0;
        memcpy(&data_count, &value[0], sizeof(uint8_t));
        if (is_nullable && data_count == UINT8_MAX) {
            val = esp_matter_char_str(nullptr, data_count);
        } else if (data_count < UINT8_MAX) {
            val = esp_matter_octet_str((value + sizeof(uint8_t)), data_count);
        }
        break;
    }

    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: {
        uint16_t data_count = 0;
        memcpy(&data_count, &value[0], sizeof(uint16_t));
        if (is_nullable && data_count == UINT16_MAX) {
            val = esp_matter_char_str(nullptr, data_count);
        } else if (data_count < UINT16_MAX) {
            val = esp_matter_long_octet_str((value + sizeof(uint16_t)), data_count);
        }
        break;
    }

    case ZCL_INT8S_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<int8_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_int8(nullable<int8_t>());
            } else {
                val = esp_matter_nullable_int8(attribute_value);
            }
        } else {
            val = esp_matter_int8(attribute_value);
        }
        break;
    }

    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_ACTION_ID_ATTRIBUTE_TYPE:
    case ZCL_TAG_ATTRIBUTE_TYPE:
    case ZCL_NAMESPACE_ATTRIBUTE_TYPE:
    case ZCL_FABRIC_IDX_ATTRIBUTE_TYPE:
    case ZCL_PERCENT_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint8_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_uint8(nullable<uint8_t>());
            } else {
                val = esp_matter_nullable_uint8(attribute_value);
            }
        } else {
            val = esp_matter_uint8(attribute_value);
        }
        break;
    }

    case ZCL_INT16S_ATTRIBUTE_TYPE:
    case ZCL_TEMPERATURE_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<int16_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_int16(nullable<int16_t>());
            } else {
                val = esp_matter_nullable_int16(attribute_value);
            }
        } else {
            val = esp_matter_int16(attribute_value);
        }
        break;
    }

    case ZCL_INT16U_ATTRIBUTE_TYPE:
    case ZCL_ENTRY_IDX_ATTRIBUTE_TYPE:
    case ZCL_GROUP_ID_ATTRIBUTE_TYPE:
    case ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE:
    case ZCL_VENDOR_ID_ATTRIBUTE_TYPE:
    case ZCL_PERCENT100THS_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint16_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_uint16(nullable<uint16_t>());
            } else {
                val = esp_matter_nullable_uint16(attribute_value);
            }
        } else {
            val = esp_matter_uint16(attribute_value);
        }
        break;
    }

    case ZCL_INT32S_ATTRIBUTE_TYPE:
    case ZCL_INT24S_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<int32_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_int32(nullable<int32_t>());
            } else {
                val = esp_matter_nullable_int32(attribute_value);
            }
        } else {
            val = esp_matter_int32(attribute_value);
        }
        break;
    }

    case ZCL_INT32U_ATTRIBUTE_TYPE:
    case ZCL_TRANS_ID_ATTRIBUTE_TYPE:
    case ZCL_CLUSTER_ID_ATTRIBUTE_TYPE:
    case ZCL_ATTRIB_ID_ATTRIBUTE_TYPE:
    case ZCL_FIELD_ID_ATTRIBUTE_TYPE:
    case ZCL_EVENT_ID_ATTRIBUTE_TYPE:
    case ZCL_COMMAND_ID_ATTRIBUTE_TYPE:
    case ZCL_EPOCH_S_ATTRIBUTE_TYPE:
    case ZCL_ELAPSED_S_ATTRIBUTE_TYPE:
    case ZCL_DATA_VER_ATTRIBUTE_TYPE:
    case ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE:
    case ZCL_INT24U_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint32_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_uint32(nullable<uint32_t>());
            } else {
                val = esp_matter_nullable_uint32(attribute_value);
            }
        } else {
            val = esp_matter_uint32(attribute_value);
        }
        break;
    }

    case ZCL_INT64S_ATTRIBUTE_TYPE:
    case ZCL_ENERGY_MWH_ATTRIBUTE_TYPE:
    case ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE:
    case ZCL_POWER_MW_ATTRIBUTE_TYPE:
    case ZCL_INT56S_ATTRIBUTE_TYPE:
    case ZCL_INT48S_ATTRIBUTE_TYPE:
    case ZCL_INT40S_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<int64_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_int64(nullable<int64_t>());
            } else {
                val = esp_matter_nullable_int64(attribute_value);
            }
        } else {
            val = esp_matter_int64(attribute_value);
        }
        break;
    }

    case ZCL_INT64U_ATTRIBUTE_TYPE:
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE:
    case ZCL_NODE_ID_ATTRIBUTE_TYPE:
    case ZCL_POSIX_MS_ATTRIBUTE_TYPE:
    case ZCL_EPOCH_US_ATTRIBUTE_TYPE:
    case ZCL_SYSTIME_US_ATTRIBUTE_TYPE:
    case ZCL_SYSTIME_MS_ATTRIBUTE_TYPE:
    case ZCL_EVENT_NO_ATTRIBUTE_TYPE:
    case ZCL_INT56U_ATTRIBUTE_TYPE:
    case ZCL_INT48U_ATTRIBUTE_TYPE:
    case ZCL_INT40U_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint64_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_uint64(nullable<uint64_t>());
            } else {
                val = esp_matter_nullable_uint64(attribute_value);
            }
        } else {
            val = esp_matter_uint64(attribute_value);
        }
        break;
    }

    case ZCL_ENUM8_ATTRIBUTE_TYPE:
    case ZCL_STATUS_ATTRIBUTE_TYPE:
    case ZCL_PRIORITY_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint8_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_enum8(nullable<uint8_t>());
            } else {
                val = esp_matter_nullable_enum8(attribute_value);
            }
        } else {
            val = esp_matter_enum8(attribute_value);
        }
        break;
    }

    case ZCL_ENUM16_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint16_t>;
        Traits::StorageType attribute_value;
        memcpy((uint16_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_enum16(nullable<uint16_t>());
            } else {
                val = esp_matter_nullable_enum16(attribute_value);
            }
        } else {
            val = esp_matter_enum16(attribute_value);
        }
        break;
    }

    case ZCL_BITMAP8_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint8_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_bitmap8(nullable<uint8_t>());
            } else {
                val = esp_matter_nullable_bitmap8(attribute_value);
            }
        } else {
            val = esp_matter_bitmap8(attribute_value);
        }
        break;
    }

    case ZCL_BITMAP16_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint16_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_bitmap16(nullable<uint16_t>());
            } else {
                val = esp_matter_nullable_bitmap16(attribute_value);
            }
        } else {
            val = esp_matter_bitmap16(attribute_value);
        }
        break;
    }

    case ZCL_BITMAP32_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<uint32_t>;
        Traits::StorageType attribute_value;
        memcpy((uint8_t *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_bitmap32(nullable<uint32_t>());
            } else {
                val = esp_matter_nullable_bitmap32(attribute_value);
            }
        } else {
            val = esp_matter_bitmap32(attribute_value);
        }
        break;
    }

    case ZCL_SINGLE_ATTRIBUTE_TYPE: {
        using Traits = chip::app::NumericAttributeTraits<float>;
        Traits::StorageType attribute_value;
        memcpy((float *)&attribute_value, value, sizeof(Traits::StorageType));
        if (is_nullable) {
            if (Traits::IsNullValue(attribute_value)) {
                val = esp_matter_nullable_float(nullable<float>());
            } else {
                val = esp_matter_nullable_float(attribute_value);
            }
        } else {
            val = esp_matter_float(attribute_value);
        }
        break;
    }

    default:
        val = esp_matter_invalid(NULL);
        break;
    }
    if (val.type == ESP_MATTER_VAL_TYPE_INVALID) {
        return Status::InvalidDataType;
    }
    return Status::Success;
}

EmberAfAttributeType get_ember_attr_type_from_val_type(esp_matter_val_type_t val_type)
{
    switch (val_type) {
    case ESP_MATTER_VAL_TYPE_BOOLEAN:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BOOLEAN:
        return ZCL_BOOLEAN_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_FLOAT:
    case ESP_MATTER_VAL_TYPE_NULLABLE_FLOAT:
        return ZCL_SINGLE_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_OCTET_STRING:
        return ZCL_OCTET_STRING_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_CHAR_STRING:
        return ZCL_CHAR_STRING_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_LONG_OCTET_STRING:
        return ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_LONG_CHAR_STRING:
        return ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_INT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT8:
        return ZCL_INT8S_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP8:
        return ZCL_BITMAP8_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_ENUM8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM8:
        return ZCL_ENUM8_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_UINT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT8:
        return ZCL_INT8U_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_INT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT16:
        return ZCL_INT16S_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP16:
        return ZCL_BITMAP16_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_ENUM16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM16:
        return ZCL_ENUM16_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_UINT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT16:
        return ZCL_INT16U_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_INT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT32:
        return ZCL_INT32S_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP32:
        return ZCL_BITMAP32_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_UINT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT32:
        return ZCL_INT32U_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_INT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT64:
        return ZCL_INT64S_ATTRIBUTE_TYPE;
    case ESP_MATTER_VAL_TYPE_UINT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT64:
        return ZCL_INT64U_ATTRIBUTE_TYPE;
    default:
        break;
    }
    return ZCL_NO_DATA_ATTRIBUTE_TYPE;
}

uint16_t get_ember_attr_size_from_val(const esp_matter_attr_val_t &val)
{
    switch (val.type) {
    case ESP_MATTER_VAL_TYPE_BOOLEAN:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BOOLEAN:
    case ESP_MATTER_VAL_TYPE_INT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT8:
    case ESP_MATTER_VAL_TYPE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_ENUM8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM8:
    case ESP_MATTER_VAL_TYPE_UINT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT8:
        return 1;
    case ESP_MATTER_VAL_TYPE_INT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT16:
    case ESP_MATTER_VAL_TYPE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_ENUM16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM16:
    case ESP_MATTER_VAL_TYPE_UINT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT16:
        return 2;
    case ESP_MATTER_VAL_TYPE_INT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT32:
    case ESP_MATTER_VAL_TYPE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_UINT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT32:
        return 4;
    case ESP_MATTER_VAL_TYPE_INT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT64:
    case ESP_MATTER_VAL_TYPE_UINT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT64:
        return 8;

    case ESP_MATTER_VAL_TYPE_FLOAT:
    case ESP_MATTER_VAL_TYPE_NULLABLE_FLOAT:
        return sizeof(float);
    case ESP_MATTER_VAL_TYPE_OCTET_STRING:
    case ESP_MATTER_VAL_TYPE_CHAR_STRING:
        return (val.val.a.s == UINT8_MAX ? 0 : val.val.a.s) + 1;
    case ESP_MATTER_VAL_TYPE_LONG_OCTET_STRING:
    case ESP_MATTER_VAL_TYPE_LONG_CHAR_STRING:
        return (val.val.a.s == UINT16_MAX ? 0 : val.val.a.s) + 1;
    default:
        break;
    }
    return 0;
}

EmberAfDefaultAttributeValue get_default_attr_value_from_val(esp_matter_attr_val_t *val)
{
    switch (val->type) {
    case ESP_MATTER_VAL_TYPE_BOOLEAN:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BOOLEAN:
        return EmberAfDefaultAttributeValue((uint16_t)val->val.b);
    case ESP_MATTER_VAL_TYPE_FLOAT:
    case ESP_MATTER_VAL_TYPE_NULLABLE_FLOAT:
        return EmberAfDefaultAttributeValue((uint8_t *)&val->val.f);
    case ESP_MATTER_VAL_TYPE_INT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT8:
        return EmberAfDefaultAttributeValue((uint16_t)val->val.i8);
    case ESP_MATTER_VAL_TYPE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP8:
    case ESP_MATTER_VAL_TYPE_ENUM8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM8:
    case ESP_MATTER_VAL_TYPE_UINT8:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT8:
        return EmberAfDefaultAttributeValue((uint16_t)val->val.u8);
    case ESP_MATTER_VAL_TYPE_INT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT16:
        return EmberAfDefaultAttributeValue((uint16_t)val->val.i16);
    case ESP_MATTER_VAL_TYPE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP16:
    case ESP_MATTER_VAL_TYPE_ENUM16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_ENUM16:
    case ESP_MATTER_VAL_TYPE_UINT16:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT16:
        return EmberAfDefaultAttributeValue((uint16_t)val->val.u16);
    case ESP_MATTER_VAL_TYPE_INT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT32:
        return EmberAfDefaultAttributeValue((uint8_t *)&val->val.i32);
    case ESP_MATTER_VAL_TYPE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP32:
    case ESP_MATTER_VAL_TYPE_UINT32:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT32:
        return EmberAfDefaultAttributeValue((uint8_t *)&val->val.u32);
    case ESP_MATTER_VAL_TYPE_INT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_INT64:
        return EmberAfDefaultAttributeValue((uint8_t *)&val->val.i64);
    case ESP_MATTER_VAL_TYPE_UINT64:
    case ESP_MATTER_VAL_TYPE_NULLABLE_UINT64:
        return EmberAfDefaultAttributeValue((uint8_t *)&val->val.u64);
    default:
        break;
    }
    return EmberAfDefaultAttributeValue(nullptr);
}

class GlobalInteractionModelEngineChangedpathListener : public chip::app::AttributesChangedListener {
public:
    ~GlobalInteractionModelEngineChangedpathListener() = default;

    void MarkDirty(const chip::app::AttributePathParams &path) override
    {
        chip::app::InteractionModelEngine::GetInstance()->GetReportingEngine().SetDirty(path);
    }
};

static GlobalInteractionModelEngineChangedpathListener gListener;
} // namespace

namespace chip {
namespace app {
EnabledEndpointsWithServerCluster::EnabledEndpointsWithServerCluster(ClusterId clusterId)
    : mEndpointCount(esp_matter::endpoint::get_count(esp_matter::node::get()))
    , mClusterId(clusterId)
{
    EnsureMatchingEndpoint();
}

EndpointId EnabledEndpointsWithServerCluster::operator*() const
{
    return emberAfEndpointFromIndex(mEndpointIndex);
}

EnabledEndpointsWithServerCluster &EnabledEndpointsWithServerCluster::operator++()
{
    ++mEndpointIndex;
    EnsureMatchingEndpoint();
    return *this;
}

void EnabledEndpointsWithServerCluster::EnsureMatchingEndpoint()
{
    for (; mEndpointIndex < mEndpointCount; ++mEndpointIndex) {
        esp_matter::endpoint_t *ep = get_endpoint_at_index(mEndpointIndex);
        if (!esp_matter::endpoint::is_enabled(ep)) {
            continue;
        }
        if (esp_matter::cluster::get(ep, mClusterId)) {
            break;
        }
    }
}

namespace Compatibility {
namespace Internal {

EmberAfAttributeType AttributeBaseType(EmberAfAttributeType type)
{
    switch (type) {
    case ZCL_ACTION_ID_ATTRIBUTE_TYPE: // Action Id
    case ZCL_FABRIC_IDX_ATTRIBUTE_TYPE: // Fabric Index
    case ZCL_BITMAP8_ATTRIBUTE_TYPE: // 8-bit bitmap
    case ZCL_ENUM8_ATTRIBUTE_TYPE: // 8-bit enumeration
    case ZCL_STATUS_ATTRIBUTE_TYPE: // Status Code
    case ZCL_PERCENT_ATTRIBUTE_TYPE: // Percentage
        static_assert(std::is_same<chip::Percent, uint8_t>::value,
                      "chip::Percent is expected to be uint8_t, change this when necessary");
        return ZCL_INT8U_ATTRIBUTE_TYPE;

    case ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE: // Endpoint Number
    case ZCL_GROUP_ID_ATTRIBUTE_TYPE: // Group Id
    case ZCL_VENDOR_ID_ATTRIBUTE_TYPE: // Vendor Id
    case ZCL_ENUM16_ATTRIBUTE_TYPE: // 16-bit enumeration
    case ZCL_BITMAP16_ATTRIBUTE_TYPE: // 16-bit bitmap
    case ZCL_PERCENT100THS_ATTRIBUTE_TYPE: // 100ths of a percent
        static_assert(std::is_same<chip::EndpointId, uint16_t>::value,
                      "chip::EndpointId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::GroupId, uint16_t>::value,
                      "chip::GroupId is expected to be uint16_t, change this when necessary");
        static_assert(std::is_same<chip::Percent100ths, uint16_t>::value,
                      "chip::Percent100ths is expected to be uint16_t, change this when necessary");
        return ZCL_INT16U_ATTRIBUTE_TYPE;

    case ZCL_CLUSTER_ID_ATTRIBUTE_TYPE: // Cluster Id
    case ZCL_ATTRIB_ID_ATTRIBUTE_TYPE: // Attribute Id
    case ZCL_FIELD_ID_ATTRIBUTE_TYPE: // Field Id
    case ZCL_EVENT_ID_ATTRIBUTE_TYPE: // Event Id
    case ZCL_COMMAND_ID_ATTRIBUTE_TYPE: // Command Id
    case ZCL_TRANS_ID_ATTRIBUTE_TYPE: // Transaction Id
    case ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE: // Device Type Id
    case ZCL_DATA_VER_ATTRIBUTE_TYPE: // Data Version
    case ZCL_BITMAP32_ATTRIBUTE_TYPE: // 32-bit bitmap
    case ZCL_EPOCH_S_ATTRIBUTE_TYPE: // Epoch Seconds
    case ZCL_ELAPSED_S_ATTRIBUTE_TYPE: // Elapsed Seconds
        static_assert(std::is_same<chip::ClusterId, uint32_t>::value,
                      "chip::Cluster is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::AttributeId, uint32_t>::value,
                      "chip::AttributeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::EventId, uint32_t>::value,
                      "chip::EventId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::CommandId, uint32_t>::value,
                      "chip::CommandId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::TransactionId, uint32_t>::value,
                      "chip::TransactionId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DeviceTypeId, uint32_t>::value,
                      "chip::DeviceTypeId is expected to be uint32_t, change this when necessary");
        static_assert(std::is_same<chip::DataVersion, uint32_t>::value,
                      "chip::DataVersion is expected to be uint32_t, change this when necessary");
        return ZCL_INT32U_ATTRIBUTE_TYPE;

    case ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE: // Amperage
    case ZCL_ENERGY_MWH_ATTRIBUTE_TYPE: // Energy
    case ZCL_ENERGY_MVAH_ATTRIBUTE_TYPE: // Apparent Energy
    case ZCL_ENERGY_MVARH_ATTRIBUTE_TYPE: // Reactive Energy
    case ZCL_POWER_MW_ATTRIBUTE_TYPE: // Power
    case ZCL_POWER_MVA_ATTRIBUTE_TYPE: // Apparent Power
    case ZCL_POWER_MVAR_ATTRIBUTE_TYPE: // Reactive Power
    case ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE: // Voltage
    case ZCL_MONEY_ATTRIBUTE_TYPE: // Money
        return ZCL_INT64S_ATTRIBUTE_TYPE;

    case ZCL_EVENT_NO_ATTRIBUTE_TYPE: // Event Number
    case ZCL_FABRIC_ID_ATTRIBUTE_TYPE: // Fabric Id
    case ZCL_NODE_ID_ATTRIBUTE_TYPE: // Node Id
    case ZCL_BITMAP64_ATTRIBUTE_TYPE: // 64-bit bitmap
    case ZCL_EPOCH_US_ATTRIBUTE_TYPE: // Epoch Microseconds
    case ZCL_POSIX_MS_ATTRIBUTE_TYPE: // POSIX Milliseconds
    case ZCL_SYSTIME_MS_ATTRIBUTE_TYPE: // System time Milliseconds
    case ZCL_SYSTIME_US_ATTRIBUTE_TYPE: // System time Microseconds
        static_assert(std::is_same<chip::EventNumber, uint64_t>::value,
                      "chip::EventNumber is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::FabricId, uint64_t>::value,
                      "chip::FabricId is expected to be uint64_t, change this when necessary");
        static_assert(std::is_same<chip::NodeId, uint64_t>::value,
                      "chip::NodeId is expected to be uint64_t, change this when necessary");
        return ZCL_INT64U_ATTRIBUTE_TYPE;

    case ZCL_TEMPERATURE_ATTRIBUTE_TYPE: // Temperature
        return ZCL_INT16S_ATTRIBUTE_TYPE;

    default:
        return type;
    }
}

} // namespace Internal
} // namespace Compatibility
} // namespace app
} // namespace chip

// TODO: Remove the BindingTable definition when binding cluster is decoupled from ember.
namespace chip {

BindingTable BindingTable::sInstance;

BindingTable::BindingTable()
{
    memset(mNextIndex, kNextNullIndex, sizeof(mNextIndex));
}

CHIP_ERROR BindingTable::Add(const EmberBindingTableEntry &entry)
{
    if (entry.type == MATTER_UNUSED_BINDING) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t newIndex = MATTER_BINDING_TABLE_SIZE;
    for (uint8_t i = 0; i < MATTER_BINDING_TABLE_SIZE; i++) {
        if (mBindingTable[i].type == MATTER_UNUSED_BINDING) {
            newIndex = i;
        }
    }
    if (newIndex >= MATTER_BINDING_TABLE_SIZE) {
        return CHIP_ERROR_NO_MEMORY;
    }
    mBindingTable[newIndex] = entry;
    CHIP_ERROR error = SaveEntryToStorage(newIndex, kNextNullIndex);
    if (error == CHIP_NO_ERROR) {
        if (mTail == kNextNullIndex) {
            error = SaveListInfo(newIndex);
        } else {
            error = SaveEntryToStorage(mTail, newIndex);
        }
        if (error != CHIP_NO_ERROR) {
            mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(newIndex).KeyName());
        }
    }
    if (error != CHIP_NO_ERROR) {
        // Roll back
        mBindingTable[newIndex].type = MATTER_UNUSED_BINDING;
        return error;
    }

    if (mTail == kNextNullIndex) {
        mTail = newIndex;
        mHead = newIndex;
    } else {
        mNextIndex[mTail] = newIndex;
        mNextIndex[newIndex] = kNextNullIndex;
        mTail = newIndex;
    }

    mSize++;
    return CHIP_NO_ERROR;
}

const EmberBindingTableEntry &BindingTable::GetAt(uint8_t index)
{
    return mBindingTable[index];
}

CHIP_ERROR BindingTable::SaveEntryToStorage(uint8_t index, uint8_t nextIndex)
{
    EmberBindingTableEntry &entry = mBindingTable[index];
    uint8_t buffer[kEntryStorageSize] = {0};
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::TLVType::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagFabricIndex), entry.fabricIndex));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagLocalEndpoint), entry.local));
    if (entry.clusterId.has_value()) {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagCluster), *entry.clusterId));
    }
    if (entry.type == MATTER_UNICAST_BINDING) {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagRemoteEndpoint), entry.remote));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagNodeId), entry.nodeId));
    } else {
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagGroupId), entry.groupId));
    }
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagNextEntry), nextIndex));
    ReturnErrorOnFailure(writer.EndContainer(container));
    ReturnErrorOnFailure(writer.Finalize());
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(index).KeyName(), buffer,
                                     static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR BindingTable::SaveListInfo(uint8_t head)
{
    uint8_t buffer[kListInfoStorageSize] = {0};
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType container;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::TLVType::kTLVType_Structure, container));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagStorageVersion), kStorageVersion));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagHead), head));
    ReturnErrorOnFailure(writer.EndContainer(container));
    ReturnErrorOnFailure(writer.Finalize());
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::BindingTable().KeyName(), buffer,
                                     static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR BindingTable::LoadFromStorage()
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint8_t buffer[kListInfoStorageSize] = {0};
    uint16_t size = sizeof(buffer);
    CHIP_ERROR error;

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::BindingTable().KeyName(), buffer, size));
    TLV::TLVReader reader;
    reader.Init(buffer, size);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType container;
    ReturnErrorOnFailure(reader.EnterContainer(container));

    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagStorageVersion)));
    uint32_t version;
    ReturnErrorOnFailure(reader.Get(version));
    VerifyOrReturnError(version == kStorageVersion, CHIP_ERROR_VERSION_MISMATCH);
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagHead)));
    uint8_t index;
    ReturnErrorOnFailure(reader.Get(index));
    mHead = index;
    while (index != kNextNullIndex) {
        uint8_t nextIndex;
        error = LoadEntryFromStorage(index, nextIndex);
        if (error != CHIP_NO_ERROR) {
            mHead = kNextNullIndex;
            mTail = kNextNullIndex;
            return error;
        }
        mTail = index;
        index = nextIndex;
        mSize++;
    }
    error = reader.ExitContainer(container);
    if (error != CHIP_NO_ERROR) {
        mHead = kNextNullIndex;
        mTail = kNextNullIndex;
    }
    return error;
}

CHIP_ERROR BindingTable::LoadEntryFromStorage(uint8_t index, uint8_t &nextIndex)
{
    uint8_t buffer[kEntryStorageSize] = {0};
    uint16_t size = sizeof(buffer);
    EmberBindingTableEntry entry;

    ReturnErrorOnFailure(
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(index).KeyName(), buffer, size));
    TLV::TLVReader reader;
    reader.Init(buffer, size);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType container;
    ReturnErrorOnFailure(reader.EnterContainer(container));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagFabricIndex)));
    ReturnErrorOnFailure(reader.Get(entry.fabricIndex));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagLocalEndpoint)));
    ReturnErrorOnFailure(reader.Get(entry.local));
    ReturnErrorOnFailure(reader.Next());
    if (reader.GetTag() == TLV::ContextTag(kTagCluster)) {
        ClusterId clusterId;
        ReturnErrorOnFailure(reader.Get(clusterId));
        entry.clusterId.emplace(clusterId);
        ReturnErrorOnFailure(reader.Next());
    } else {
        entry.clusterId = std::nullopt;
    }
    if (reader.GetTag() == TLV::ContextTag(kTagRemoteEndpoint)) {
        entry.type = MATTER_UNICAST_BINDING;
        ReturnErrorOnFailure(reader.Get(entry.remote));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagNodeId)));
        ReturnErrorOnFailure(reader.Get(entry.nodeId));
    } else {
        entry.type = MATTER_MULTICAST_BINDING;
        VerifyOrReturnError(reader.GetTag() == TLV::ContextTag(kTagGroupId), CHIP_ERROR_INVALID_TLV_TAG);
        ReturnErrorOnFailure(reader.Get(entry.groupId));
    }
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagNextEntry)));
    ReturnErrorOnFailure(reader.Get(nextIndex));
    ReturnErrorOnFailure(reader.ExitContainer(container));
    mBindingTable[index] = entry;
    mNextIndex[index] = nextIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingTable::RemoveAt(Iterator &iter)
{
    CHIP_ERROR error;
    if (iter.mTable != this || iter.mIndex == kNextNullIndex) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (iter.mIndex == mTail) {
        mTail = iter.mPrevIndex;
    }
    uint8_t next = mNextIndex[iter.mIndex];
    if (iter.mIndex != mHead) {
        error = SaveEntryToStorage(iter.mPrevIndex, next);
        if (error == CHIP_NO_ERROR) {
            mNextIndex[iter.mPrevIndex] = next;
        }
    } else {
        error = SaveListInfo(next);
        if (error == CHIP_NO_ERROR) {
            mHead = next;
        }
    }
    if (error == CHIP_NO_ERROR) {
        // The remove is considered "submitted" once the change on prev node takes effect
        if (mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::BindingTableEntry(iter.mIndex).KeyName()) !=
            CHIP_NO_ERROR) {
            ChipLogError(AppServer, "Failed to remove binding table entry %u from storage", iter.mIndex);
        }
        mBindingTable[iter.mIndex].type = MATTER_UNUSED_BINDING;
        mNextIndex[iter.mIndex] = kNextNullIndex;
        mSize--;
    }
    iter.mIndex = next;
    return error;
}

BindingTable::Iterator BindingTable::begin()
{
    Iterator iter;
    iter.mTable = this;
    iter.mPrevIndex = kNextNullIndex;
    iter.mIndex = mHead;
    return iter;
}

BindingTable::Iterator BindingTable::end()
{
    Iterator iter;
    iter.mTable = this;
    iter.mIndex = kNextNullIndex;
    return iter;
}

BindingTable::Iterator BindingTable::Iterator::operator++()
{
    if (mIndex != kNextNullIndex) {
        mPrevIndex = mIndex;
        mIndex = mTable->mNextIndex[mIndex];
    }
    return *this;
}

} // namespace chip

// Override Ember functions

// TODO: Remove the emberAfEndpointIndexIsEnabled and emberAfEndpointCount functions when FabricTableImpl is decoupled
// from ember
chip::EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    esp_matter::endpoint_t *ep = get_endpoint_at_index(index);
    if (ep) {
        return esp_matter::endpoint::get_id(ep);
    }
    return chip::kInvalidEndpointId;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    esp_matter::endpoint_t *ep = get_endpoint_at_index(index);
    if (ep) {
        return esp_matter::endpoint::is_enabled(ep);
    }
    return false;
}

uint16_t emberAfEndpointCount()
{
    return esp_matter::endpoint::get_count(esp_matter::node::get());
}

// TODO: Remove the emberAfGetClusterCountForEndpoint when scenes cluster is decoupled from ember
uint8_t emberAfGetClusterCountForEndpoint(chip::EndpointId endpoint)
{
    esp_matter::endpoint_t *ep = esp_matter::endpoint::get(endpoint);
    if (ep) {
        esp_matter::cluster_t *cluster = esp_matter::cluster::get_first(ep);
        if (cluster) {
            uint8_t count = 0;
            while (cluster) {
                count++;
                cluster = esp_matter::cluster::get_next(cluster);
            }
            return count;
        }
    }
    return 0;
}

// TODO: Remove the emberAfGetClustersFromEndpoint function when scenes cluster is decoupled from ember
uint8_t emberAfGetClustersFromEndpoint(chip::EndpointId endpoint, chip::ClusterId *clusterList, uint8_t listLen,
                                       bool server)
{
    esp_matter::endpoint_t *ep = esp_matter::endpoint::get(endpoint);
    if (ep) {
        esp_matter::cluster_t *cluster = esp_matter::cluster::get_first(ep);
        if (cluster) {
            uint8_t count = 0;
            while (cluster) {
                if ((server && (esp_matter::cluster::get_flags(cluster) & esp_matter::CLUSTER_FLAG_SERVER)) ||
                    (!server && (esp_matter::cluster::get_flags(cluster) & esp_matter::CLUSTER_FLAG_CLIENT))) {
                    clusterList[count++] = esp_matter::cluster::get_id(cluster);
                    if (count >= listLen) {
                        break;
                    }
                }
                cluster = esp_matter::cluster::get_next(cluster);
            }
            return count;
        }
    }
    return 0;
}

// TODO: Remove the emberAfGetClusterServerEndpointIndex function when laundry-dryer-controls, keypad-input,
// door-lock, level-control, target-navigator, fan-control, occupancy-sensor, valve-configuration-and-control,
// media-playback, content-launch, audio-output, power-source, application-basic, low-power, diagnostic-logs,
// scenes, color-control, channel, laundry-washer-controls, wake-on-lan, window-covering, content-control,
// dishwasher-alarm, on-off, media-input, application-launcher, account-login, thermostat, electrical-energy-measurement,
// content-app-observer, and boolean-state-configuration clusters are decoupled from ember.
uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                              uint16_t fixedClusterServerEndpointCount)
{
    esp_matter::endpoint_t *ep = esp_matter::endpoint::get(endpoint);
    if (ep) {
        esp_matter::cluster_t *cluster = esp_matter::cluster::get(ep, clusterId);
        if (!cluster) {
            return 0xFFFF;
        }
        ep = esp_matter::endpoint::get_first(esp_matter::node::get());
        uint16_t ret = 0;
        while (ep && esp_matter::endpoint::get_id(ep) != endpoint) {
            if (esp_matter::cluster::get(ep, clusterId)) {
                ret++;
            }
            ep = esp_matter::endpoint::get_next(ep);
        }
        return ret;
    }
    return 0xFFFF;
}

// TODO: Remove the emberAfIsKnownVolatileAttribute function when level-control, mode-select, mode-base, and on-off
// clusters are decoupled from ember.
bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                     chip::AttributeId attributeId)
{
    esp_matter::attribute_t *attr = esp_matter::attribute::get(endpoint, clusterId, attributeId);
    if (!attr) {
        return false;
    }
    return !(esp_matter::attribute::get_flags(attr) & esp_matter::ATTRIBUTE_FLAG_NONVOLATILE);
}

// TODO: Remove the emberAfContainsServer function when soil-measurement, on-off, mode-base, resource-monitoring,
// mode-select, color-control, microwave-oven-control, concentration-measurement, air-quality, operational-state,
// thread-network-diagnostics, general-diagnostics, level-control, and service-area clusters are decoupled from ember.
bool emberAfContainsServer(chip::EndpointId endpoint, chip::ClusterId clusterId)
{
    esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, clusterId);
    if (cluster && (esp_matter::cluster::get_flags(cluster) & esp_matter::CLUSTER_FLAG_SERVER)) {
        return true;
    }
    return false;
}

// TODO: Remove the emberAfContainsAttribute function when level-control, mode-select, resource-monitoring, mode-base,
// on-off, and pump-configuration-and-control clusters are decoupled from ember
bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return esp_matter::attribute::get(endpoint, clusterId, attributeId);
}

// TODO: Remove the emberAfContainsClient function when binding cluster is decoupled from ember
bool emberAfContainsClient(chip::EndpointId endpoint, chip::ClusterId clusterId)
{
    esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, clusterId);
    if (cluster && (esp_matter::cluster::get_flags(cluster) & esp_matter::CLUSTER_FLAG_SERVER)) {
        return true;
    }
    return false;
}

// TODO: Remove the GetSemanticTagForEndpointAtIndex function when descriptor cluster is decoupled from ember
CHIP_ERROR GetSemanticTagForEndpointAtIndex(chip::EndpointId endpoint, size_t index,
                                            chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type &tag)
{
    esp_matter::endpoint_t *ep = esp_matter::endpoint::get(endpoint);
    if (!ep || (esp_matter::endpoint::get_semantic_tag_at_index(ep, index, tag) != ESP_OK)) {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CHIP_NO_ERROR;
}

// TODO: Remove the emberAfRead/Write functions when all the clusters are decoupled from ember.
chip::Protocols::InteractionModel::Status emberAfReadAttribute(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                               chip::AttributeId attributeId, uint8_t *dataPtr,
                                                               uint16_t readLength)
{
    esp_matter::endpoint_t *endpoint = esp_matter::endpoint::get(endpointId);
    if (!endpoint) {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, clusterId);
    if (!cluster) {
        return chip::Protocols::InteractionModel::Status::UnsupportedCluster;
    }
    esp_matter::attribute_t *attribute = esp_matter::attribute::get(cluster, attributeId);
    if (!attribute) {
        return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
    if (esp_matter::attribute::get_val(attribute, &val) != ESP_OK) {
        return chip::Protocols::InteractionModel::Status::Failure;
    }
    return get_raw_data_buffer_from_attr_val(val, dataPtr, readLength);
}

Status emberAfWriteAttribute(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                             uint8_t *value, EmberAfAttributeType dataType)
{
    return emberAfWriteAttribute(chip::app::ConcreteAttributePath(endpointId, clusterId, attributeId),
                                 EmberAfWriteDataInput(value, dataType).SetChangeListener(&gListener));
}

Status emberAfWriteAttribute(const chip::app::ConcreteAttributePath &path, const EmberAfWriteDataInput &input)
{
    esp_matter::endpoint_t *endpoint = esp_matter::endpoint::get(path.mEndpointId);
    if (!endpoint) {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    esp_matter::cluster_t *cluster = esp_matter::cluster::get(endpoint, path.mClusterId);
    if (!cluster) {
        return chip::Protocols::InteractionModel::Status::UnsupportedCluster;
    }
    esp_matter::attribute_t *attribute = esp_matter::attribute::get(cluster, path.mAttributeId);
    if (!attribute) {
        return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
    Status status = get_attr_val_from_raw_data_buffer(input.dataPtr, input.dataType, val,
                                                      esp_matter::attribute::get_flags(attribute) &
                                                          esp_matter::ATTRIBUTE_FLAG_NULLABLE);
    if (status != Status::Success) {
        return status;
    }
    esp_err_t err = esp_matter::attribute::set_val(attribute, &val);
    if (err != ESP_OK && err != ESP_ERR_NOT_FINISHED) {
        status = Status::Failure;
    }
    if (status == Status::Success) {
        if (input.markDirty == chip::app::MarkAttributeDirty::kYes ||
            ((err == ESP_OK) && (input.markDirty != chip::app::MarkAttributeDirty::kNo))) {
            if (input.changeListener) {
                input.changeListener->MarkDirty(
                    chip::app::AttributePathParams(path.mEndpointId, path.mClusterId, path.mAttributeId));
            } else {
                gListener.MarkDirty(
                    chip::app::AttributePathParams(path.mEndpointId, path.mClusterId, path.mAttributeId));
            }
        }
    }
    return status;
}

// TODO: Remove this function when scenes and thermostat clusters are decoupled from ember APIs
// Since the attribute Metadata should always be accessed in Matter context, we return a pointer of static value.
// But it might be dangerous when the user use this API out of Matter context.
const EmberAfAttributeMetadata *emberAfLocateAttributeMetadata(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                               chip::AttributeId attributeId)
{
    static esp_matter_attr_bounds_t sBounds;
    static EmberAfAttributeMinMaxValue sMinMaxValue{EmberAfDefaultAttributeValue(nullptr),
                                             EmberAfDefaultAttributeValue(nullptr),
                                             EmberAfDefaultAttributeValue(nullptr)};
    static EmberAfAttributeMetadata s_metadata{EmberAfDefaultOrMinMaxAttributeValue((uint32_t)0), attributeId, 0, 0, 0};

    esp_matter::attribute_t *attribute = esp_matter::attribute::get(endpointId, clusterId, attributeId);
    if (attribute) {
        esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
        if (esp_matter::attribute::get_val(attribute, &val) == ESP_OK) {
            s_metadata.attributeId = attributeId;
            s_metadata.attributeType = get_ember_attr_type_from_val_type(val.type);
            s_metadata.mask = esp_matter::attribute::get_flags(attribute) & 0xFF;
            s_metadata.size = get_ember_attr_size_from_val(val);
            if (s_metadata.HasMinMax() && esp_matter::attribute::get_bounds(attribute, &sBounds) == ESP_OK) {
                sMinMaxValue.minValue = get_default_attr_value_from_val(&sBounds.min);
                sMinMaxValue.maxValue = get_default_attr_value_from_val(&sBounds.max);
                s_metadata.defaultValue.ptrToMinMaxValue = &sMinMaxValue;
            }
            return &s_metadata;
        }
    }
    return nullptr;
}
