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

#include "./test_utils.hpp"

void ConfigureDummyTypeSupport(
  const char * message_name,
  const char * package_name,
  rosidl_message_type_support_t * dummy_type_support,
  message_type_support_callbacks_t * dummy_callbacks)
{
  dummy_callbacks->message_name_ = message_name;
  dummy_callbacks->package_name_ = package_name;
  dummy_callbacks->cdr_serialize = [](const void * untyped_ros_message, ucdrBuffer * cdr) {
      return true;
    };
  dummy_callbacks->cdr_deserialize =
    [](ucdrBuffer * cdr, void * untyped_ros_message, uint8_t * raw_mem_ptr, size_t raw_mem_size) {
      return true;
    };
  dummy_callbacks->get_serialized_size = [](const void *) {return (uint32_t)0;};
  dummy_callbacks->max_serialized_size = [](bool full_bounded) {return (size_t)0;};

  dummy_type_support->typesupport_identifier =
    ROSIDL_TYPESUPPORT_MICROXRCEDDS_C__IDENTIFIER_VALUE;
  dummy_type_support->data = dummy_callbacks;
  dummy_type_support->func =
    [](const rosidl_message_type_support_t * type_support, const char * id) {
      return type_support;
    };
}

void ConfigureDefaultQOSPolices(rmw_qos_profile_t * dummy_qos_policies)
{
  dummy_qos_policies->avoid_ros_namespace_conventions = false;
  dummy_qos_policies->depth = 0;

  // durability options:
  //  RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT
  //  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
  //  RMW_QOS_POLICY_DURABILITY_VOLATILE
  dummy_qos_policies->durability = RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT;

  // history options:
  //  RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT
  //  RMW_QOS_POLICY_HISTORY_KEEP_LAST
  //  RMW_QOS_POLICY_HISTORY_KEEP_ALL
  dummy_qos_policies->history = RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT;

  // reliability options:
  //  RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT
  //  RMW_QOS_POLICY_RELIABILITY_RELIABLE
  //  RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT
  dummy_qos_policies->reliability = RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT;
}

bool CheckErrorState()
{
  bool ok = true;

  const rcutils_error_state_t * error_state;
  error_state = rcutils_get_error_state();

  ok &= error_state->file != NULL;
  ok &= error_state->line_number != 0;
  ok &= error_state->message != NULL;

  // if (ok) std::cout << error_state->file
  //  << ":" << error_state->line_number << " -> " << error_state->message << std::endl;

  return ok;
}
