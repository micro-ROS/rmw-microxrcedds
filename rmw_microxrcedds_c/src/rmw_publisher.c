// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

//#include "./rmw_publisher.h"  // NOLINT

#include "utils.h"
#include "rmw_microxrcedds_topic.h"

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif
#include <rosidl_typesupport_microxrcedds_c/message_type_support.h>

#include <rmw/allocators.h>
#include <rmw/error_handling.h>
#include <rmw/rmw.h>

rmw_ret_t
rmw_init_publisher_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_runtime_c__Sequence__bound * message_bounds,
  rmw_publisher_allocation_t * allocation)
{
  (void) type_support;
  (void) message_bounds;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_fini_publisher_allocation(
  rmw_publisher_allocation_t * allocation)
{
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_publisher_t *
rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
  const rmw_publisher_options_t * publisher_options)
{
  (void) publisher_options;

  EPROS_PRINT_TRACE()
  rmw_publisher_t * rmw_publisher = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("publisher topic is null or empty string");
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
  } else {
    rmw_publisher = (rmw_publisher_t *)rmw_allocate(sizeof(rmw_publisher_t));
    rmw_publisher->data = NULL;
    rmw_publisher->implementation_identifier = rmw_get_implementation_identifier();
    rmw_publisher->topic_name =
      (const char *)(rmw_allocate(sizeof(char) * (strlen(topic_name) + 1)));
    if (!rmw_publisher->topic_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }

    rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)node->data;
    struct rmw_uxrce_mempool_item_t * memory_node = get_memory(&publisher_memory);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    // TODO(Borja) micro_xrcedds_id is duplicated in publisher_id and in publisher_gid.data
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)memory_node->data;
    custom_publisher->rmw_handle = rmw_publisher;
    custom_publisher->owner_node = custom_node;
    custom_publisher->publisher_gid.implementation_identifier = rmw_get_implementation_identifier();
    memcpy(&custom_publisher->qos, qos_policies, sizeof(rmw_qos_profile_t));

    custom_publisher->stream_id =
      (qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) || (custom_node->context->entity_creation_output->type == UXR_BEST_EFFORT_STREAM) ?
      custom_node->context->best_effort_output :
      custom_node->context->reliable_output;

    const rosidl_message_type_support_t * type_support_xrce = NULL;
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE
    type_support_xrce = get_message_typesupport_handle(
      type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE);
#endif
#ifdef ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE
    if (NULL == type_support_xrce) {
      type_support_xrce = get_message_typesupport_handle(
        type_support, ROSIDL_TYPESUPPORT_MICROXRCEDDS_CPP__IDENTIFIER_VALUE);
    }
#endif
    if (NULL == type_support_xrce) {
      RMW_SET_ERROR_MSG("Undefined type support");
      goto fail;
    }

    custom_publisher->type_support_callbacks =
      (const message_type_support_callbacks_t *)type_support_xrce->data;

    if (custom_publisher->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    } else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
      RMW_SET_ERROR_MSG("Not enough memory for impl ids");
      goto fail;
    }

    memset(custom_publisher->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
    memcpy(
      custom_publisher->publisher_gid.data, &custom_publisher->publisher_id,
      sizeof(uxrObjectId));

    custom_publisher->topic = create_topic(
      custom_node, topic_name,
      custom_publisher->type_support_callbacks, qos_policies);
    if (custom_publisher->topic == NULL) {
      goto fail;
    }

    custom_publisher->publisher_id = uxr_object_id(
      custom_node->context->id_publisher++,
      UXR_PUBLISHER_ID);
    uint16_t publisher_req = UXR_INVALID_REQUEST_ID;

  #ifdef MICRO_XRCEDDS_USE_XML
    char publisher_name[20];
    generate_name(&custom_publisher->publisher_id, publisher_name, sizeof(publisher_name));
    if (!build_publisher_xml(publisher_name, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer))) {
      RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
      goto fail;
    }
    publisher_req = uxr_buffer_create_publisher_xml(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->entity_creation_output,
      custom_publisher->publisher_id,
      custom_node->participant_id, rmw_uxrce_xml_buffer, UXR_REPLACE);
  #elif defined(MICRO_XRCEDDS_USE_REFS)
    publisher_req = uxr_buffer_create_publisher_xml(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->entity_creation_output,
      custom_publisher->publisher_id,
      custom_node->participant_id, 
      "", 
      UXR_REPLACE);
  #endif

    custom_publisher->datawriter_id = uxr_object_id(
      custom_node->context->id_datawriter++,
      UXR_DATAWRITER_ID);
    uint16_t datawriter_req = UXR_INVALID_REQUEST_ID;

  #ifdef MICRO_XRCEDDS_USE_XML
    if (!build_datawriter_xml(
        topic_name, custom_publisher->type_support_callbacks,
        qos_policies, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer)))
    {
      RMW_SET_ERROR_MSG("failed to generate xml request for publisher creation");
      goto fail;
    }

    datawriter_req = uxr_buffer_create_datawriter_xml(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->entity_creation_output,
      custom_publisher->datawriter_id,
      custom_publisher->publisher_id, 
      rmw_uxrce_xml_buffer, 
      UXR_REPLACE);
  #elif defined(MICRO_XRCEDDS_USE_REFS)
    if (!build_datawriter_profile(topic_name, rmw_uxrce_profile_name, sizeof(rmw_uxrce_profile_name))) {
      RMW_SET_ERROR_MSG("failed to generate ref request datawriter creation");
      goto fail;
    }

    datawriter_req = uxr_buffer_create_datawriter_ref(
      &custom_publisher->owner_node->context->session,
      *custom_node->context->entity_creation_output,
      custom_publisher->datawriter_id,
      custom_publisher->publisher_id, 
      rmw_uxrce_profile_name, 
      UXR_REPLACE);
  #endif

    rmw_publisher->data = custom_publisher;

    uint16_t requests[] = {publisher_req, datawriter_req};
    uint8_t status[sizeof(requests) / 2];

    if (UXR_BEST_EFFORT_STREAM == custom_node->context->entity_creation_output->type)
    {
      uxr_flash_output_streams(&custom_node->context->session);
    }
    else if(!uxr_run_session_until_all_status(
          &custom_publisher->owner_node->context->session, 1000, requests,
          status, sizeof(status)))
    {
      RMW_SET_ERROR_MSG("Issues creating Publisher Micro XRCE-DDS entities");
      put_memory(&publisher_memory, &custom_publisher->mem);
      goto fail;
    }
  }
  return rmw_publisher;

fail:
  rmw_uxrce_fini_publisher_memory(rmw_publisher);
  rmw_publisher = NULL;
  return rmw_publisher;
}

rmw_ret_t
rmw_publisher_count_matched_subscriptions(
  const rmw_publisher_t * publisher,
  size_t * subscription_count)
{
  (void) publisher;
  (void) subscription_count;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_publisher_assert_liveliness(const rmw_publisher_t * publisher)
{
  (void) publisher;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_publisher_get_actual_qos(
  const rmw_publisher_t * publisher,
  rmw_qos_profile_t * qos)
{
  (void) qos;

  rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;
  qos = &custom_publisher->qos;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_borrow_loaned_message(
  const rmw_publisher_t * publisher,
  const rosidl_message_type_support_t * type_support,
  void ** ros_message)
{
  (void) publisher;
  (void) type_support;
  (void) ros_message;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_return_loaned_message_from_publisher(
  const rmw_publisher_t * publisher,
  void * loaned_message)
{
  (void) publisher;
  (void) loaned_message;

  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_destroy_publisher(
  rmw_node_t * node,
  rmw_publisher_t * publisher)
{
  EPROS_PRINT_TRACE()
  rmw_ret_t result_ret = RMW_RET_OK;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!node->data) {
    RMW_SET_ERROR_MSG("node imp is null");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher) {
    RMW_SET_ERROR_MSG("publisher handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (!is_uxrce_rmw_identifier_valid(publisher->implementation_identifier))
  {
    RMW_SET_ERROR_MSG("publisher handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!publisher->data) {
    RMW_SET_ERROR_MSG("publisher imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    rmw_uxrce_publisher_t * custom_publisher = (rmw_uxrce_publisher_t *)publisher->data;

    destroy_topic(custom_publisher->topic);

    uint16_t delete_writer = uxr_buffer_delete_entity(
      &custom_publisher->owner_node->context->session,
      *custom_publisher->owner_node->context->entity_creation_output,
      custom_publisher->datawriter_id);
    uint16_t delete_publisher = uxr_buffer_delete_entity(
      &custom_publisher->owner_node->context->session,
      *custom_publisher->owner_node->context->entity_creation_output,
      custom_publisher->publisher_id);

    uint16_t requests[] = {delete_writer, delete_publisher};
    uint8_t status[sizeof(requests) / 2];

    if (UXR_BEST_EFFORT_STREAM == custom_publisher->owner_node->context->entity_creation_output->type)
    {
      uxr_flash_output_streams(&custom_publisher->owner_node->context->session);
      rmw_uxrce_fini_publisher_memory(publisher);
      result_ret = RMW_RET_OK;
    }
    else
    {
      if (!uxr_run_session_until_all_status(
          &custom_publisher->owner_node->context->session, 1000, requests, status,
          sizeof(status)))
      {
        RMW_SET_ERROR_MSG("Unable to remove publisher from the server");
        result_ret = RMW_RET_ERROR;
      } 
      else 
      {
        rmw_uxrce_fini_publisher_memory(publisher);
        result_ret = RMW_RET_OK;
      }
    }
  }

  return result_ret;
}
