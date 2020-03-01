// librdkafka-wrapper.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"

static rd_kafka_conf_t* conf;
static rd_kafka_t* rk;
static char errstr[512] = "";

static void clear_last_error()
{
    errstr[0] = 0;
}

char* wrapper_get_last_error()
{
    return errstr;
}

int wrapper_add_to_config(char *configname, char *configvalue)
{
    clear_last_error();
    if (!conf)
    {
        conf = rd_kafka_conf_new();
    }

    int result;
    result = rd_kafka_conf_set(conf, configname, configvalue, errstr, sizeof(errstr));
    if (result != RD_KAFKA_CONF_OK) {
        rd_kafka_conf_destroy(conf);
    }

    return result;
}


int wrapper_create_consumer()
{
    clear_last_error();

    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!rk)
    {
        return 1;
    }

    conf = NULL;

    rd_kafka_poll_set_consumer(rk);

    return 0;
}

int wrapper_subscribe_to_topic(char *topic)
{
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t* subscription;

    clear_last_error();

    subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topic, RD_KAFKA_PARTITION_UA);

    /* Subscribe to the list of topics */
    err = rd_kafka_subscribe(rk, subscription);
    if (err) {
        strcpy(errstr, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(rk);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    return 0;
}

rd_kafka_message_t* wrapper_get_message(int timeout)
{
    rd_kafka_message_t* rkm;

    clear_last_error();

    rkm = rd_kafka_consumer_poll(rk, timeout);
    if (!rkm)
    {
        clear_last_error();
        return NULL;
    }

    if (rkm->err) {
        return rkm;
    }

    return rkm;
}


void wrapper_destroy_message(rd_kafka_message_t* rkm)
{
    clear_last_error();

    rd_kafka_message_destroy(rkm);
}

void wrapper_destroy_consumer()
{
    clear_last_error();

    rd_kafka_consumer_close(rk);

    rd_kafka_destroy(rk);
}
