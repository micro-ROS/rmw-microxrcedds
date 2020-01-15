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

#include "./types.h"  // NOLINT

#include "./memory.h"

void init_service_memory(
  struct MemPool * memory,
  CustomService services[MAX_SERVICES_X_NODE], size_t size)
{
  if (size > 0) {
    link_prev(NULL, &services[0].mem, NULL);
    size > 1 ? link_next(&services[0].mem, &services[1].mem, &services[0]) : link_next(
      &services[0].mem, NULL, &services[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&services[i - 1].mem, &services[i].mem, &services[i]);
    }
    link_next(&services[size - 1].mem, NULL, &services[size - 1]);
    set_mem_pool(memory, &services[0].mem);
  }
}

void init_client_memory(
  struct MemPool * memory,
  CustomClient clients[MAX_CLIENTS_X_NODE], size_t size)
{
  if (size > 0) {
    link_prev(NULL, &clients[0].mem, NULL);
    size > 1 ? link_next(&clients[0].mem, &clients[1].mem, &clients[0]) : link_next(
      &clients[0].mem, NULL, &clients[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&clients[i - 1].mem, &clients[i].mem, &clients[i]);
    }
    link_next(&clients[size - 1].mem, NULL, &clients[size - 1]);
    set_mem_pool(memory, &clients[0].mem);
  }
}

void init_publisher_memory(
  struct MemPool * memory,
  CustomPublisher publishers[MAX_PUBLISHERS_X_NODE], size_t size)
{
  if (size > 0) {
    link_prev(NULL, &publishers[0].mem, NULL);
    size > 1 ? link_next(&publishers[0].mem, &publishers[1].mem, &publishers[0]) : link_next(
      &publishers[0].mem, NULL, &publishers[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&publishers[i - 1].mem, &publishers[i].mem, &publishers[i]);
    }
    link_next(&publishers[size - 1].mem, NULL, &publishers[size - 1]);
    set_mem_pool(memory, &publishers[0].mem);
  }
}

void init_subscriber_memory(
  struct MemPool * memory,
  CustomSubscription subscribers[MAX_SUBSCRIPTIONS_X_NODE], size_t size)
{
  if (size > 0) {
    link_prev(NULL, &subscribers[0].mem, NULL);
    size > 1 ? link_next(&subscribers[0].mem, &subscribers[1].mem, &subscribers[0]) : link_next(
      &subscribers[0].mem, NULL, &subscribers[0]);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&subscribers[i - 1].mem, &subscribers[i].mem, &subscribers[i]);
    }
    link_next(&subscribers[size - 1].mem, NULL, &subscribers[size - 1]);
    set_mem_pool(memory, &subscribers[0].mem);
  }
}

void init_nodes_memory(struct MemPool * memory, CustomNode nodes[MAX_NODES], size_t size)
{
  if (size > 0) {
    link_prev(NULL, &nodes[0].mem, NULL);
    size > 1 ? link_next(&nodes[0].mem, &nodes[1].mem, &nodes[0]) : link_next(&nodes[0].mem, NULL,
      &nodes[0]);
    init_publisher_memory(&nodes[0].publisher_mem, nodes[0].publisher_info, MAX_PUBLISHERS_X_NODE);
    init_subscriber_memory(&nodes[0].subscription_mem, nodes[0].subscription_info,
      MAX_PUBLISHERS_X_NODE);
    init_service_memory(&nodes[0].service_mem, nodes[0].service_info, MAX_SERVICES_X_NODE);
    init_client_memory(&nodes[0].client_mem, nodes[0].client_info, MAX_CLIENTS_X_NODE);
    for (unsigned int i = 1; i <= size - 1; i++) {
      link_prev(&nodes[i - 1].mem, &nodes[i].mem, &nodes[i]);
      init_publisher_memory(&nodes[i].publisher_mem, nodes[i].publisher_info,
        MAX_PUBLISHERS_X_NODE);
      init_subscriber_memory(&nodes[i].subscription_mem, nodes[i].subscription_info,
        MAX_PUBLISHERS_X_NODE);
      init_service_memory(&nodes[i].service_mem, nodes[i].service_info,
        MAX_SERVICES_X_NODE);
      init_client_memory(&nodes[i].client_mem, nodes[i].client_info,
        MAX_CLIENTS_X_NODE);
    }
    link_next(&nodes[size - 1].mem, NULL, &nodes[size - 1]);
    set_mem_pool(memory, &nodes[0].mem);
  }
}
