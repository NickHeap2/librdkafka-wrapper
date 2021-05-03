#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "oekafka-wrapper.h"

#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

static volatile sig_atomic_t consume_messages = 1;

void sleeper(int sleepMs)
{
#ifdef LINUX
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef WINDOWS
    Sleep(sleepMs);
#endif
}

static void stop(int signal) {
    consume_messages = 0;
}

static void set_config_option(char *configname, char *configvalue)
{
    char *errstr;

    int result = wrapper_add_to_config(configname, configvalue);
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to set config with error: %s\n", errstr);
        exit(-1);
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING %s returned: %s\n", configname, errstr);
    }
}

void cleanup()
{
    fprintf(stderr, "Destroying serdes...\n");
    wrapper_destroy_serdes();

    fprintf(stderr, "Closing and destroying consumer...\n");
    wrapper_destroy_consumer();
}

int main()
{
    char *brokers = "host.docker.internal:9092";
    char *consumer_group = "rdkafka-consumer-group-1";
    char *topic = "test-topic-1";
    char *offset_reset = "latest";
    char *debug = "";


    int timeout = 10000;

    char *errstr;

    // trap ctrl-c and cleanly stop consumer
    signal(SIGINT, stop);

    // add to config*
    fprintf(stdout, "Setting config options...\n");
  
    if (strlen(debug) != 0)
    {
        set_config_option("debug", debug);
    }
    set_config_option("bootstrap.servers", brokers);
    set_config_option("group.id", consumer_group);
    //set_config_option("auto.offset.reset", offset_reset);

    // don't store offset
    set_config_option("enable.auto.commit", "false");

    // create the consumer
    fprintf(stdout, "Creating consumer...\n");

    int result = wrapper_create_consumer();
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to create consumer with error: %s\n", errstr);
        cleanup();
        return -4;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING create consumer returned: %s\n", errstr);
    }

    //// subscribe to a topic
    //fprintf(stdout, "Subscribing to topic %s...\n", topic);

    //result = wrapper_subscribe_to_topic(topic);
    //errstr = wrapper_get_last_error();
    //if (result != 0)
    //{
    //    fprintf(stderr, "Failed to subscribe to topic with error: %s\n", errstr);
    //    cleanup();
    //    return -5;
    //}
    //if (strlen(errstr) > 0)
    //{
    //    fprintf(stdout, "WARNING subscribe to topic returned: %s\n", errstr);
    //}

    // create serdes instance
    wrapper_create_serdes_conf("http://localhost:8081");
    if (result != 0)
    {
        errstr = wrapper_get_last_error();
        fprintf(stderr, "Failed to create serdes with result: %d\n", result);
        fprintf(stderr, "Failed to create serdes with error: %s\n", errstr);
        cleanup();
        return -6;
    }

    //wrapper_add_to_serdes_config("debug", "all");

    fprintf(stdout, "Creating serdes...\n");
    result = wrapper_create_serdes();
    if (result != 0)
    {
        errstr = wrapper_get_last_error();
        fprintf(stderr, "Failed to create serdes with result: %d\n", result);
        fprintf(stderr, "Failed to create serdes with error: %s\n", errstr);
        cleanup();
        return -7;
    }
    //don't need to manually register schema to consume
    /*
    // read value schema
    fprintf(stdout, "Reading value schema from file...\n");
    FILE* valueschemafile = fopen("messageSchema.json", "rb");
    char* valueschema = NULL;
    size_t messagelen;
    ssize_t bytes_read = getdelim(&valueschema, &messagelen, '\0', valueschemafile);
    fclose(valueschemafile);

    if (bytes_read == -1) {
        free(valueschema);
        cleanup();
        return -8;
    }

    // register value schema
    fprintf(stdout, "Registering value schema...\n");
    result = wrapper_register_value_schema("test-topic-1-value", valueschema);
    free(valueschema);
    if (result != 0)
    {
        errstr = wrapper_get_last_error();
        fprintf(stderr, "Failed to register value schema with result: %d\n", result);
        fprintf(stderr, "Failed to register value schema with error: %s\n", errstr);
        cleanup();
        return -9;
    }

    // read key schema
    fprintf(stdout, "Reading key schema from file...\n");
    FILE* keyschemafile = fopen("keySchema.json", "rb");
    char* keyschema = NULL;
    size_t keylen;
    bytes_read = getdelim(&keyschema, &keylen, '\0', keyschemafile);
    fclose(keyschemafile);
    if (bytes_read == -1) {
        cleanup();
        return -10;
    }

    // register key schema
    fprintf(stdout, "Registering key schema...\n");
    result = wrapper_register_key_schema("test-topic-1-key", keyschema);
    free(keyschema);
    if (result != 0)
    {
        errstr = wrapper_get_last_error();
        fprintf(stderr, "Failed to register key schema with result: %d\n", result);
        fprintf(stderr, "Failed to register key schema with error: %s\n", errstr);
        cleanup();
        return -11;
    }*/
  
    // move to end
    //result = wrapper_seek_offset(-2001 ,10000);
    //errstr = wrapper_get_last_error();
    //if (result != 0)
    //{
    //    fprintf(stderr, "Failed to seek offset with error: %s\n", errstr);
    //    cleanup();
    //    return -5;
    //}
    //if (strlen(errstr) > 0)
    //{
    //    fprintf(stdout, "WARNING seek offset returned: %s\n", errstr);
    //}

    while (consume_messages)
    {
        consume_messages = 0;
        sleeper(1000);

        rd_kafka_message_t *rkm = NULL;

        //fprintf(stdout, "Getting next message with %dms timeout...\n", timeout);
        //rkm = wrapper_get_message(timeout);

        fprintf(stdout, "Getting last message with %dms timeout...\n", timeout);
        rkm = wrapper_get_last_message(topic, timeout);
        if (!rkm)
        {
            fprintf(stdout, "No message.\n");
            errstr = wrapper_get_last_error();
            if (strlen(errstr) > 0)
            {
                fprintf(stderr, "Error getting message: %s\n", errstr);
            }

            continue;
        }
        else
        {
            fprintf(stdout, "Got message.\n");
            if (rkm->err)
            {
                fprintf(stderr, "Error message: %s\n", errstr);
                continue;
            }

            errstr = wrapper_get_last_error();
            if (strlen(errstr) > 0)
            {
                fprintf(stdout, "WARNING get message returned: %s\n", errstr);
                continue;
            }

            fprintf(stdout, "Got a valid message offset=%d key_len=%d len=%d!\n", rkm->offset, rkm->key_len, rkm->len);

            char key_string[100];
            char json_string[100];
            size_t size;

            result = wrapper_extract_key_and_value(rkm);
            if (result != SERDES_ERR_OK)
            {
                fprintf(stderr, "Failed to extract key and value return value: %d\n", result);
                errstr = wrapper_get_last_error();
                if (strlen(errstr) > 0)
                {
                    fprintf(stderr, "Failed to extract key with error: %s\n", errstr);
                }
            }
            else
            {
                //key_string = wrapper_get_key();
                wrapper_get_key(key_string);
                fprintf(stdout, "KEY IS [%s]\n", key_string);

                wrapper_get_value_field_string("Id", json_string);
                //wrapper_get_value_field_string("EventPayloadJson", json_string);
                fprintf(stdout, "VALUE IS [%s]\n", json_string);

            }

            wrapper_clear_values();

            /*avro_value_t avro_key;
            result = wrapper_deserialise_message_key(rkm, &avro_key); //, message_key);
            if (result != 0)
            {
                errstr = wrapper_get_last_error();
                if (strlen(errstr) > 0)
                {
                    fprintf(stderr, "Failed to deserialise message key with return value: %d\n", result);
                    fprintf(stderr, "Failed to deserialise message key with error: %s\n", errstr);
                }
            }
            else
            {
                wrapper_get_avro_string(&avro_key, &key_string, size);

                fprintf(stdout, "KEY IS [%s]\n", key_string);

                wrapper_destroy_avro_value(&avro_key);
            }

            //const char *new_key_string;
            //wrapper_get_key_from_message(rkm, &new_key_string);
            //fprintf(stdout, "NEW KEY IS [%s]\n", new_key_string);

            const char* value_string;
            avro_value_t avro_value;
            result = wrapper_deserialise_message_value(rkm, &avro_value); //, message_key);
            if (result != 0)
            {
                errstr = wrapper_get_last_error();
                if (strlen(errstr) > 0)
                {
                    fprintf(stderr, "Failed to deserialise message value with return value: %d\n", result);
                    fprintf(stderr, "Failed to deserialise message value with error: %s\n", errstr);
                }
            }
            else
            {
                avro_value_t avro_field;
                result = wrapper_get_value_field(&avro_value, &avro_field, "EventPayloadJson");
                if (result != 0)
                {
                    fprintf(stderr, "Failed to get value field with return code: %d\n", result);
                }
                else
                {
                    wrapper_get_avro_string(&avro_field, &value_string, size);
                    fprintf(stdout, "VALUE IS [%s]\n", value_string);
                }
                // this doesn't need to be freed
                //wrapper_destroy_avro_value(&avro_field);
            }
            wrapper_destroy_avro_value(&avro_value);*/

            fprintf(stdout, "Destroying message...\n");
            wrapper_destroy_message(rkm);
        }
    }

    cleanup();
}
