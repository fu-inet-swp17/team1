/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#ifndef COAP_CLIENT_H
#define COAP_CLIENT_H

#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SENML_LEN       75

kernel_pid_t coap_client_init(void);
void coap_server_init(void);

#ifdef __cplusplus
}
#endif

#endif /* COAP_CLIENT_H */
/** @} */
