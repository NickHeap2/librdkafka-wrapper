#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "oekafka-wrapper.h"

static volatile sig_atomic_t produce_and_consume_messages = 1;

static void stop(int signal) {
    produce_and_consume_messages = 0;
}

static int set_config_option(char *configname, char *configvalue)
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

int main()
{
    char *brokers = "host.docker.internal:9092";
    char *consumer_group = "rdkafka-consumer-group-1";
    char *topic = "test-topic-1";
    char *offset_reset = "earliest";
    char *debug = "";

    int timeout = 1000;

    char *errstr;

    /* trap ctrl-c and cleanly stop consumer */
    signal(SIGINT, stop);

    /* add to config*/
    fprintf(stdout, "Setting consumer config options...\n");

    if (strlen(debug) != 0)
    {
        set_config_option("debug", debug);
    }
    set_config_option("bootstrap.servers", brokers);
    set_config_option("group.id", consumer_group);
    set_config_option("auto.offset.reset", offset_reset);

    /* create the consumer */
    fprintf(stdout, "Creating consumer...\n");

    int result = wrapper_create_consumer();
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to create consumer with error: %s\n", errstr);
        return -4;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING create consumer returned: %s\n", errstr);
    }

    /* add to config*/
    fprintf(stdout, "Setting producer config options...\n");

    if (strlen(debug) != 0)
    {
        set_config_option("debug", debug);
    }
    set_config_option("bootstrap.servers", brokers);
    set_config_option("group.id", consumer_group);
    set_config_option("auto.offset.reset", offset_reset);

    /* create the producer */
    fprintf(stdout, "Creating producer...\n");

    result = wrapper_create_producer();
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to create producer with error: %s\n", errstr);
        return -4;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING create producer returned: %s\n", errstr);
    }

    /* subscribe to a topic */
    fprintf(stdout, "Subscribing to topic %s...\n", topic);

    result = wrapper_subscribe_to_topic(topic);
    errstr = wrapper_get_last_error();
    if (result != 0)
    {
        fprintf(stderr, "Failed to subscribe to topic with error: %s\n", errstr);
        return -5;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING subscribe to topic returned: %s\n", errstr);
    }

    while (produce_and_consume_messages)
    {
        char *key = "KEY0";
        char *payload = "PAYLOAD0";

        sleep(1);

        fprintf(stdout, "Producing message...\n");
        int result = wrapper_produce_message(topic, key, strlen(key), payload, strlen(payload));
        errstr = wrapper_get_last_error();
        if (strlen(errstr) > 0)
        {
            fprintf(stderr, "ERROR producing message returned: %s\n", errstr);
        }
        else
        {
            fprintf(stdout, "    Message produced.\n");
        }


        rd_kafka_message_t *rkm;

        fprintf(stdout, "Getting next message with %dms timeout...\n", timeout);
        rkm = wrapper_get_message(timeout);
        errstr = wrapper_get_last_error();
        if (!rkm)
        {
            fprintf(stdout, "No message.\n");
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
            else
            {
                if (strlen(errstr) > 0)
                {
                    fprintf(stdout, "WARNING get message returned: %s\n", errstr);
                    continue;
                }

                fprintf(stdout, "Got a valid message!\n");
            }

            wrapper_destroy_message(rkm);
        }
    }

    fprintf(stderr, "Closing and destroying consumer...\n");
    wrapper_destroy_consumer();

    fprintf(stderr, "Closing and destroying producer...\n");
    wrapper_destroy_producer();
}