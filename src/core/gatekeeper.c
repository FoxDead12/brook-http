#include "core/gatekeeper.h"

int
brook_gatekeeper_load ( brook_conf_t* config ) {

  const char* file_name = "gatekeeper.json";
  const char* file_path = "config";

  char current_path[PATH_MAX] = {0};
  char full_path[PATH_MAX] = {0};

  // ... get current path of work ...
  if ( getcwd(current_path, sizeof(current_path)) == NULL ) {
    perror("getcwd() error");
    return 1;
  }

  // ... build path to gatekeeper.json ...
  snprintf(full_path, sizeof(full_path), "%s/%s/%s", current_path, file_path, file_name);

  // ... read file ...
  FILE* gatekeeper = fopen(full_path, "r");
  if ( !gatekeeper ) {
    perror("Erro ao abrir ficheiro");
    return 1;
  }

  // ... get size of file to load in memory ...
  fseek(gatekeeper, 0, SEEK_END);
  long size = ftell(gatekeeper);
  rewind(gatekeeper);

  // ...
  char* buffer = malloc(size);
  fread(buffer, 1, size, gatekeeper);
  fclose(gatekeeper);

  // ... parse JSON ...
  cJSON* json = cJSON_Parse(buffer);
  if ( !json ) {
    printf("Erro no parse JSON\n");
    free(buffer);
    return 1;
  }

  if (!cJSON_IsArray(json)) {
    printf("O JSON não é um array!\n");
    cJSON_Delete(json);
    free(buffer);
    return 1;
  }

  int array_size = cJSON_GetArraySize(json);

  for (int i = 0; i < array_size; i++) {
    cJSON *item = cJSON_GetArrayItem(json, i);
    if (!cJSON_IsObject(item)) continue;

    // methods
    cJSON *methods = cJSON_GetObjectItem(item, "method");
    if (!cJSON_IsArray(methods)) continue;

    // method
    cJSON *route = cJSON_GetObjectItem(item, "route");
    if (!cJSON_IsString(route)) continue;

    // job
    cJSON *job = cJSON_GetObjectItem(item, "job");
    if (!cJSON_IsObject(job)) continue;

    // tube
    cJSON *tube = cJSON_GetObjectItem(job, "tube");
    if (!cJSON_IsString(tube)) continue;

    uint32_t method_mask = 0;
    cJSON *method = NULL;

    cJSON_ArrayForEach(method, methods) {
      if ( !cJSON_IsString(method) ) continue;
      if (strcmp(method->valuestring, "DELETE") == 0) {
        method_mask |= (1 << DELETE);
      } else if (strcmp(method->valuestring, "GET") == 0) {
        method_mask |= (1 << GET);
      } else if (strcmp(method->valuestring, "POST") == 0) {
        method_mask |= (1 << POST);
      } else if (strcmp(method->valuestring, "PUT") == 0) {
        method_mask |= (1 << PUT);
      } else if (strcmp(method->valuestring, "PATCH") == 0) {
        method_mask |= (1 << PATCH);
      }
    }

    config->root = brook_gatekeeper_insert_route(config->root, route->valuestring, tube->valuestring, method_mask);
  }

  cJSON_Delete(json);
  free(buffer);

  return BROOK_OK;
}

brook_gatekeeper_node_t*
brook_gatekeeper_create_node ( const char* path, const char* tube, uint32_t method_mask ) {

  brook_gatekeeper_node_t* node = malloc(sizeof(brook_gatekeeper_node_t));
  node->methods_mask = method_mask;

  node->url.data = strdup(path);
  node->url.len = strlen(path);

  node->tube.data = strdup(tube);
  node->tube.len = strlen(tube);

  node->left = NULL;
  node->rigth = NULL;
  return node;
}

/**
 * Will insert a new node in tree
 * the tree is sequencial each node contain one child and one sibling
 * This system will be simple only compare the exact url, will assume url are static
 * if is necessary variables need be sended in params of url
 */
brook_gatekeeper_node_t*
brook_gatekeeper_insert_route ( brook_gatekeeper_node_t* root, const char* path, const char* tube, uint32_t method_mask ) {

  if ( root == NULL ) {
    return brook_gatekeeper_create_node(path, tube, method_mask);
  }

  int res = strncmp(root->url.data, path, root->url.len);

  if ( res < 0 ) {
    root->left = brook_gatekeeper_insert_route(root->left, path, tube, method_mask);
  } else if ( res > 0 ) {
    root->rigth = brook_gatekeeper_insert_route(root->rigth, path, tube, method_mask);
  } else {
    root->methods_mask |= method_mask;
  }

  return root;
}

brook_gatekeeper_node_t*
brook_gatekeeper_match_route ( brook_gatekeeper_node_t *root, brook_str_t url, uint32_t method ) {
  if ( root == NULL ) {
    return NULL;
  }

  int res = 0;
  if ( root->url.len > url.len ) {
    // ... this case problably don't crash because is inside a buffer
    res = strncmp(root->url.data, url.data, root->url.len);
  } else {
    res = strncmp(root->url.data, url.data, url.len);
  }

  if ( res == 0 ) {
    if ( root->methods_mask & (1 << method) ) {
      return root;
    } else {
      return NULL;
    }
  } else if ( res < 0 ) {
    return brook_gatekeeper_match_route(root->left, url, method);
  } else {
    return brook_gatekeeper_match_route(root->rigth, url, method);
  }
}
