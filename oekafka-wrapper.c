// librdkafka-wrapper.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"

static rd_kafka_conf_t *conf;
static rd_kafka_t *rkc;
static rd_kafka_t *rkp;

static char errstr[512] = "";

static void clear_last_error()
{
    errstr[0] = 0;
}

void wrapper_destroy_message(rd_kafka_message_t *rkm)
{
    clear_last_error();

    rd_kafka_message_destroy(rkm);
}

void wrapper_destroy_consumer()
{
    clear_last_error();

    rd_kafka_consumer_close(rkc);

    rd_kafka_destroy(rkc);
}

void wrapper_destroy_producer()
{
    clear_last_error();

    rd_kafka_flush(rkp, 2000);
    //if rd_kafka_outq_len(rkp) > 0)

    rd_kafka_destroy(rkp);
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

    rkc = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!rkc)
    {
        return 1;
    }

    conf = NULL;

    rd_kafka_poll_set_consumer(rkc);

    return 0;
}

int wrapper_create_producer()
{
    clear_last_error();

    rkp = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rkp)
    {
        return 1;
    }

    conf = NULL;

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
    err = rd_kafka_subscribe(rkc, subscription);
    if (err) {
        strcpy(errstr, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        wrapper_destroy_consumer();
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    return 0;
}

rd_kafka_message_t *wrapper_get_message(int timeout)
{
    rd_kafka_message_t *rkm;

    clear_last_error();

    rkm = rd_kafka_consumer_poll(rkc, timeout);
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

int wrapper_produce_this_message(char *topic, rd_kafka_message_t *message)
{
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(rkp,
        RD_KAFKA_V_TOPIC(topic),
        RD_KAFKA_V_KEY(message->key, message->key_len),
        RD_KAFKA_V_VALUE(message->payload, message->len),
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        RD_KAFKA_V_END);
    if (err) {
        strcpy(errstr, rd_kafka_err2str(err));
    }
    return err;
}

int wrapper_produce_message(char *topic, char *key, size_t key_len, char *payload, size_t len)
{
    rd_kafka_resp_err_t err;

    err = rd_kafka_producev(rkp,
        RD_KAFKA_V_TOPIC(topic),
        RD_KAFKA_V_KEY(key, key_len),
        RD_KAFKA_V_VALUE(payload, len),
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        RD_KAFKA_V_END);
    if (err) {
        strcpy(errstr, rd_kafka_err2str(err));
    }
    return err;
}

