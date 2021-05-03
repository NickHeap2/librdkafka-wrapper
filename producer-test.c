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

static volatile sig_atomic_t produce_messages = 1;

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
    produce_messages = 0;
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
    fprintf(stderr, "Closing and destroying serdes...\n");
    wrapper_destroy_serdes();

    fprintf(stderr, "Closing and destroying producer...\n");
    wrapper_destroy_producer();
}

int main()
{
    char *brokers = "host.docker.internal:9092";
    char *consumer_group = "rdkafka-consumer-group-1";
    char *topic = "test-topic-1";
    char *offset_reset = "earliest";
    char *debug = ""; //broker,topic,msg

    int timeout = 1000;

    char *errstr;

    /* trap ctrl-c and cleanly stop consumer */
    signal(SIGINT, stop);

    /* add to config*/
    fprintf(stdout, "Setting config options...\n");

    if (strlen(debug) != 0)
    {
        set_config_option("debug", debug);
    }
    set_config_option("bootstrap.servers", brokers);
    set_config_option("linger.ms", "5");

    /* create the producer */
    fprintf(stdout, "Creating producer...\n");

    int result = wrapper_create_producer();
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to create producer with error: %s\n", errstr);
        cleanup();
        return -4;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING create producer returned: %s\n", errstr);
    }
    
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

    wrapper_add_to_serdes_config("debug", "all");

    fprintf(stdout, "Creating serdes...\n");
    result = wrapper_create_serdes();
    if (result != 0)
    {
        errstr = wrapper_get_last_error();
        fprintf(stderr, "Failed to create serdes with result: %d\n", result);
        fprintf(stderr, "Failed to create serdes with error: %s\n", errstr);
        cleanup();
        return -5;
    }

    // read value schema
    fprintf(stdout, "Reading value schema from file...\n");
    FILE *valueschemafile = fopen("messageSchema.json", "rb");
    char *valueschema = NULL;
    size_t messagelen;
    ssize_t bytes_read = getdelim(&valueschema, &messagelen, '\0', valueschemafile);
    fclose(valueschemafile);
    if (bytes_read == -1) {
        free(valueschema);
        cleanup();
        return -6;
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
        return -7;
    }
   
    // read key schema
    fprintf(stdout, "Reading key schema from file...\n");
    FILE* keyschemafile = fopen("keySchema.json", "rb");
    char* keyschema = NULL;
    size_t keylen;
    bytes_read = getdelim(&keyschema, &keylen, '\0', keyschemafile);
    fclose(keyschemafile);
    if (bytes_read == -1) {
        free(keyschema);
        cleanup();
        return -8;
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
        return -9;
    }

    while (produce_messages)
    {
        // get delivery reports
        //rd_kafka_poll(rk, 0/*non-blocking */);
        sleeper(1000);

        fprintf(stdout, "Creating message...\n");
        result = wrapper_create_avro_message();
        if (result != 0)
        {
            errstr = wrapper_get_last_error();
            fprintf(stderr, "Failed to create avro mesasge with result: %d\n", result);
            fprintf(stderr, "Failed to create avro mesasge with error: %s\n", errstr);
            cleanup();
            return -10;
        }

        fprintf(stdout, "Setting field value...\n");
        result = wrapper_add_value_to_message_string("EventPayloadJson", "{ 'json_value': 'the_value' }");
        if (result != 0)
        {
            errstr = wrapper_get_last_error();
            fprintf(stderr, "Failed to set field value with result: %d\n", result);
            fprintf(stderr, "Failed to set field value with error: %s\n", errstr);
            cleanup();
            return -11;
        }

        fprintf(stdout, "Getting field value...\n");
        result = wrapper_get_value_from_message_string("EventPayloadJson");
        if (result != 0)
        {
            errstr = wrapper_get_last_error();
            fprintf(stderr, "Failed to get field value with result: %d\n", result);
            fprintf(stderr, "Failed to get field value with error: %s\n", errstr);
            cleanup();
            return -12;
        }

        fprintf(stdout, "Serialising message...\n");
        result = wrapper_serialise_and_send_message(topic, "KEY1");
        if (result != 0)
        {
            errstr = wrapper_get_last_error();
            fprintf(stderr, "Failed to serialise message with result: %d\n", result);
            fprintf(stderr, "Failed to serialise message with error: %s\n", errstr);
            cleanup();
            return -13;
        }

        fprintf(stdout, "Destroying message...\n");
        wrapper_destroy_avro_message();

        //    char* key = "KEY0";
        //    char* payload = "PAYLOAD0";
        //    ////fprintf(stdout, "Producing message...\n");
        //    //int result = wrapper_produce_message(topic, key, strlen(key), payload, strlen(payload));
        //    //errstr = wrapper_get_last_error();
        //    //if (strlen(errstr) > 0)
        //    //{
        //    //    //fprintf(stderr, "ERROR producing message returned: %s\n", errstr);
        //    //}
    }
    cleanup();
}
