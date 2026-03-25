#include "core/beanstalkd.h"

bsc* bean_client = NULL;

int
brook_beanstalkd_connect () {
  char errstr[BSC_ERRSTR_LEN];

  // ... create client of beanstalkd ...
  bean_client = bsc_new("127.0.0.1", "11301", "default", brook_benstalkd_connection_error, 1024, 1024, 256, errstr);
  if ( !bean_client ) {
    //printf("Can't create beanstalkd client: %s\n", errstr);
    return BROOK_ERROR;
  }

  // ... connect client ...
  if ( !bsc_connect(bean_client, errstr) ) {
    //printf("Can't connect connect to beantslakd: %s\n", errstr);
    return BROOK_ERROR;
  }

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
        "payload": http body
      }
    }
  */
  cJSON* job = cJSON_CreateObject();
  if ( job == NULL ) {
    brook_connection_reply(con, 400, (brook_str_t) brook_str("Invalid JSON"), (brook_str_t) brook_str("The provided payload is not a valid JSON."));
    return BROOK_ERROR;
  }

  cJSON_AddStringToObject(job, "channel", _process_brook_id);

  // ... add payload of request to job ...
  if ( con->_parser->content_length > 0 && (con->_parser->method == POST || con->_parser->method == PATCH) ) {
    // ... transform http body in json object ...
    char* tmp = malloc(con->_parser->content_length);

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
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Invalid JSON"), (brook_str_t) brook_str("The provided payload is not a valid JSON."));
      return BROOK_ERROR;
    }

    cJSON_AddItemToObject(job, "payload", payload);
  }

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
  //printf("beanstalkd connection error or protocol: %d\n", error);
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
      brook_connection_reply(con, 500, (brook_str_t) brook_str("Job Submission Failed"), (brook_str_t) brook_str("The system was unable to persist the job after multiple retry attempts. Please check the queue status."));
    }
  }
}
