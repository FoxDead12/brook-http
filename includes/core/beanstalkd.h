#ifndef _BROOK_beanstalkd_H
#define  _BROOK_beanstalkd_H

#include "core/config.h"
#include "beanstalkclient.h"

int brook_beanstalkd_connect(brook_conf_t* config);
int brook_beanstalkd_retry_connect(brook_conf_t* config);
int brook_benstalkd_create_job(brook_connection_t* con);
int brook_benstalkd_write();
int brook_benstalkd_job_payload(brook_connection_t* con);
void brook_benstalkd_connection_error(bsc *client, bsc_error_t error);
void brook_benstalkd_on_use(bsc *client, struct bsc_use_info *info);
void brook_benstalkd_on_put(bsc *client, struct bsc_put_info *info);

enum job_state {
  s_job_create,
  s_job_used,
  s_job_put,
  s_job_receive
};

#endif
