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

#include "./utils.h"  // NOLINT

#include "rmw/allocators.h"

#include "./types.h"
#include "./rmw_microxrcedds_topic.h"

static const char ros_topic_prefix[] = "rt";
static const char ros_request_prefix[] = "rq";
static const char ros_reply_prefix[] =  "rr";
static const char ros_request_subfix[] = "Request";
static const char ros_reply_subfix[] =  "Reply";

void custompublisher_clear(CustomPublisher * publisher);
void customsubscription_clear(CustomSubscription * subscription);

void rmw_delete(void * rmw_allocated_ptr)
{
  rmw_free(rmw_allocated_ptr);
  rmw_allocated_ptr = NULL;
}

void rmw_node_delete(rmw_node_t * node)
{
  if (node->namespace_) {
    rmw_delete((char *)node->namespace_);
  }
  if (node->name) {
    rmw_delete((char *)node->name);
  }
  if (node->implementation_identifier) {
    node->implementation_identifier = NULL;
  }
  if (node->data) {
    customnode_clear((CustomNode *)node->data);
    node->data = NULL;
  }

  rmw_node_free(node);
  node = NULL;
}

void rmw_publisher_delete(rmw_publisher_t * publisher)
{
  if (publisher->implementation_identifier) {
    publisher->implementation_identifier = NULL;
  }
  if (publisher->topic_name) {
    rmw_delete((char *)publisher->topic_name);
  }
  if (publisher->data) {
    CustomPublisher * custom_publisher =
      (CustomPublisher *)publisher->data;

    if (custom_publisher->topic != NULL) {
      destroy_topic(custom_publisher->topic);
    }

    put_memory(&custom_publisher->owner_node->publisher_mem,
      &custom_publisher->mem);

    custompublisher_clear((CustomPublisher *)publisher->data);
    publisher->data = NULL;
  }

  rmw_delete(publisher);
}

void custompublisher_clear(CustomPublisher * publisher)
{
  if (publisher) {
    memset(&publisher->publisher_id, 0, sizeof(uxrObjectId));
    memset(&publisher->datawriter_id, 0, sizeof(uxrObjectId));
    memset(&publisher->topic_id, 0, sizeof(uxrObjectId));
    publisher->publisher_gid.implementation_identifier = NULL;
    memset(&publisher->publisher_gid.data, 0, RMW_GID_STORAGE_SIZE);
    publisher->type_support_callbacks = NULL;
  }
}

void publishers_clear(CustomPublisher publishers[MAX_PUBLISHERS_X_NODE])
{
  for (size_t i = 0; i < MAX_PUBLISHERS_X_NODE; i++) {
    custompublisher_clear(&publishers[i]);
  }
}

void rmw_subscription_delete(rmw_subscription_t * subscriber)
{
  if (subscriber->implementation_identifier) {
    subscriber->implementation_identifier = NULL;
  }
  if (subscriber->topic_name) {
    rmw_delete((char *)subscriber->topic_name);
  }
  if (subscriber->data) {
    CustomSubscription * custom_Subscription =
      (CustomSubscription *)subscriber->data;

    if (custom_Subscription->topic != NULL) {
      destroy_topic(custom_Subscription->topic);
    }

    put_memory(&custom_Subscription->owner_node->subscription_mem,
      &custom_Subscription->mem);

    customsubscription_clear((CustomSubscription *)subscriber->data);
    subscriber->data = NULL;
  }
  rmw_delete(subscriber);
}

void customsubscription_clear(CustomSubscription * subscription)
{
  if (subscription) {
    memset(&subscription->subscriber_id, 0, sizeof(uxrObjectId));
    memset(&subscription->datareader_id, 0, sizeof(uxrObjectId));
    memset(&subscription->topic_id, 0, sizeof(uxrObjectId));
    subscription->subscription_gid.implementation_identifier = NULL;
    memset(&subscription->subscription_gid.data, 0, RMW_GID_STORAGE_SIZE);
    subscription->type_support_callbacks = NULL;
  }
}

void subscriptions_clear(CustomSubscription subscriptions[MAX_SUBSCRIPTIONS_X_NODE])
{
  for (size_t i = 0; i < MAX_SUBSCRIPTIONS_X_NODE; i++) {
    customsubscription_clear(&subscriptions[i]);
  }
}

void customservice_clear(CustomService * service)
{
  if (service) {
    memset(&service->service_id, 0, sizeof(uxrObjectId));
    service->type_support_callbacks = NULL;
    service->history_write_index = 0;
    service->history_read_index = 0;
    service->micro_buffer_in_use = false;
    service->service_gid.implementation_identifier = NULL;
    memset(&service->service_gid.data, 0, RMW_GID_STORAGE_SIZE);
  }
}

void rmw_service_delete(rmw_service_t * service)
{
  if (service->implementation_identifier) {
    service->implementation_identifier = NULL;
  }
  if (service->service_name) {
    rmw_delete((char *)service->service_name);
  }
  if (service->data) {
    CustomService * custom_service =
      (CustomService *)service->data;

    put_memory(&custom_service->owner_node->service_mem,
      &custom_service->mem);

    customservice_clear((CustomService *)service->data);
    service->data = NULL;
  }
  rmw_delete(service);
}

void customclient_clear(CustomClient * client)
{
  if (client) {
    memset(&client->client_id, 0, sizeof(uxrObjectId));
    client->type_support_callbacks = NULL;
    client->history_write_index = 0;
    client->history_read_index = 0;
    client->micro_buffer_in_use = false;
    client->client_gid.implementation_identifier = NULL;
    memset(&client->client_gid.data, 0, RMW_GID_STORAGE_SIZE);
  }
}

void rmw_client_delete(rmw_client_t * client)
{
  if (client->implementation_identifier) {
    client->implementation_identifier = NULL;
  }
  if (client->service_name) {
    rmw_delete((char *)client->service_name);
  }
  if (client->data) {
    CustomClient * custom_client =
      (CustomClient *)client->data;

    put_memory(&custom_client->owner_node->client_mem,
      &custom_client->mem);

    customclient_clear((CustomClient *)client->data);
    client->data = NULL;
  }
  rmw_delete(client);
}

void customnode_clear(CustomNode * node)
{
  if (node) {
    publishers_clear(node->publisher_info);
    free_mem_pool(&node->publisher_mem);
    subscriptions_clear(node->subscription_info);
    free_mem_pool(&node->subscription_mem);
  }
}

int build_participant_xml(
  size_t domain_id, const char * participant_name, char xml[],
  size_t buffer_size)
{
  (void)domain_id;
  static const char format[] =
    "<dds>"
    "<participant>"
    "<rtps>"
    "<name>%s</name>"
    "</rtps>"
    "</participant>"
    "</dds>";

  int ret = snprintf(xml, buffer_size, format, participant_name);
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

int build_service_xml(const char * service_name_id, const char * service_name, bool requester,  const service_type_support_callbacks_t * members,
 const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size)
{
  int ret;

  static const char format[] =  "<dds>"
                                "<%s profile_name=\"%s\" "
                                         "service_name=\"%s\" "
                                         "request_type=\"%s\" "
                                         "reply_type=\"%s\">"
                                    "<request_topic_name>%s</request_topic_name>"
                                    "<reply_topic_name>%s</reply_topic_name>"
                                "</%s>"
                                "</dds>";

  // Retrive request and response types
  const rosidl_message_type_support_t * req_members = members->request_members_();
  const rosidl_message_type_support_t * res_members = members->response_members_();

  const message_type_support_callbacks_t * req_callbacks = (const message_type_support_callbacks_t *)req_members->data;
  const message_type_support_callbacks_t * res_callbacks = (const message_type_support_callbacks_t *)res_members->data;

  
  static char req_type_name_buffer[RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH];
  static char res_type_name_buffer[RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH];

  generate_type_name(req_callbacks,req_type_name_buffer,RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH);
  generate_type_name(res_callbacks,res_type_name_buffer,RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH);

  // Generate request and reply topic names
  char req_full_topic_name[RMW_MICROXRCEDDS_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_request_prefix) + 1 + sizeof(ros_request_subfix)];
  req_full_topic_name[0] = '\0';

  char res_full_topic_name[RMW_MICROXRCEDDS_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_reply_prefix) + 1 + sizeof(ros_reply_subfix)];
  res_full_topic_name[0] = '\0';

  if (!qos_policies->avoid_ros_namespace_conventions) {
    ret = snprintf(req_full_topic_name, sizeof(req_full_topic_name), "%s%s%s", ros_request_prefix,
        service_name,ros_request_subfix);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }
      
    ret = snprintf(res_full_topic_name, sizeof(res_full_topic_name), "%s%s%s", ros_reply_prefix,
        service_name,ros_reply_subfix);
    if ((ret < 0) || (ret >= (int)sizeof(res_full_topic_name))) {
      return 0;
    }
  } else {
    ret = snprintf(req_full_topic_name, sizeof(req_full_topic_name), "%s", service_name);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }
    ret = snprintf(res_full_topic_name, sizeof(res_full_topic_name), "%s", service_name);
    if ((ret < 0) || (ret >= (int)sizeof(req_full_topic_name))) {
      return 0;
    }
  }

    
  ret = snprintf(xml, buffer_size, format, 
                  requester ? "requester" : "replier", 
                  service_name_id, 
                  service_name, 
                  req_type_name_buffer, 
                  res_type_name_buffer,
                  req_full_topic_name,
                  res_full_topic_name,
                  requester ? "requester" : "replier"
                  );
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

int build_publisher_xml(const char * publisher_name, char xml[], size_t buffer_size)
{
  static const char format[] = "<publisher name=\"%s\">";
  int ret = snprintf(xml, buffer_size, format, publisher_name);
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

int build_subscriber_xml(const char * subscriber_name, char xml[], size_t buffer_size)
{
  static const char format[] = "<subscriber name=\"%s\">";

  int ret = snprintf(xml, buffer_size, format, subscriber_name);
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

int generate_name(const uxrObjectId * id, char name[], size_t buffer_size)
{
  static const char format[] = "%hu_%hi";

  int ret = snprintf(name, buffer_size, format, id->id, id->type);
  if ((ret < 0) && (ret >= (int)buffer_size)) {
    ret = 0;
  }

  return ret;
}

size_t generate_type_name(
  const message_type_support_callbacks_t * members, char type_name[],
  size_t buffer_size)
{
  static const char* sep = "::";
  static const char* protocol = "dds";
  static const char* suffix = "_";
  size_t ret = 0;
  size_t full_name_size = strlen(protocol) + strlen(suffix) + strlen(sep) + strlen(members->message_name_) + strlen(suffix) + ((NULL != members->message_namespace_) ? strlen(members->message_namespace_):0) + 1;
  type_name[0] = 0;

  if (full_name_size < buffer_size)
  {
      if (NULL != members->message_namespace_)
      {
        strcat(type_name, members->message_namespace_);
        strcat(type_name, sep);
      }
      strcat(type_name, protocol);
      strcat(type_name, suffix);
      strcat(type_name, sep);
      strcat(type_name, members->message_name_);
      strcat(type_name, suffix);
      ret = full_name_size;
  }

  return ret;
}

int build_topic_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size)
{
  static const char format[] =
    "<dds>"
    "<topic>"
    "<name>%s</name>"
    "<dataType>%s</dataType>"
    "</topic>"
    "</dds>";

  int ret = 0;
  static char type_name_buffer[RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH];

  if (RMW_MICROXRCEDDS_TOPIC_NAME_MAX_NAME_LENGTH >= strlen(topic_name) &&
    0 != generate_type_name(members, type_name_buffer, sizeof(type_name_buffer)))
  {
    char full_topic_name[RMW_MICROXRCEDDS_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_topic_prefix)];

    if (!qos_policies->avoid_ros_namespace_conventions) {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
          topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    } else {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    }

    ret = snprintf(xml, buffer_size, format, full_topic_name, type_name_buffer);
    if ((ret < 0) && (ret >= (int)buffer_size)) {
      ret = 0;
    }
  }

  return ret;
}

int build_xml(
  const char * format, const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size)
{
  int ret = 0;
  static char type_name_buffer[RMW_MICROXRCEDDS_TYPE_NAME_MAX_NAME_LENGTH];

  if (0 != generate_type_name(members, type_name_buffer, sizeof(type_name_buffer))) {
    char full_topic_name[RMW_MICROXRCEDDS_TOPIC_NAME_MAX_NAME_LENGTH + 1 + sizeof(ros_topic_prefix)];
    full_topic_name[0] = '\0';

    if (!qos_policies->avoid_ros_namespace_conventions) {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s%s", ros_topic_prefix,
          topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    } else {
      ret = snprintf(full_topic_name, sizeof(full_topic_name), "%s", topic_name);
      if ((ret < 0) && (ret >= (int)buffer_size)) {
        return 0;
      }
    }

    ret = snprintf(xml, buffer_size, format, full_topic_name, type_name_buffer);
    if ((ret < 0) && (ret >= (int)buffer_size)) {
      ret = 0;
    }
  }

  return ret;
}
int build_datawriter_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size)
{
  static const char format[] =
    "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>%s</name>"
    "<dataType>%s</dataType>"
    "<historyQos>"
    "<kind>KEEP_ALL</kind>"
    "</historyQos>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  return build_xml(format, topic_name, members, qos_policies, xml, buffer_size);
}

int build_datareader_xml(
  const char * topic_name, const message_type_support_callbacks_t * members,
  const rmw_qos_profile_t * qos_policies, char xml[], size_t buffer_size)
{
  static const char format[] =
    "<dds>"
    "<data_reader>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>%s</name>"
    "<dataType>%s</dataType>"
    "<historyQos>"
    "<kind>KEEP_ALL</kind>"
    "</historyQos>"
    "</topic>"
    "</data_reader>"
    "</dds>";
  return build_xml(format, topic_name, members, qos_policies, xml, buffer_size);
}

bool build_participant_profile(char profile_name[], size_t buffer_size)
{
  static const char profile[] = "participant_profile";
  bool ret = false;
  if (buffer_size >= sizeof(profile)) {
    memcpy(profile_name, profile, sizeof(profile));
    ret = true;
  }
  return ret;
}

bool build_topic_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_t";
  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}

bool build_datawriter_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_p";
  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}

bool build_datareader_profile(const char * topic_name, char profile_name[], size_t buffer_size)
{
  const char * const format = "%s_s";
  topic_name++;
  bool ret = false;
  int written = snprintf(profile_name, buffer_size, format, topic_name);
  ret = (written > 0) && (written < (int)buffer_size);
  return ret;
}
