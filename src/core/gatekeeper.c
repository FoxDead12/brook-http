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

  brook_gatekeeper_node_t* root = brook_gatekeeper_create_node(NULL);

  for (int i = 0; i < array_size; i++) {
    cJSON *item = cJSON_GetArrayItem(json, i);
    if (!cJSON_IsObject(item)) continue;

    cJSON *route = cJSON_GetObjectItem(item, "route");
    if (!cJSON_IsString(route)) continue;

    // job
    cJSON *job = cJSON_GetObjectItem(item, "job");
    if (!cJSON_IsObject(job)) continue;

    // tube
    cJSON *tube = cJSON_GetObjectItem(job, "tube");
    if (!cJSON_IsString(tube)) continue;

    printf("Inserir route: %s -> %s\n", route->valuestring, tube->valuestring);
    brook_gatekeeper_insert_route(root, route->valuestring, tube->valuestring);
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
  n->child = NULL;
  n->sibling = NULL;
  return n;
}

int
brook_gatekeeper_insert_route ( brook_gatekeeper_node_t* root, const char* path, const char* tube ) {
  char tmp[PATH_MAX] = {0};
  strcpy(tmp, path);

  brook_gatekeeper_node_t* current = root;
  char* token = strtok(tmp, "/");

  while ( token ) {

    brook_gatekeeper_node_t *child = current->child;
    brook_gatekeeper_node_t *prev = NULL;

    while (child && strcmp(child->segment, token) != 0) {
      prev = child;
      child = child->sibling;
    }

    // se não existir, criar
    if (!child) {
      child = brook_gatekeeper_create_node(token);
      if (prev) {
        prev->sibling = child;
      } else {
        current->child = child;
      }
    }

    current = child;
    token = strtok(NULL, "/");

  }

  current->tube = strdup(tube);

}
