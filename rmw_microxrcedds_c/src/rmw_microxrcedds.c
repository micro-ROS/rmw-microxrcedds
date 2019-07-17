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

#include "rmw_microxrcedds_c/rmw_microxrcedds.h"  // NOLINT

#include <limits.h>

#include <uxr/client/client.h>
#include <rosidl_typesupport_microxrcedds_shared/identifier.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"

#include "./identifiers.h"

#include "./rmw_node.h"
#include "./rmw_publisher.h"
#include "./rmw_subscriber.h"
#include "./types.h"
#include "./utils.h"
