#include "core/beanstalkd.h"

bsc* bean_client = NULL;

int
brook_beanstalkd_connect () {
  char errstr[BSC_ERRSTR_LEN];

  // ... create client of beanstalkd ...
  bean_client = bsc_new("127.0.0.1", "11301", "default", brook_benstalkd_connection_error, 1024, 1024, 256, errstr);
  if ( !bean_client ) {
    printf("Can't create beanstalkd client: %s\n", errstr);
    return BROOK_ERROR;
  }

  // ... connect client ...
  if ( !bsc_connect(bean_client, errstr) ) {
    printf("Can't connect connect to beantslakd: %s\n", errstr);
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

  con->job.data.data = "OLA";
  con->job.data.len = 3;

  con->job.tube = con->_role->tube;

  // ... set tube to send job ...
  bsc_use(bean_client, brook_benstalkd_on_use, con, con->job.tube.data);

  // ... put job to beanstalkd
  bsc_put(bean_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, con->job.data.data, false);

  // ... beanstalkd add event of write ...
  _fds[POOL_INDEX_BEANSTALKD].events |= POLLOUT;

  return BROOK_OK;
}

int
brook_benstalkd_write () {
  bsc_write(bean_client);
  if (AQ_NODES_FREE(bean_client->outq) == bean_client->outq->size && bean_client->outq_offset == 0) {
    _fds[POOL_INDEX_BEANSTALKD].events &= ~POLLOUT;
  }
  return BROOK_OK;
}

void
brook_benstalkd_connection_error ( bsc *client, bsc_error_t error ) {
  printf("beanstalkd connection error or protocol: %d\n", error);
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
      bsc_put(bean_client, brook_benstalkd_on_put, con, con->job.priority, con->job.delay, con->job.ttr, con->job.data.len, con->job.data.data, false);
      ++con->job.retray;
    } else {
      // ... need call response ...
      brook_connection_reply(con, 500, (brook_str_t) brook_str("Job Submission Failed"), (brook_str_t) brook_str("The system was unable to persist the job after multiple retry attempts. Please check the queue status."));
    }
  }
}
