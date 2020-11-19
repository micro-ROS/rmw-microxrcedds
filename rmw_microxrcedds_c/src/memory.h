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

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct rmw_uxrce_mempool_item_t
{
  struct rmw_uxrce_mempool_item_t * prev;
  struct rmw_uxrce_mempool_item_t * next;
  void * data;
} rmw_uxrce_mempool_item_t;

typedef struct rmw_uxrce_mempool_t
{
  struct rmw_uxrce_mempool_item_t * allocateditems;
  struct rmw_uxrce_mempool_item_t * freeitems;

  size_t size;
  size_t element_size;
} rmw_uxrce_mempool_t;

void link_next(
  rmw_uxrce_mempool_item_t * current, rmw_uxrce_mempool_item_t * next,
  void * data);
void link_prev(
  rmw_uxrce_mempool_item_t * previous,
  rmw_uxrce_mempool_item_t * current, void * data);
void set_mem_pool(rmw_uxrce_mempool_t * mem, rmw_uxrce_mempool_item_t * first);
bool has_memory(rmw_uxrce_mempool_t * mem);
rmw_uxrce_mempool_item_t * get_memory(rmw_uxrce_mempool_t * mem);
void put_memory(rmw_uxrce_mempool_t * mem, rmw_uxrce_mempool_item_t * item);
void free_mem_pool(rmw_uxrce_mempool_t * mem);

#endif  // MEMORY_H_
