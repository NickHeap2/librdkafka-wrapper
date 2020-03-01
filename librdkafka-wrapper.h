
#pragma once
#ifndef _LIBRDKAFKAWRAPPER_H
#define _LIBRDKAFKAWRAPPER_H
#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"

char* wrapper_get_last_error();
int wrapper_add_to_config(char *configname, char *configvalue);
int wrapper_create_consumer();
int wrapper_subscribe_to_topic(char *topic);
rd_kafka_message_t* wrapper_get_message(int timeout);
void wrapper_destroy_message(rd_kafka_message_t* rkm);
void wrapper_destroy_consumer();

#endif