// librdkafka-wrapper.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
//#include "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/rdkafka.h"
#include "rdkafka/rdkafka.h"
#include "serdes/serdes.h"
#include "serdes/serdes-avro.h"
#include "avro/avro.h"

// kafka
static rd_kafka_conf_t *conf = NULL;
static rd_kafka_t *rkc;
static rd_kafka_t *rkp;

//serdes
static serdes_conf_t *sconf;
static serdes_t *serdes;
static serdes_schema_t *value_schema;
static serdes_schema_t *key_schema;
static serdes_schema_t *offset_schema;

static avro_value_iface_t *value_class;
static avro_value_t  message_instance;

static avro_value_t key_avro;
static avro_value_t value_avro;

//shared
static char errstr[512] = "";

static char* last_message_key;
static char* last_message_value;

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

    //don't think we need this also: rd_kafka_consume_stop(rkt, partition);
    rd_kafka_consumer_close(rkc);

    rd_kafka_destroy(rkc);
}

void wrapper_destroy_producer()
{
    clear_last_error();

    rd_kafka_flush(rkp, 10000);
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

    int events = rd_kafka_poll(rkp, 0);
    //fprintf(stdout, "  Polled %d events\n", events);
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

    int events = rd_kafka_poll(rkp, 0);
    //fprintf(stdout, "  Polled %d events\n", events);
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

// serdes
int wrapper_create_serdes_conf(char* registryurl)
{
    clear_last_error();

    sconf = serdes_conf_new(NULL, 0,
        "schema.registry.url", registryurl,
        NULL);
    if (!sconf)
    {
        strcpy(errstr, "Failed to create serdes_conf");
        return 1;
    }

    return 0;
}

int wrapper_create_serdes(char* registryurl)
{
    clear_last_error();

    serdes = serdes_new(sconf, errstr, sizeof(errstr));
    if (!serdes) {
        return 1;
    }

    return 0;
}

void wrapper_destroy_serdes()
{
    clear_last_error();

    if (key_schema)
    {
        fprintf(stdout, "destroying key_schema...\n");
        serdes_schema_destroy(key_schema);
    }
    if (value_schema)
    {
        fprintf(stdout, "destroying value_schema...\n");
        serdes_schema_destroy(value_schema);
    }
    if (offset_schema)
    {
        fprintf(stdout, "destroying offset_schema...\n");
        serdes_schema_destroy(offset_schema);
    }

    if (serdes)
    {
        fprintf(stdout, "destroying serdes...\n");
        serdes_destroy(serdes);
    }

    if (sconf)
    {
        fprintf(stdout, "destroying sconf...\n");
        // as we passed the sconf in to serdes_new we don't need to destroy it ourself
        //serdes_conf_destroy(sconf);
    }
}

int wrapper_add_to_serdes_config(char* configname, char* configvalue)
{
    clear_last_error();
    if (!sconf)
    {
        strcpy(errstr, "No sconfig!");
        return 1;
    }

    int result;
    result = serdes_conf_set(sconf, configname, configvalue, errstr, sizeof(errstr));
    if (result != SERDES_ERR_OK) {
        fprintf(stdout, "serdes_conf_set failed!!!!...\n");
        serdes_conf_destroy(sconf);
    }

    return result;
}


int wrapper_register_value_schema(char* value_schema_name, char* value_schema_definition)
{
    if (value_schema)
    {
        serdes_schema_destroy(value_schema);
    }

    clear_last_error();

    //avro_schema_from_json_literal()
    value_schema = serdes_schema_add(serdes,
                                     value_schema_name, -1,
                                     value_schema_definition, -1,
                                     errstr, sizeof(errstr));
    if (!value_schema)
    {
        return 1;
    }

    fprintf(stderr, "%% Added schema %s with id %d\n",
        serdes_schema_name(value_schema),
        serdes_schema_id(value_schema));
    return 0;
}


int wrapper_register_key_schema(char* key_schema_name, char* key_schema_definition)
{
    if (key_schema)
    {
        serdes_schema_destroy(key_schema);
    }

    clear_last_error();

    //avro_schema_from_json_literal()
    key_schema = serdes_schema_add(serdes,
        key_schema_name, -1,
        key_schema_definition, -1,
        errstr, sizeof(errstr));
    if (!key_schema)
    {
        return 1;
    }

    fprintf(stderr, "%% Added schema %s with id %d\n",
        serdes_schema_name(key_schema),
        serdes_schema_id(key_schema));
    return 0;
}

int wrapper_register_offset_schema(char* offset_schema_name, char* offset_schema_definition)
{
    if (offset_schema)
    {
        serdes_schema_destroy(offset_schema);
    }

    clear_last_error();

    offset_schema = serdes_schema_add(serdes,
                                     offset_schema_name, -1,
                                     offset_schema_definition, -1,
                                     errstr, sizeof(errstr));
    if (!offset_schema)
    {
        return 1;
    }

    fprintf(stderr, "%% Added schema %s with id %d\n",
        serdes_schema_name(offset_schema),
        serdes_schema_id(offset_schema));
    return 0;
}

int wrapper_create_avro_message()
{
    value_class = avro_generic_class_from_schema(serdes_schema_avro(value_schema));
    if (!value_class)
    {
        strcpy(errstr, "Failed to get value class!");
        return 1;
    }
    
    avro_generic_value_new(value_class, &message_instance);
    if (!&message_instance)
    {
        strcpy(errstr, "Failed to get message instance!");
        return 2;
    }

    return 0;
}

int wrapper_add_value_to_message_string(char* field_name, char* field_value)
{
    avro_value_t field_value_instance;

    if (avro_value_get_by_name(&message_instance, field_name, &field_value_instance, NULL) == 0)
    {
        avro_value_set_string(&field_value_instance, field_value);
    }
    else
    {
        strcpy(errstr, "Failed to get value by name!");
        return 1;
    }

    // clean up field value
    //avro_value_decref(&field_value_instance);

    return 0;
}

int wrapper_get_value_from_message_string(char* field_name)
{
    avro_value_t field_value_instance;

    const char *p;
    size_t size;

    if (avro_value_get_by_name(&message_instance, field_name, &field_value_instance, NULL) == 0)
    {
        avro_value_get_string(&field_value_instance, &p, &size);
        fprintf(stdout, "Field value is: %s\n", p);
    }
    else
    {
        strcpy(errstr, "Failed to get value by name!");
        return 1;
    }

    // clean up field value
    //avro_value_decref(&field_value_instance);

    return 0;
}

int wrapper_destroy_avro_message()
{
    /* Decrement all our references to prevent memory from leaking */
    //fprintf(stdout, "avro_value_decref...\n");
    avro_value_decref(&message_instance);
    //fprintf(stdout, "avro_value_iface_decref...\n");
    avro_value_iface_decref(value_class);
    //serdes_schema_serialize_avro


    return 0;
}

int wrapper_serialiase_and_send_message(char* topic, char* key)
{
    void* ser_key_buf = NULL;
    size_t ser_key_buf_size;
    void* ser_buf = NULL;
    size_t ser_buf_size;
    int error;

    avro_value_t key_value;
    serdes_err_t err;
    rd_kafka_resp_err_t kerr;

    error = avro_generic_string_new(&key_value, key);
    //fprintf(stdout, "    error=%d...\n", error);

    //fprintf(stdout, "    serialising key...\n");
    err = serdes_schema_serialize_avro(key_schema, &key_value,
        &ser_key_buf,
        &ser_key_buf_size,
        errstr,
        sizeof(errstr));
    avro_value_decref(&key_value);
    if (err != SERDES_ERR_OK)
    {
        fprintf(stderr,
            "%% serialize_avro() failed: %s\n",
            errstr);
        //continue;
        free(ser_key_buf);
        return 1;
    }

    clear_last_error();

    //fprintf(stdout, "    serialising payload...\n");
    err = serdes_schema_serialize_avro(value_schema, &message_instance,
        &ser_buf,
        &ser_buf_size,
        errstr,
        sizeof(errstr));
    if (err != SERDES_ERR_OK)
    {
        fprintf(stderr,
            "%% serialize_avro() failed: %s\n",
            errstr);
        free(ser_buf);
        return 2;
    }

    fprintf(stdout,
        "%% Serialised to %zd bytes\n",
        ser_buf_size);


    //fprintf(stdout, "    producing...\n");

    clear_last_error();

    kerr = rd_kafka_producev(rkp,
        RD_KAFKA_V_TOPIC(topic),
        RD_KAFKA_V_KEY(ser_key_buf, ser_key_buf_size),
        RD_KAFKA_V_VALUE(ser_buf, ser_buf_size),
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        RD_KAFKA_V_END);
    if (kerr != RD_KAFKA_RESP_ERR_NO_ERROR) {
        strcpy(errstr, rd_kafka_err2str(kerr));
    }

    free(ser_key_buf);
    free(ser_buf);

    return kerr;

    //if (rd_kafka_produce(rkp, RD_KAFKA_PARTITION_UA,
    //    RD_KAFKA_MSG_F_FREE,
    //    ser_buf, ser_buf_size,
    //    NULL, 0,
    //    NULL) == -1)
    //{
    //    fprintf(stderr,
    //        "%% Failed to produce message: %s\n",
    //        rd_kafka_err2str(rd_kafka_last_error()));

    //    fprintf(stdout, "    freeing...\n");
    //    free(ser_buf);
    //    return 2;
    //}

    //fprintf(stdout,
    //    "%% Produced %zd bytes\n",
    //    ser_buf_size);

    //return 0;
}

/* int wrapper_deserialiase_message_key(rd_kafka_message_t *rkm, const char **key_string)
{
     serdes_err_t err;
     avro_value_t avro;
     const char *p;
     size_t size;
     serdes_schema_t *schema;

     clear_last_error();

     //err = serdes_schema_deserialize_avro(key_schema,
     //                                     &avro,
     //                                     rkm->key, rkm->key_len,
     //                                     errstr, sizeof(errstr));
     err = serdes_deserialize_avro(serdes,
                                   &avro,
                                   &schema,
                                   rkm->key, rkm->key_len,
                                   errstr, sizeof(errstr));
     if (err == SERDES_ERR_OK)
     {
         fprintf(stdout, "    err = %d\n", err);
         err = avro_value_get_string(&avro, key_string, &size);
         if (err == 0)
         {
             //strncpy(key_string, p, sizeof(p));
             //fprintf(stdout, "Field value is: %s\n", p);
             fprintf(stdout, "Field value is: %s\n", *key_string);
         }
         else
         {
             strcpy(errstr, "Failed to deserialise message key!");
             return err;
         }
     }
     avro_value_decref(&avro);
     fprintf(stdout, "ADDR3 [%d]\n", &key_string);
     return err;
}*/

void wrapper_destroy_avro_value(avro_value_t* avro)
{
    avro_value_decref(avro);
}


int wrapper_deserialiase_message_key(rd_kafka_message_t* rkm, avro_value_t *avro)
{
    serdes_err_t err;
    //avro_value_t avro;
    const char* p;
    size_t size;
    serdes_schema_t* schema;

    clear_last_error();

    err = serdes_deserialize_avro(serdes,
        avro,
        &schema,
        rkm->key, rkm->key_len,
        errstr, sizeof(errstr));

    return err;
}

int wrapper_deserialiase_message_value(rd_kafka_message_t* rkm, avro_value_t* avro)
{
    serdes_err_t err;
    //avro_value_t avro;
    const char* p;
    size_t size;
    serdes_schema_t* schema;

    clear_last_error();

    err = serdes_deserialize_avro(serdes,
        avro,
        &schema,
        rkm->payload, rkm->len,
        errstr, sizeof(errstr));

    return err;
}



void wrapper_get_avro_string(avro_value_t* avro_value, const char** string, size_t size)
{
    avro_value_get_string(avro_value, string, &size);
    //fprintf(stdout, "Field value is: %s\n", *string);
}


int wrapper_get_key_from_message(rd_kafka_message_t* rkm, const char** string)
{
    avro_value_t avro_value;
    size_t size;

    wrapper_deserialiase_message_key(rkm, &avro_value);

    //wrapper_get_avro_string(avro_value, string, size);
    avro_value_get_string(&avro_value, string, &size);
    //fprintf(stdout, "Field value is: %s\n", *string);

    avro_value_decref(&avro_value);
}

int wrapper_get_value_field(avro_value_t* avro_source, avro_value_t *avro_field, char* field_name)
{
    serdes_err_t err;
    avro_value_t field_value_instance;

    err = avro_value_get_by_name(avro_source, field_name, avro_field, NULL);

    return err;
}

int wrapper_extract_key_and_value(rd_kafka_message_t* rkm)
{
    serdes_err_t err;

    err = wrapper_deserialiase_message_key(rkm, &key_avro);
    if (err != SERDES_ERR_OK)
    {
        return err;
    }
    err = wrapper_deserialiase_message_value(rkm, &value_avro);
    if (err != SERDES_ERR_OK)
    {
        return err;
    }
}

//const char* wrapper_get_key()
//{
//    const char* string;
//    size_t size;
//
//    avro_value_get_string(&key_avro, &string, &size);
//    //fprintf(stdout, "Field value is: %s\n", *string);
//
//    return string;
//}

void wrapper_get_key(char* out_string)
{
    const char* string;
    size_t size;

    avro_value_get_string(&key_avro, &string, &size);
    //fprintf(stdout, "Field value is: %s\n", *string);

    strcpy(out_string, string);
}

//const char* wrapper_get_value_field_string(char* field_name)
//{
//    const char* string;
//    size_t size;
//
//    serdes_err_t err;
//    avro_value_t avro_field;
//
//    err = avro_value_get_by_name(&value_avro, field_name, &avro_field, NULL);
//    if (err != SERDES_ERR_OK)
//    {
//        return NULL;
//    }
//
//    avro_value_get_string(&avro_field, &string, &size);
//
//    // this doesn't need to be freed
//    //wrapper_destroy_avro_value(&avro_field);
//
//    return string;
//}

int wrapper_get_value_field_string(char* field_name, char* out_string)
{
    const char* string;
    size_t size;

    serdes_err_t err;
    avro_value_t avro_field;

    err = avro_value_get_by_name(&value_avro, field_name, &avro_field, NULL);
    if (err != SERDES_ERR_OK)
    {
        return err;
    }

    avro_value_get_string(&avro_field, &string, &size);

    // this doesn't need to be freed
    //wrapper_destroy_avro_value(&avro_field);
    
    strcpy(out_string, string);

    return 0;
}


void wrapper_clear_values()
{
    avro_value_decref(&key_avro);
    avro_value_decref(&value_avro);
}

void destroy_key_value()
{
    avro_value_decref(&key_avro);
    avro_value_decref(&value_avro);
}
/*

char* wrapper_get_value()
{
    char* string;
    size_t size;

    avro_value_get_string(&key_avro, string, &size);
    //fprintf(stdout, "Field value is: %s\n", *string);

    avro_value_decref(&key_avro);
}
*/