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
        return -4;
    }
    if (strlen(errstr) > 0)
    {
        fprintf(stdout, "WARNING create producer returned: %s\n", errstr);
    }

    while (produce_messages)
    {
        // get delivery reports
        //rd_kafka_poll(rk, 0/*non-blocking */);

        char* key = "KEY0";
        char* payload = "PAYLOAD0";

        fprintf(stdout, "Producing message...\n");
        int result = wrapper_produce_message(topic, key, strlen(key), payload, strlen(payload));
        errstr = wrapper_get_last_error();
        if (strlen(errstr) > 0)
        {
            fprintf(stderr, "ERROR producing message returned: %s\n", errstr);
        }

        sleeper(1);
    }

    fprintf(stderr, "Closing and destroying producer...\n");
    wrapper_destroy_producer();
}