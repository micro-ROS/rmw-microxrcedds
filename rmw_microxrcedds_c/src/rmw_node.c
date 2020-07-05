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

#include "rmw_node.h"  // NOLINT
#include "types.h"
#include "utils.h"
#include "identifiers.h"

#include <rmw_microxrcedds_c/config.h>

#ifdef MICRO_XRCEDDS_SERIAL
#include <fcntl.h>  // O_RDWR, O_NOCTTY, O_NONBLOCK
#include <termios.h>
#endif

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

#include "./types.h"
#include "./utils.h"

/* Variables definition to fix compiler warnings */

#define TIMEOUT_IN_MS       1000

rmw_node_t * create_node(const char * name, const char * namespace_, size_t domain_id, const rmw_context_t * context)
{
  if (!context) {
    RMW_SET_ERROR_MSG("context is null");
    return NULL;
  }

  struct rmw_uxrce_mempool_item_t * memory_node = get_memory(&node_memory);
  if (!memory_node) {
    RMW_SET_ERROR_MSG("Not available memory node");
    return NULL;
  }

  rmw_uxrce_node_t * node_info = (rmw_uxrce_node_t *)memory_node->data;

  node_info->context = context->impl;

  rmw_node_t * node_handle = NULL;
  node_handle = rmw_node_allocate();
  if (!node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
    return NULL;
  }
  node_handle->implementation_identifier = rmw_get_implementation_identifier();
  node_handle->data = node_info;
  node_handle->name = (const char *)(rmw_allocate(sizeof(char) * (strlen(name) + 1)));
  if (!node_handle->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    rmw_uxrce_fini_node_memory(node_handle);
    return NULL;
  }
  memcpy((char *)node_handle->name, name, strlen(name) + 1);

  node_handle->namespace_ = rmw_allocate(sizeof(char) * (strlen(namespace_) + 1));
  if (!node_handle->namespace_) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    rmw_uxrce_fini_node_memory(node_handle);
    return NULL;
  }
  memcpy((char *)node_handle->namespace_, namespace_, strlen(namespace_) + 1);

  node_info->participant_id = uxr_object_id(node_info->context->id_participant++, UXR_PARTICIPANT_ID);
  uint16_t participant_req = 0;
#ifdef MICRO_XRCEDDS_USE_XML
  char participant_xml[RMW_UXRCE_XML_BUFFER_LENGTH];
  if (!build_participant_xml(domain_id, name, participant_xml, sizeof(participant_xml))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    return NULL;
  }
  participant_req =
    uxr_buffer_create_participant_xml(
      &node_info->context->session,
      node_info->context->reliable_output,
      node_info->participant_id, (uint16_t)domain_id, participant_xml, UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
  char profile_name[RMW_UXRCE_REF_BUFFER_LENGTH];
  if (!build_participant_profile(profile_name, sizeof(profile_name))) {
    RMW_SET_ERROR_MSG("failed to generate xml request for node creation");
    return NULL;
  }
  participant_req =
    uxr_buffer_create_participant_ref(
      &node_info->context->session,
      node_info->context->reliable_output,
      node_info->participant_id, (uint16_t)domain_id, profile_name, UXR_REPLACE);
#endif
  uint8_t status[1];
  uint16_t requests[] = {participant_req};

  if (!uxr_run_session_until_all_status(&node_info->context->session, TIMEOUT_IN_MS, requests, status, 1)) {
    uxr_delete_session(&node_info->context->session);
    rmw_uxrce_fini_node_memory(node_handle);
    RMW_SET_ERROR_MSG("Issues creating micro XRCE-DDS entities");
    return NULL;
  }
  
  return node_handle;
}

rmw_node_t *
rmw_create_node(
  rmw_context_t * context,
  const char * name,
  const char * namespace,
  size_t domain_id,
  const rmw_node_security_options_t * security_options)
{
  (void) context;
  EPROS_PRINT_TRACE()
  rmw_node_t * rmw_node = NULL;
  if (!name || strlen(name) == 0) {
    RMW_SET_ERROR_MSG("name is null");
  } else if (!namespace || strlen(namespace) == 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!security_options) {
    RMW_SET_ERROR_MSG("security_options is null");
  } else {
    rmw_node = create_node(name, namespace, domain_id, context);
  }
  return rmw_node;
}

rmw_ret_t rmw_destroy_node(rmw_node_t * node)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }

  if (strcmp(node->implementation_identifier, rmw_get_implementation_identifier()) != 0) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    return RMW_RET_ERROR;
  }

  if (!node->data) {
    RMW_SET_ERROR_MSG("node impl is null");
    return RMW_RET_ERROR;
  }

  rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)node->data;
  // TODO(Borja) make sure that session deletion deletes participant and related entities.
  // TODO(Pablo) make sure that other entities are removed from the pools

  struct rmw_uxrce_mempool_item_t * item = NULL;

  item = publisher_memory.allocateditems;
  while (item != NULL) {
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)item->data;
    item = item->next;
    if (custom_publisher->owner_node == custom_node){ 
      rmw_destroy_publisher(node, custom_publisher->rmw_handle);
    }
  }

  item = subscription_memory.allocateditems;
  while (item != NULL) {
    rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)item->data;
    item = item->next;
    if (custom_subscription->owner_node == custom_node){ 
      rmw_destroy_subscription(node, custom_subscription->rmw_handle);
    }
  }

  item = service_memory.allocateditems;
  while (item != NULL) {
    rmw_uxrce_service_t * custom_service = (rmw_uxrce_service_t *)item->data;
    item = item->next;
    if (custom_service->owner_node == custom_node){ 
      rmw_destroy_service(node, custom_service->rmw_handle);
    }
  }

  item = client_memory.allocateditems;
  while (item != NULL) {
    rmw_uxrce_client_t * custom_client = (rmw_uxrce_client_t *)item->data;
    item = item->next;
    if (custom_client->owner_node == custom_node){ 
      rmw_destroy_client(node, custom_client->rmw_handle);
    }
  }

  rmw_uxrce_fini_node_memory(node);

  return result_ret;
}

rmw_ret_t
rmw_node_assert_liveliness(const rmw_node_t * node)
{
  (void) node;
  RMW_SET_ERROR_MSG("function not implemeted");
  return RMW_RET_ERROR;
}

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  (void)node;
  EPROS_PRINT_TRACE()
  rmw_guard_condition_t *
  ret = (rmw_guard_condition_t *)rmw_allocate(sizeof(rmw_guard_condition_t));
  ret->data = NULL;
  ret->implementation_identifier = eprosima_microxrcedds_identifier;
  return ret;
}
