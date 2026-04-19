#include "core/beanstalkd.h"

bsc* bean_client = NULL;

int
brook_beanstalkd_connect ( brook_conf_t* config ) {
  char errstr[BSC_ERRSTR_LEN];

  char port[10];
  snprintf(port, sizeof(port), "%d", config->beanstalkd.port);

  // ... create client of beanstalkd ...
  bean_client = bsc_new(config->beanstalkd.host, port, "default", brook_benstalkd_connection_error, 1024, 1024, 256, errstr);
  if ( !bean_client ) {
    kill(getppid(), SIGTERM);
    brook_log(config, LOG_ERR, "Can't create beanstalkd client: %s\n", errstr);
    return BROOK_ERROR;
  }

  // ... set data to store in beanstalkd client ...
  bean_client->data = config;

  // ... connect client ...
  if ( !bsc_connect(bean_client, errstr) ) {
    kill(getppid(), SIGTERM);
    brook_log(config, LOG_ERR, "Can't connect connect to beantslakd: %s\n", errstr);
    return BROOK_ERROR;
  }

  brook_log(config, LOG_INFO, "Process connected to beanstalkd ...\n");

  return BROOK_OK;
}

int
brook_benstalkd_create_job ( brook_connection_t* con ) {
  con->job.state = s_job_create;
  con->job.id = 0;

  con->job.priority = 1;
  con->job.delay = 0;
  con->job.ttr = 1000;

  con->job.max_retray = 3;
  con->job.retray = 0;

  brook_benstalkd_job_payload(con);

  con->job.tube = con->_role->tube;

  // ... set tube to send job ...
  bsc_use(bean_client, brook_benstalkd_on_use, con, (const char*) con->job.tube.data);

  // ... put job to beanstalkd
  bsc_put(bean_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, (const char*) con->job.data.data, false);

  // ... beanstalkd add event of write ...
  _fds[POOL_INDEX_BEANSTALKD].events |= POLLOUT;

  return BROOK_OK;
}

int
brook_benstalkd_job_payload ( brook_connection_t* con ) {

  /*
    {
      "job": {
        "channel": process id
        "session": {
          user_id
          role_mask
          schema (optional)
        }
        "payload": http body,
        "params": http params
      }
    }
  */
  cJSON* job = cJSON_CreateObject();
  if ( job == NULL ) {
    brook_log(con->_config, LOG_ERR, "Unable to allocate memory for cJSON object at %s:%d", __FILE__, __LINE__);
    brook_connection_reply(con, 500, (brook_str_t) brook_str("Internal Server Error"), (brook_str_t) brook_str("An unexpected error occurred while processing the request resources."));
    return BROOK_ERROR;
  }

  cJSON_AddStringToObject(job, "channel", brook_process_id);

  // ... add session data to job ...
  if (con->session.user_id != 0 ) {

    cJSON *session = cJSON_CreateObject();

    cJSON_AddNumberToObject(session, "role_mask",   con->session.role_mask);
    cJSON_AddStringToObject(session, "product_key", con->session.product_key);
    cJSON_AddNumberToObject(session, "user_id",     con->session.user_id);

    if (con->session.schema[0] != '\0') {
      cJSON_AddStringToObject(session, "schema", con->session.schema);
    }

    cJSON_AddItemToObject(job, "session", session);
  }

  // ... add payload of request to job ...
  if ( con->_parser->content_length > 0 && (con->_parser->method == POST || con->_parser->method == PATCH) ) {
    // ... transform http body in json object ...
    char* tmp = malloc(con->_parser->content_length + 1);
    if (tmp == NULL) return BROOK_ERROR;

    tmp[con->_parser->content_length] = '\0';

    brook_buffer_chain_t* buffer = con->_data;
    int range_start = con->_parser->nheader;
    int bytes = 0;

    while ( buffer != NULL ) {
      char* rec = tmp + bytes;
      unsigned char* send = buffer->data + range_start;
      uint64_t b = buffer->len - range_start;
      memcpy(rec, send, b);
      bytes += b;
      range_start = 0;
      buffer = buffer->next;
    }

    cJSON* payload = cJSON_Parse(tmp);
    free(tmp);
    if ( payload == NULL ) {
      cJSON_Delete(job);
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Bad Request"), (brook_str_t) brook_str("The request payload could not be parsed as valid JSON."));
      return BROOK_ERROR;
    }

    cJSON_AddItemToObject(job, "payload", payload);
  }

  // ... add params ...
  if ( con->_parser->params != NULL ) {
    cJSON *params = cJSON_CreateObject();

    for (size_t i = 0; i < con->_parser->params_n; i++) {
      brook_str_t key = con->_parser->params[i].key;
      brook_str_t value = con->_parser->params[i].value;

      // ... transform key string ...
      char* key_string = malloc(key.len + 1);
      memcpy(key_string, key.data, key.len);
      key_string[key.len] = '\0';

      // ... transform key string ...
      char* value_string = malloc(value.len);
      memcpy(value_string, value.data, value.len);
      value_string[value.len] = '\0';

      cJSON_AddStringToObject(params, key_string, value_string);
      free(key_string);
      free(value_string);
    }

    cJSON_AddItemToObject(job, "params", params);
  }

  // ... sanity check to clean data ...
  if (con->job.data.data != NULL) {
    free(con->job.data.data);
    con->job.data.data = NULL;
  }

  // ... generate json string to send ...
  con->job.data.data = (unsigned char*) cJSON_PrintUnformatted(job);
  con->job.data.len = strlen(con->job.data.data);

  cJSON_Delete(job);
  return BROOK_OK;
}

int
brook_benstalkd_write () {
  bsc_write(bean_client);
  if (AQ_NODES_FREE(bean_client->outq) == bean_client->outq->size) {
    bean_client->outq_offset = 0;
    _fds[POOL_INDEX_BEANSTALKD].events &= ~POLLOUT;
  }
  return BROOK_OK;
}

void
brook_benstalkd_connection_error ( bsc *client, bsc_error_t error ) {

  brook_conf_t *config = (brook_conf_t *)client->data;

  const char *error_msg;
  switch (error) {
    case BSC_ERROR_NONE:
      error_msg = "No error";
      break;
    case BSC_ERROR_INTERNAL:
      error_msg = "Internal library error";
      break;
    case BSC_ERROR_SOCKET:
      error_msg = "Socket communication failure (Network)";
      break;
    case BSC_ERROR_MEMORY:
      error_msg = "Memory allocation failed";
      break;
    case BSC_ERROR_QUEUE_FULL:
      error_msg = "Internal command queue is full";
      break;
    default:
      error_msg = "Unknown Beanstalkd error";
      break;
  }

  brook_log(config, LOG_ERR, "Beanstalkd connection lost unexpectedly. Reason: %s\n", error_msg);

  sleep(5);
  exit(BROOK_ERROR);
}

void
brook_benstalkd_on_use ( bsc *client, struct bsc_use_info *info ) {
  brook_connection_t* con = (brook_connection_t*) info->user_data;
  con->job.state = s_job_used;
}

void
brook_benstalkd_on_put ( bsc *client, struct bsc_put_info *info ) {
  brook_connection_t* con = (brook_connection_t*) info->user_data;
  if (info->response.code == BSC_PUT_RES_INSERTED) {
    con->job.state = s_job_put;
    con->job.id = info->response.id;
  } else {
    if ( con->job.retray < con->job.max_retray ) {
      bsc_use(bean_client, brook_benstalkd_on_use, con, con->job.tube.data);
      bsc_put(bean_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, con->job.data.data, false);
      _fds[POOL_INDEX_BEANSTALKD].events |= POLLOUT;
      ++con->job.retray;
    } else {
      // ... need call response ...
      brook_log(con->_config, LOG_ERR, "Job queue failed, maximum retry attempts (%d) exceeded. Last Beanstalkd response code: %d. file: %s:%d", con->job.max_retray, info->response.code, __FILE__, __LINE__);
      brook_connection_reply(con, 500, (brook_str_t) brook_str("Internal Server Error"), (brook_str_t) brook_str("The server was unable to enqueue the job. Please resubmit your request."));
    }
  }
}
