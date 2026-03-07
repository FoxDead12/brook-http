#ifndef _BROOK_BEANSTALKD_H
#define  _BROOK_BEANSTALKD_H

#include "core/config.h"
#include "beanstalkclient.h"


int brook_beanstalkd_connect();
void brook_beanstalkd_on_error(struct _bsc *b, bsc_error_t error_code);

#endif
