/*
 * Copyright 2018-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"){}
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kms_kv_list.h"
#include "kms_message.h"
#include "kms_private.h"

static void
kv_init (kms_kv_t *kv, kms_request_str_t *key, kms_request_str_t *value)
{
   kv->key = kms_request_str_dup (key);
   kv->value = kms_request_str_dup (value);
}

static void
kv_cleanup (kms_kv_t *kv)
{
   kms_request_str_destroy (kv->key);
   kms_request_str_destroy (kv->value);
}

kms_kv_list_t *
kms_kv_list_new (void)
{
   kms_kv_list_t *lst = malloc (sizeof (kms_kv_list_t));

   lst->size = 16;
   lst->kvs = malloc (lst->size * sizeof (kms_kv_t));
   lst->len = 0;

   return lst;
}

void
kms_kv_list_destroy (kms_kv_list_t *lst)
{
   size_t i;

   for (i = 0; i < lst->len; i++) {
      kv_cleanup (&lst->kvs[i]);
   }

   free (lst->kvs);
   free (lst);
}

void
kms_kv_list_add (kms_kv_list_t *lst,
                 kms_request_str_t *key,
                 kms_request_str_t *value)
{
   if (lst->len == lst->size) {
      lst->size *= 2;
      lst->kvs = realloc (lst->kvs, lst->size * sizeof (kms_kv_t));
   }

   kv_init (&lst->kvs[lst->len], key, value);
   ++lst->len;
}

static int
sort_kvs_cmp (const void *a, const void *b)
{
   return strcmp ((const char *) (((kms_kv_t *) a)->key->str),
                  (const char *) (((kms_kv_t *) b)->key->str));
}

kms_kv_list_t *
kms_kv_list_sorted (kms_kv_list_t *lst)
{
   kms_kv_list_t *dup;
   size_t i;

   if (lst->len == 0) {
      return kms_kv_list_new ();
   }

   dup = malloc (sizeof (kms_kv_list_t));
   dup->size = dup->len = lst->len;
   dup->kvs = malloc (lst->len * sizeof (kms_kv_t));

   for (i = 0; i < lst->len; i++) {
      kv_init (&dup->kvs[i], lst->kvs[i].key, lst->kvs[i].value);
   }

   qsort (dup->kvs, dup->len, sizeof (kms_kv_t), sort_kvs_cmp);

   return dup;
}