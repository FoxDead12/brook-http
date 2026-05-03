#include "core/gatekeeper.h"

int
brook_gatekeeper_load ( brook_conf_t* config ) {

  // ... read file ...
  FILE* gatekeeper = fopen(config->gatekeeper, "r");
  if ( !gatekeeper ) {
    perror("Erro ao abrir ficheiro");
    brook_log(config, LOG_ERR, "File open failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
    return 1;
  }

  // ... get size of file to load in memory ...
  fseek(gatekeeper, 0, SEEK_END);
  long size = ftell(gatekeeper);
  rewind(gatekeeper);

  // ... alloc memory to store all file content ...
  char* buffer = malloc(size + 1);
  if (!buffer) {
    brook_log(config, LOG_ERR, "Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
    return BROOK_ERROR;
  }

  size_t read_n = fread(buffer, 1, size, gatekeeper);

  fclose(gatekeeper);

  buffer[read_n] = '\0';

  // ... parse JSON ...
  cJSON* json = cJSON_Parse(buffer);
  if ( !json ) {
    //printf("Erro no parse JSON\n");
    free(buffer);
    return 1;
  }

  if (!cJSON_IsArray(json)) {
    //printf("O JSON não é um array!\n");
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

    // role_mask
    int role_mask = 0;
    cJSON *r = cJSON_GetObjectItem(item, "role_mask");
    if (cJSON_IsString(r) && (r->valuestring != NULL)) {
      role_mask = (int) strtol(r->valuestring, NULL, 16);  // ... convert base 16 to integer
    }

    // product key, identifier of web app route is defined
    const char* product_key = "NO_PRODUCT_KEY";     // ... default product key ...
    cJSON *product_key_j = cJSON_GetObjectItem(item, "product_key");
    if (product_key_j) {
      product_key = product_key_j->valuestring;
    }


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

    config->root = brook_gatekeeper_insert_route(config->root, route->valuestring, tube->valuestring, product_key, method_mask, role_mask);
  }

  cJSON_Delete(json);
  free(buffer);

  return BROOK_OK;
}

void
brook_gatekeeper_free (brook_gatekeeper_node_t* root) {
  if (root == NULL) {
    return;
  }

  brook_gatekeeper_free(root->left);
  brook_gatekeeper_free(root->rigth);

  if (root->url.data) {
    free(root->url.data);
  }

  if (root->tube.data) {
    free(root->tube.data);
  }

  if (root->product_key.data) {
    free(root->product_key.data);
  }

  free(root);
}

brook_gatekeeper_node_t*
brook_gatekeeper_create_node ( const char* path, const char* tube, const char* product_key, uint32_t method_mask, int role_mask ) {

  brook_gatekeeper_node_t* node = malloc(sizeof(brook_gatekeeper_node_t));
  node->methods_mask = method_mask;
  node->role_mask = role_mask;

  node->url.data = (unsigned char*) strdup(path);
  node->url.len = strlen(path);

  node->tube.data = (unsigned char*) strdup(tube);
  node->tube.len = strlen(tube);

  node->product_key.data = (unsigned char*) strdup(product_key);
  node->product_key.len = strlen(product_key);

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
brook_gatekeeper_insert_route ( brook_gatekeeper_node_t* root, const char* path, const char* tube, const char* product_key, uint32_t method_mask, int role_mask ) {

  if ( root == NULL ) {
    return brook_gatekeeper_create_node(path, tube, product_key, method_mask, role_mask);
  }

  int res = strncmp((char*) root->url.data, path, root->url.len);

  if ( res < 0 ) {
    root->left = brook_gatekeeper_insert_route(root->left, path, tube, product_key, method_mask, role_mask);
  } else if ( res > 0 ) {
    root->rigth = brook_gatekeeper_insert_route(root->rigth, path, tube, product_key, method_mask, role_mask);
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
    res = strncmp((char*) root->url.data, (char*) url.data, root->url.len);
  } else {
    res = strncmp((char*) root->url.data, (char*) url.data, url.len);
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

int
brook_gatekeeper_validate_session ( brook_connection_t* con ) {

  brook_gatekeeper_node_t* gatekeeper_role = con->_role;

  // ... validate role mask of session with route ...
  if ( (gatekeeper_role->role_mask & con->session.role_mask) == 0 ) {
    return BROOK_ERROR;
  }

  if ( strcmp((char*) gatekeeper_role->product_key.data, con->session.product_key) != 0 ) {
    return BROOK_ERROR;
  }

  return BROOK_OK;
}
