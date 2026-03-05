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
  printf("Array tem %d elementos\n", array_size);

  config->root = brook_gatekeeper_create_node(NULL);

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

    printf("Inserir route: %s -> %s\n", route->valuestring, tube->valuestring);
    brook_gatekeeper_insert_route(config->root, route->valuestring, tube->valuestring, method_mask);
  }

  cJSON_Delete(json);
  free(buffer);

  return BROOK_OK;
}

brook_gatekeeper_node_t*
brook_gatekeeper_create_node ( const char* segment ) {
  brook_gatekeeper_node_t* n = malloc(sizeof(brook_gatekeeper_node_t));
  n->segment = segment ? strdup(segment) : NULL;
  n->tube = NULL;
  n->methods_mask = 0;
  n->child = NULL;
  n->sibling = NULL;
  return n;
}

/**
 * Will insert a new node in tree
 * the tree is sequencial each node contain one child and one sibling
 * This system will be simple only compare the exact url, will assume url are static
 * if is necessary variables need be sended in params of url
 */
int
brook_gatekeeper_insert_route ( brook_gatekeeper_node_t* root, const char* path, const char* tube, uint32_t method_mask ) {

  char tmp[PATH_MAX] = {0};
  strcpy(tmp, path);

  brook_gatekeeper_node_t* current = root;
  char* token = strtok(tmp, "/");

  while ( token ) {

    // ... variables to manager if we move down in tree ...
    brook_gatekeeper_node_t *child = current->child;
    brook_gatekeeper_node_t *prev = NULL;

    // ... will change to siblings if necessary ...
    while (child && strcmp(child->segment, token) != 0) {
      prev = child;
      child = child->sibling;
    }

    if (!child) {
      // ... create new node ...
      child = brook_gatekeeper_create_node(token);

      if (prev) {
        // ... add children to a sibling (we move to rigth in tree)
        prev->sibling = child;
      } else {
        // ... we only move down in tree, so keep flow
        current->child = child;
      }
    }

    current = child;
    token = strtok(NULL, "/");
  }

  // ... already parse all url, we are in new item ...
  current->tube = strdup(tube);
  current->methods_mask = method_mask;

  return BROOK_OK;
}

const char*
brook_gatekeeper_match_route ( brook_gatekeeper_node_t *root, const char *path ) {
  char temp[PATH_MAX];
  strcpy(temp, path);

  brook_gatekeeper_node_t *current = root;
  char *token = strtok(temp, "/");

  while (token && current) {
    brook_gatekeeper_node_t *child = current->child;

    // procurar segmento correspondente
    while (child && strcmp(child->segment, token) != 0) {
      child = child->sibling;
    }

    if (!child) return NULL;

    current = child;
    token = strtok(NULL, "/");
  }

  if (current) return current->tube;

  return NULL;
}
