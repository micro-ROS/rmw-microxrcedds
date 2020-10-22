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

#include "utils.h"
#include "rmw_microxrcedds_topic.h"

#ifdef HAVE_C_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_c/identifier.h>
#endif
#ifdef HAVE_CPP_TYPESUPPORT
#include <rosidl_typesupport_microxrcedds_cpp/identifier.h>
#endif

#include <rmw/rmw.h>
#include <rmw/error_handling.h>
#include <rmw/types.h>
#include <rmw/allocators.h>

rmw_ret_t
rmw_init_subscription_allocation(
  const rosidl_message_type_support_t * type_support,
  const rosidl_runtime_c__Sequence__bound * message_bounds,
  rmw_subscription_allocation_t * allocation)
{
  (void) type_support;
  (void) message_bounds;
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_fini_subscription_allocation(rmw_subscription_allocation_t * allocation)
{
  (void) allocation;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_subscription_t *
rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_policies,
  const rmw_subscription_options_t * subscription_options)
{
  (void) subscription_options;

  EPROS_PRINT_TRACE()
  rmw_subscription_t * rmw_subscription = NULL;
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
  } else if (!type_support) {
    RMW_SET_ERROR_MSG("type support is null");
  } else if (!is_uxrce_rmw_identifier_valid(node->implementation_identifier)) {
    RMW_SET_ERROR_MSG("node handle not from this implementation");
  } else if (!topic_name || strlen(topic_name) == 0) {
    RMW_SET_ERROR_MSG("subscription topic is null or empty string");
    return NULL;
  } else if (!qos_policies) {
    RMW_SET_ERROR_MSG("qos_profile is null");
    return NULL;
  } else {
    rmw_subscription = (rmw_subscription_t *)rmw_allocate(
      sizeof(rmw_subscription_t));
    rmw_subscription->data = NULL;
    rmw_subscription->implementation_identifier = rmw_get_implementation_identifier();
    rmw_subscription->topic_name =
      (const char *)(rmw_allocate(sizeof(char) * (strlen(topic_name) + 1)));
    if (!rmw_subscription->topic_name) {
      RMW_SET_ERROR_MSG("failed to allocate memory");
      goto fail;
    }

    rmw_uxrce_node_t * custom_node = (rmw_uxrce_node_t *)node->data;
    struct rmw_uxrce_mempool_item_t * memory_node = get_memory(&subscription_memory);
    if (!memory_node) {
      RMW_SET_ERROR_MSG("Not available memory node");
      goto fail;
    }

    // TODO(Borja) micro_xrcedds_id is duplicated in subscriber_id and in subscription_gid.data
    rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)memory_node->data;
    custom_subscription->rmw_handle = rmw_subscription;

    custom_subscription->owner_node = custom_node;
    custom_subscription->subscription_gid.implementation_identifier =
      rmw_get_implementation_identifier();
    custom_subscription->micro_buffer_in_use = false;
    memcpy(&custom_subscription->qos, qos_policies, sizeof(rmw_qos_profile_t));

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

    custom_subscription->type_support_callbacks =
      (const message_type_support_callbacks_t *)type_support_xrce->data;

    if (custom_subscription->type_support_callbacks == NULL) {
      RMW_SET_ERROR_MSG("type support data is NULL");
      goto fail;
    } else if (sizeof(uxrObjectId) > RMW_GID_STORAGE_SIZE) {
      RMW_SET_ERROR_MSG("Not enough memory for impl ids");
      goto fail;
    }

    memset(custom_subscription->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
    memcpy(
      custom_subscription->subscription_gid.data, &custom_subscription->subscriber_id,
      sizeof(uxrObjectId));


    custom_subscription->topic = create_topic(
      custom_node, topic_name,
      custom_subscription->type_support_callbacks, qos_policies);
    if (custom_subscription->topic == NULL) {
      goto fail;
    }

    custom_subscription->subscriber_id = uxr_object_id(
      custom_node->context->id_subscriber++,
      UXR_SUBSCRIBER_ID);
    uint16_t subscriber_req = UXR_INVALID_REQUEST_ID;

#ifdef MICRO_XRCEDDS_USE_XML
    char subscriber_name[20];
    generate_name(&custom_subscription->subscriber_id, subscriber_name, sizeof(subscriber_name));
    if (!build_subscriber_xml(subscriber_name, rmw_uxrce_xml_buffer, sizeof(rmw_uxrce_xml_buffer))) {
      RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
      goto fail;
    }
    subscriber_req = uxr_buffer_create_subscriber_xml(
      &custom_node->context->session,
      *custom_node->context->entity_creation_output, 
      custom_subscription->subscriber_id,
      custom_node->participant_id, 
      rmw_uxrce_xml_buffer, 
      UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
    subscriber_req = uxr_buffer_create_subscriber_xml(
      &custom_node->context->session,
      *custom_node->context->entity_creation_output, 
      custom_subscription->subscriber_id,
      custom_node->participant_id, 
      "", 
      UXR_REPLACE);
#endif


    custom_subscription->datareader_id = uxr_object_id(
      custom_node->context->id_datareader++,
      UXR_DATAREADER_ID);
    uint16_t datareader_req = UXR_INVALID_REQUEST_ID;

#ifdef MICRO_XRCEDDS_USE_XML
    if (!build_datareader_xml(
        topic_name, custom_subscription->type_support_callbacks,
        qos_policies, rmw_uxrce_xml_buffer,
        sizeof(rmw_uxrce_xml_buffer)))
    {
      RMW_SET_ERROR_MSG("failed to generate xml request for subscriber creation");
      goto fail;
    }

    datareader_req = uxr_buffer_create_datareader_xml(
      &custom_node->context->session,
      *custom_node->context->entity_creation_output, 
      custom_subscription->datareader_id,
      custom_subscription->subscriber_id, 
      rmw_uxrce_xml_buffer, 
      UXR_REPLACE);
#elif defined(MICRO_XRCEDDS_USE_REFS)
    if (!build_datareader_profile(topic_name, rmw_uxrce_profile_name, sizeof(rmw_uxrce_profile_name))) {
      RMW_SET_ERROR_MSG("failed to generate ref request for datareader creation");
      goto fail;
    }

    datareader_req = uxr_buffer_create_datareader_ref(
      &custom_node->context->session,
      *custom_node->context->entity_creation_output, 
      custom_subscription->datareader_id,
      custom_subscription->subscriber_id, 
      rmw_uxrce_profile_name, 
      UXR_REPLACE);
#endif

    rmw_subscription->data = custom_subscription;

    uint16_t requests[] = {subscriber_req, datareader_req};
    uint8_t status[sizeof(requests) / 2];

    if (UXR_BEST_EFFORT_STREAM == custom_node->context->entity_creation_output->type)
    {
      uxr_flash_output_streams(&custom_node->context->session);
    }
    else if(!uxr_run_session_until_all_status(&custom_node->context->session, 1000, requests, status,sizeof(status)))
    {
      RMW_SET_ERROR_MSG("Issues creating Subscriber Micro XRCE-DDS entities");
      put_memory(&subscription_memory, &custom_subscription->mem);
      goto fail;
    }

    uxrDeliveryControl delivery_control;
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    delivery_control.min_pace_period = 0;
    delivery_control.max_elapsed_time = UXR_MAX_ELAPSED_TIME_UNLIMITED;
    delivery_control.max_bytes_per_second = UXR_MAX_BYTES_PER_SECOND_UNLIMITED;

    custom_subscription->stream_id =
      (qos_policies->reliability == RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT) ?
      custom_node->context->best_effort_input :
      custom_node->context->reliable_input;

    custom_subscription->subscription_request = uxr_buffer_request_data(
      &custom_node->context->session,
      *custom_node->context->entity_creation_output, 
      custom_subscription->datareader_id,
      custom_subscription->stream_id, &delivery_control);
  }
  return rmw_subscription;

fail:
  rmw_uxrce_fini_subscription_memory(rmw_subscription);
  rmw_subscription = NULL;
  return rmw_subscription;
}

rmw_ret_t
rmw_subscription_count_matched_publishers(
  const rmw_subscription_t * subscription,
  size_t * publisher_count)
{
  (void) subscription;
  (void) publisher_count;
  RMW_SET_ERROR_MSG("function not implemented");
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t
rmw_subscription_get_actual_qos(
  const rmw_subscription_t * subscription,
  rmw_qos_profile_t * qos)
{
  (void) qos;

  rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscription->data;
  qos = &custom_subscription->qos;

  return RMW_RET_OK;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
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
  } else if (!subscription) {
    RMW_SET_ERROR_MSG("subscription handle is null");
    result_ret = RMW_RET_ERROR;
  } else if (!is_uxrce_rmw_identifier_valid(subscription->implementation_identifier)) {
    RMW_SET_ERROR_MSG("subscription handle not from this implementation");
    result_ret = RMW_RET_ERROR;
  } else if (!subscription->data) {
    RMW_SET_ERROR_MSG("subscription imp is null");
    result_ret = RMW_RET_ERROR;
  } else {
    rmw_uxrce_subscription_t * custom_subscription = (rmw_uxrce_subscription_t *)subscription->data;

    destroy_topic(custom_subscription->topic);

    uint16_t delete_datareader =
      uxr_buffer_delete_entity(
      &custom_subscription->owner_node->context->session,
      *custom_subscription->owner_node->context->entity_creation_output,
      custom_subscription->datareader_id);
    uint16_t delete_subscriber =
      uxr_buffer_delete_entity(
      &custom_subscription->owner_node->context->session,
      *custom_subscription->owner_node->context->entity_creation_output,
      custom_subscription->subscriber_id);

    uint16_t requests[] = {delete_datareader, delete_subscriber};
    uint8_t status[sizeof(requests) / 2];

    if (UXR_BEST_EFFORT_STREAM == custom_subscription->owner_node->context->entity_creation_output->type)
    {
      uxr_flash_output_streams(&custom_subscription->owner_node->context->session);
      rmw_uxrce_fini_subscription_memory(subscription);
    }
    else if (!uxr_run_session_until_all_status(
        &custom_subscription->owner_node->context->session, 1000, requests, status,
        sizeof(status)))
    {
      RMW_SET_ERROR_MSG("Unable to remove subscriber from the server");
      result_ret = RMW_RET_ERROR;
    } else {
      rmw_uxrce_fini_subscription_memory(subscription);
      result_ret = RMW_RET_OK;
    }
  }

  return result_ret;
}
