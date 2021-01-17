
#pragma once
#ifndef _OEKAFKAWRAPPER_H
#define _OEKAFKAWRAPPER_H
//#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"
#include <rdkafka.h>

char* wrapper_get_last_error();
int wrapper_add_to_config(char *configname, char *configvalue);
int wrapper_create_consumer();
int wrapper_create_producer();
int wrapper_subscribe_to_topic(char *topic);
rd_kafka_message_t* wrapper_get_message(int timeout);
int wrapper_produce_message(char *topic, char *key, size_t key_len, char *payload, size_t len);
int wrapper_produce_this_message(char *topic, rd_kafka_message_t *message);
void wrapper_destroy_message(rd_kafka_message_t* rkm);
void wrapper_destroy_consumer();
void wrapper_destroy_producer();

#endif