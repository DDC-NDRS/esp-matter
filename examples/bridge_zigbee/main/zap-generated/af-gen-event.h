/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *
 *    Copyright (c) 2020 Silicon Labs
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
// This file is generated by Simplicity Studio.  Please do not edit manually.
//
//

// Enclosing macro to prevent multiple inclusion
#ifndef __AF_GEN_EVENT__
#define __AF_GEN_EVENT__

// Code used to configure the cluster event mechanism
#define EMBER_AF_GENERATED_EVENT_CODE                                                                                              \
    EmberEventControl emberAfLevelControlClusterServerTickCallbackControl1;                                                        \
    static void clusterTickWrapper(EmberEventControl * control, EmberAfTickFunction callback, uint8_t endpoint)                    \
    {                                                                                                                              \
        /* emberAfPushEndpointNetworkIndex(endpoint); */                                                                           \
        emberEventControlSetInactive(control);                                                                                     \
        (*callback)(endpoint);                                                                                                     \
        /* emberAfPopNetworkIndex(); */                                                                                            \
    }                                                                                                                              \
    void emberAfLevelControlClusterServerTickCallbackWrapperFunction1(void)                                                        \
    {                                                                                                                              \
        clusterTickWrapper(&emberAfLevelControlClusterServerTickCallbackControl1, emberAfLevelControlClusterServerTickCallback,    \
                           1);                                                                                                     \
    }

// EmberEventData structs used to populate the EmberEventData table
#define EMBER_AF_GENERATED_EVENTS                                                                                                  \
    { &emberAfLevelControlClusterServerTickCallbackControl1, emberAfLevelControlClusterServerTickCallbackWrapperFunction1 },

#define EMBER_AF_GENERATED_EVENT_STRINGS "Level Control Cluster Server EP 1",

// The length of the event context table used to track and retrieve cluster events
#define EMBER_AF_EVENT_CONTEXT_LENGTH 1

// EmberAfEventContext structs used to populate the EmberAfEventContext table
#define EMBER_AF_GENERATED_EVENT_CONTEXT                                                                                           \
    { 0x1, 0x8, false, EMBER_AF_LONG_POLL, EMBER_AF_OK_TO_SLEEP, &emberAfLevelControlClusterServerTickCallbackControl1 },

#endif // __AF_GEN_EVENT__
