
#pragma once
#ifndef _OEKAFKAWRAPPER_H
#define _OEKAFKAWRAPPER_H
//#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"
#include "rdkafka/rdkafka.h"
#include "serdes/serdes.h"
//#include "serdes/serdes-avro.h"
#include "avro/avro.h"

//kafka
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

//serdes
int wrapper_create_serdes_conf(char* registryurl);
int wrapper_create_serdes();
int wrapper_add_to_serdes_config(char* configname, char* configvalue);
int wrapper_register_value_schema(char* value_schema_name, char* value_schema_definition);
int wrapper_register_key_schema(char* key_schema_name, char* key_schema_definition);
int wrapper_register_offset_schema(char* offset_schema_name, char* offset_schema_definition);
int wrapper_create_avro_message();
int wrapper_add_value_to_message_string(char* field_name, char* field_value);
int wrapper_get_value_from_message_string(char* field_name);
int wrapper_destroy_avro_message();
void wrapper_destroy_serdes();
int wrapper_serialiase_and_send_message(char* topic, char* key);

//int wrapper_deserialiase_message_key(rd_kafka_message_t* rkm, const char** key_string);
int wrapper_deserialiase_message_key(rd_kafka_message_t* rkm, avro_value_t* avro);
int wrapper_deserialiase_message_value(rd_kafka_message_t* rkm, avro_value_t* avro);
void wrapper_destroy_avro_value(avro_value_t* avro);
void wrapper_get_avro_string(avro_value_t* avro_value, const char** string, size_t size);
int wrapper_get_value_field(avro_value_t* avro_source, avro_value_t* avro_field, char* field_name);

int wrapper_extract_key_and_value(rd_kafka_message_t* rkm);
void wrapper_get_key(char* out_string);
int wrapper_get_value_field_string(char* field_name, char* out_string);
void wrapper_clear_values();

#endif