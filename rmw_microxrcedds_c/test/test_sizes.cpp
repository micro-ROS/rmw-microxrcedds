// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http (%d) ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

#include "rmw_microxrcedds_c/config.h"
#include "rmw_base_test.hpp"
#include "test_utils.hpp"

#include "../src/types.h"

/*
 * Testing client construction and destruction.
 */


TEST_F(RMWBaseTest, estimate_default_static_memory)
{
    unsigned long context_size = sizeof(rmw_context_impl_t);
    unsigned long topic_size = sizeof(rmw_uxrce_topic_t);
    unsigned long service_size = sizeof(rmw_uxrce_service_t);
    unsigned long client_size = sizeof(rmw_uxrce_client_t);
    unsigned long subscription_size = sizeof(rmw_uxrce_subscription_t);
    unsigned long publisher_size = sizeof(rmw_uxrce_publisher_t);
    unsigned long node_size = sizeof(rmw_uxrce_node_t);
    // unsigned long static_input_buffer_size = sizeof(rmw_uxrce_static_input_buffer_t);

    fprintf(stderr, "rmw_microxrcedds - static memory analisys\n");
    fprintf(stderr, "DEFAULT CONFIGURATION\n");
    fprintf(stderr, "---------------------\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "MTU :\t\t\t\t\t%d B\n", RMW_UXRCE_MAX_TRANSPORT_MTU);
    fprintf(stderr, "Input buffer size :\t\t\t%d B\n", RMW_UXRCE_MAX_INPUT_BUFFER_SIZE);
    fprintf(stderr, "Input history :\t\t\t\t%d\n", RMW_UXRCE_STREAM_HISTORY_INPUT);
    fprintf(stderr, "Output buffer size  :\t\t\t%d B\n", RMW_UXRCE_MAX_OUTPUT_BUFFER_SIZE);
    fprintf(stderr, "Output history :\t\t\t%d\n", RMW_UXRCE_STREAM_HISTORY_OUTPUT);
    fprintf(stderr, "\n");
    fprintf(stderr, "Context (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_SESSIONS, context_size);
    fprintf(stderr, "Topic (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_TOPICS_INTERNAL, topic_size);
    fprintf(stderr, "Service (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_SERVICES, service_size);
    fprintf(stderr, "Client (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_CLIENTS, client_size);
    fprintf(stderr, "Subscription (%d) :\t\t\t%ld B\n", RMW_UXRCE_MAX_SUBSCRIPTIONS, subscription_size);
    fprintf(stderr, "Publisher (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_PUBLISHERS, publisher_size);
    fprintf(stderr, "Node (%d) :\t\t\t\t%ld B\n", RMW_UXRCE_MAX_NODES, node_size);
    // fprintf(stderr, "Static input buffer (%d) :\t\t%ld B\n",RMW_UXRCE_MAX_HISTORY,static_input_buffer_size);



    unsigned long total =   RMW_UXRCE_MAX_SESSIONS * context_size +
            RMW_UXRCE_MAX_TOPICS_INTERNAL * topic_size +
            RMW_UXRCE_MAX_SERVICES * service_size +
            RMW_UXRCE_MAX_CLIENTS * client_size +
            RMW_UXRCE_MAX_SUBSCRIPTIONS * subscription_size +
            RMW_UXRCE_MAX_PUBLISHERS * publisher_size +
            RMW_UXRCE_MAX_NODES * node_size;
    // RMW_UXRCE_MAX_HISTORY * static_input_buffer_size;

    fprintf(stderr, "\n");
    fprintf(stderr, "TOTAL :\t\t\t\t\t%ld B\n", total);
}
