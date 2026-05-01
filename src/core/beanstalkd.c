#include "core/beanstalkd.h"

bsc* bean_client = NULL;

int
brook_beanstalkd_connect ( brook_conf_t* config ) {

  // ... buffer to store errors of client ...
  char errstr[BSC_ERRSTR_LEN];

  // ... transform port to string ...
  char port[10];
  snprintf(port, sizeof(port), "%d", config->beanstalkd.port);

  // ... create client of beanstalkd ...
  bean_client = bsc_new(config->beanstalkd.host, port, "default", brook_benstalkd_connection_error, 1024, 1024, 256, errstr);

  if ( !bean_client ) {
    brook_log(config, LOG_ERR, "Can't create beanstalkd client: %s\n", errstr);
    keep_running = 0;
    return BROOK_ERROR;
  }

  // ... set data to store in beanstalkd client ...
  bean_client->data = config;

  // ... connect client ...
  if ( !bsc_connect(bean_client, errstr) ) {
    brook_log(config, LOG_ERR, "Can't connect to beantslakd: %s\n", errstr);
    keep_running = 0;
    return BROOK_ERROR;
  }

  brook_log(config, LOG_INFO, "Process connected to beanstalkd ...\n");
  return BROOK_OK;
}

/**
 * Callback when beanstalkd client disconnect
 */
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

  // ... make worker stop ...
  keep_running = 0;
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

int
brook_benstalkd_create_job ( brook_connection_t* con ) {

  // ... init default props of job ...
  con->job.tube = con->_role->tube;
  con->job.state = s_job_create;
  con->job.priority = 1024;
  con->job.max_retray = 3;
  con->job.ttr = 1000;
  con->job.retray = 0;
  con->job.delay = 0;
  con->job.id = 0;

  // ... create job payload ...
  if ( brook_benstalkd_job_payload(con) == BROOK_ERROR ) {
    return BROOK_ERROR;
  }

  // ... set tube to send job ...
  bsc_use(bean_client, brook_beanstalkd_on_tube_use, con, (const char*) con->job.tube.data);

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
    brook_log(con->_config, LOG_ERR, "JSON Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
    brook_destroy_connection(con);
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

    if ( !tmp ) {
      brook_log(con->_config, LOG_ERR, "Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
      brook_destroy_connection(con);
      return BROOK_ERROR;
    }
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

    if ( !payload ) {
      cJSON_Delete(job);
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Bad Request"), (brook_str_t) brook_str("The request payload could not be parsed as valid JSON."));
      return BROOK_ERROR;
    }

    cJSON_AddItemToObject(job, "payload", payload);
  }

  // ... add params ...
  if ( con->_parser->params != NULL ) {

    cJSON *params = cJSON_CreateObject();

    if ( !params ) {
      cJSON_Delete(job);
      brook_log(con->_config, LOG_ERR, "JSON Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
      brook_destroy_connection(con);
      return BROOK_ERROR;
    }

    // ... buffer to copy data from buffers to json object ...
    static char param_key[256]    = {0};
    static char param_value[4096] = {0};

    for ( int i = 0; i < con->_parser->params_n; i++ ) {

      // ... element of array ...
      brook_str_t key   = con->_parser->params[i].key;
      brook_str_t value = con->_parser->params[i].value;

      // ... create key object ...
      size_t key_len = (key.len < sizeof(param_key) - 1) ? key.len : sizeof(param_key) - 1;
      memcpy(param_key, key.data, key_len);
      param_key[key_len] = '\0';

      // ... create value object ...
      size_t value_len = (value.len < sizeof(param_value) - 1) ? value.len : sizeof(param_value) - 1;
      memcpy(param_value, value.data, value_len);
      param_value[value_len] = '\0';

      cJSON_AddStringToObject(params, param_key, param_value);
    }

    cJSON_AddItemToObject(job, "params", params);
  }

  // ... generate json string to send ...
  con->job.data.data = (unsigned char*) cJSON_PrintUnformatted(job);
  cJSON_Delete(job);

  if ( !con->job.data.data ) {
    brook_log(con->_config, LOG_ERR, "JSON Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
    brook_destroy_connection(con);
    return BROOK_ERROR;
  }

  con->job.data.len = strlen((char*) con->job.data.data);

  return BROOK_OK;
}


void
brook_beanstalkd_on_tube_use ( bsc* _, struct bsc_use_info* info ) {

  // ... parse connection from beanstalkd callback ...
  brook_connection_t* con = (brook_connection_t*) info->user_data;

  if ( con->job.state != s_job_create ) {
    brook_log(con->_config, LOG_ERR, "Invalid Job State at %s:%d: Expected s_job_create (%d), but got %d\n", __FILE__, __LINE__, s_job_create, con->job.state);
    brook_destroy_connection(con);
    return;
  }

  con->job.state = s_job_used;
}

/**
 * Callback response to PUT message to beanstalkd
 */
void
brook_benstalkd_on_put ( bsc* _client, struct bsc_put_info* info ) {

  // ... parse connection from beanstalkd callback ...
  brook_connection_t* con = (brook_connection_t*) info->user_data;

  switch (info->response.code) {
    case BSC_PUT_RES_INSERTED: {
      con->job.state = s_job_put;
      con->job.id = info->response.id;
      break;
    }

    default: {
      brook_log(con->_config, LOG_ERR, "Beanstalkd PUT failed at %s:%d: Response Code %d, Job State %d, Retry %d/%d\n", __FILE__, __LINE__, info->response.code, con->job.state, con->job.retray, con->job.max_retray);

      // ... error trying submit job to beanstalkd ...
      if ( con->job.retray < con->job.max_retray ) {
        bsc_use(_client, brook_beanstalkd_on_tube_use, con, (char*) con->job.tube.data);

        bsc_put(_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, (char*) con->job.data.data, false);

        _fds[POOL_INDEX_BEANSTALKD].events |= POLLOUT;

        ++con->job.retray;
      } else {
        brook_log(con->_config, LOG_ERR, "Job queue failed, maximum retry attempts (%d) exceeded. Last Beanstalkd response code: %d. file: %s:%d", con->job.max_retray, info->response.code, __FILE__, __LINE__);
        brook_connection_reply(con, 500, (brook_str_t) brook_str("Internal Server Error"), (brook_str_t) brook_str("The server was unable to enqueue the job. Please resubmit your request."));
      }

      break;
    }
  }

  return;
}
