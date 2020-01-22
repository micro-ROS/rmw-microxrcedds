// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPES_H_
#define TYPES_H_

#include <stddef.h>

#include <rmw/types.h>
#include <ucdr/microcdr.h>
#include <uxr/client/client.h>

#include "rosidl_generator_c/message_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/message_type_support.h"

#include "rosidl_generator_c/service_type_support_struct.h"
#include "rosidl_typesupport_microxrcedds_c/service_type_support.h"

#include "memory.h"
#include <rmw_microxrcedds_c/config.h>

#define MAX_IP_LEN 16
#define MAX_PORT_LEN 5
#define MAX_SERIAL_DEVICE 50

struct rmw_microxrcedds_connection
{
  #ifdef MICRO_XRCEDDS_SERIAL
    char serial_device[MAX_SERIAL_DEVICE];
  #elif defined(MICRO_XRCEDDS_UDP)
    char agent_address[MAX_IP_LEN];
    char agent_port[MAX_PORT_LEN];
  #endif
  uint32_t client_key;
};


struct  rmw_context_impl_t
{
  struct rmw_microxrcedds_connection connection_params;
};

struct  rmw_init_options_impl_t
{
  struct rmw_microxrcedds_connection connection_params;
};

typedef struct custom_topic_t
{
  struct custom_topic_t * next_custom_topic;
  struct custom_topic_t * prev_custom_topic;

  uxrObjectId topic_id;
  const message_type_support_callbacks_t * message_type_support_callbacks;

  bool sync_with_agent;
  int32_t usage_account;
  struct CustomNode * owner_node;
} custom_topic_t;

typedef struct CustomService
{
  struct Item mem;
  uxrObjectId service_id;
  rmw_gid_t service_gid;
  const service_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;
  uint16_t request_id;

  SampleIdentity sample_id[RMW_UXRCE_MAX_HISTORY];
  uint8_t micro_buffer[RMW_UXRCE_MAX_HISTORY][RMW_UXRCE_MAX_BUFFER_SIZE];
  size_t micro_buffer_lenght[RMW_UXRCE_MAX_HISTORY];

  uint8_t history_write_index;
  uint8_t history_read_index;
  bool micro_buffer_in_use;


  uint8_t replay_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];

  struct CustomNode * owner_node;
} CustomService;

typedef struct CustomClient
{
  struct Item mem;
  uxrObjectId client_id;
  rmw_gid_t client_gid;
  const service_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;
  uint16_t request_id;

  int64_t reply_id[RMW_UXRCE_MAX_HISTORY];
  uint8_t micro_buffer[RMW_UXRCE_MAX_HISTORY][RMW_UXRCE_MAX_BUFFER_SIZE];
  size_t micro_buffer_lenght[RMW_UXRCE_MAX_HISTORY];
  uint8_t history_write_index;
  uint8_t history_read_index;
  bool micro_buffer_in_use;

  uint8_t request_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];

  struct CustomNode * owner_node;
} CustomClient;

typedef struct CustomSubscription
{
  struct Item mem;
  uxrObjectId subscriber_id;
  uxrObjectId datareader_id;
  rmw_gid_t subscription_gid;
  const message_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;  // TODO(Javier) duplicated: owner_node->session

  struct ucdrBuffer micro_buffer;
  bool micro_buffer_in_use;

  bool waiting_for_response;
  uint16_t subscription_request;

  uxrObjectId topic_id;  // TODO(Javier) Pending to be removed
  struct custom_topic_t * topic;

  struct CustomNode * owner_node;
} CustomSubscription;

typedef struct CustomPublisher
{
  struct Item mem;
  uxrObjectId publisher_id;
  uxrObjectId datawriter_id;
  rmw_gid_t publisher_gid;

  const message_type_support_callbacks_t * type_support_callbacks;
  uxrSession * session;  // TODO(Javier) duplicated: owner_node->session

  uxrObjectId topic_id;  // TODO(Javier) Pending to be removed
  struct custom_topic_t * topic;

  struct CustomNode * owner_node;
} CustomPublisher;

typedef struct CustomNode
{
  struct Item mem;
#if defined(MICRO_XRCEDDS_SERIAL) || defined(MICRO_XRCEDDS_CUSTOM)
  uxrSerialTransport transport;
  uxrSerialPlatform serial_platform;
#elif defined(MICRO_XRCEDDS_UDP)
  uxrUDPTransport transport;
  uxrUDPPlatform udp_platform;
#endif
  uxrSession session;
  uxrObjectId participant_id;
  struct MemPool publisher_mem;
  struct MemPool subscription_mem;
  struct MemPool service_mem;
  struct MemPool client_mem;

  CustomPublisher publisher_info[RMW_UXRCE_MAX_PUBLISHERS_X_NODE];
  CustomSubscription subscription_info[RMW_UXRCE_MAX_SUBSCRIPTIONS_X_NODE];
  CustomService service_info[RMW_UXRCE_MAX_SERVICES_X_NODE];
  CustomClient client_info[RMW_UXRCE_MAX_CLIENTS_X_NODE];

  custom_topic_t * custom_topic_sp;

  bool on_subscription;

  uxrStreamId reliable_input;
  uxrStreamId reliable_output;
  uxrStreamId best_effort_output;
  uxrStreamId best_effort_input;

  uint8_t input_reliable_stream_buffer[RMW_UXRCE_MAX_BUFFER_SIZE];
  uint8_t output_reliable_stream_buffer[RMW_UXRCE_MAX_BUFFER_SIZE];
  uint8_t output_best_effort_stream_buffer[RMW_UXRCE_MAX_TRANSPORT_MTU];

  uint16_t id_gen;
} CustomNode;



void init_nodes_memory(struct MemPool * memory, CustomNode nodes[RMW_UXRCE_MAX_NODES], size_t size);

#endif  // TYPES_H_
