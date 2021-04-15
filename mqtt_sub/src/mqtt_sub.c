#include "mqtt_sub.h"
#include "mqtt_config.h"
#include "mqtt_db.h"
#include "mqtt_events.h"

static void message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
        topic *allTopics = (topic *)userdata;
        int tC = 0;
        if (message->payloadlen == 0) {
                return;
        }

        if (saveMessage(message->topic, message->payload) != 0) {
                fprintf(stderr, "The incoming message was not saved!\n");
        }

        tC = (malloc_usable_size(allTopics) / sizeof(topic));
        for (int i = 0; i < tC; i++) {
                if (strcmp(message->topic, allTopics[i].name) != 0 || allTopics[i].eC == 0) {
                        continue;
                }
                if (handleEvents(allTopics[i].topicEvents, allTopics[i].eC, message->payload) != 0) {
                        fprintf(stderr, "Events were not processed!\n");
                }
                else {
                        fprintf(stdout, "Events were processed!\n"); 
                }
        }
}

static void connect_cb(struct mosquitto *mosq, void *userdata, int rc)
{
        if (rc) {
                fprintf(stderr, "%s\n", mosquitto_connack_string(rc));
        }
}

static void subscribe_cb(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
        fprintf(stdout, "Subscription successful! QoS: %d\n", granted_qos[0]);
}

/*static void log_cb(struct mosquitto *mosq, void *obj, int level, const char *str)
{
        fprintf(stdout, "level:%d,\n\tstr:%s\n", level, str);
}*/

static int topicsSubscription(struct mosquitto ***mosq, struct topic *allTopics)
{
        int tC = (malloc_usable_size(allTopics) / sizeof(topic));
        for (int i = 0; i < tC; i++) {
                if ((mosquitto_subscribe(**mosq, NULL, allTopics[i].name, allTopics[i].QoS)) != MOSQ_ERR_SUCCESS) {
                        fprintf(stderr, "Unable to subscribe to topic!\n");
                        return -1;
                }
        }
        return 0;
}

static int setupTLS(struct broker mqttBroker, struct mosquitto ***mosq)
{
        if (strcmp(mqttBroker.tlsType, "cert") == 0) {
                if (mosquitto_tls_set(**mosq, mqttBroker.caCert, NULL, mqttBroker.clientCert, mqttBroker.clientKey, NULL)) {
                        fprintf(stderr, "Can't set TLS/SSL certificate!\n");
                        return -1;
                }
                if (mqttBroker.insecureTls) {
                        if (mosquitto_tls_insecure_set(**mosq, true)) {
                                fprintf(stderr, "Can't set TLS/SSL insecure option!\n");
                                return -1;
                        }
                }
        }
        else if (strcmp(mqttBroker.tlsType, "psk") == 0) {
                if (mosquitto_tls_psk_set(**mosq, mqttBroker.psk, mqttBroker.pskIdentity, NULL)) {
                        fprintf(stderr, "Can't set TLS/SSL PSK!\n");
                        return -1;
                }
        }
        else {
                fprintf(stderr, "Unable to set TLS/SSL!\n");
                return -1;
        }
        if (mosquitto_tls_opts_set(**mosq, 1, NULL, NULL)) {
                fprintf(stderr, "Can't set TLS/SSL options!\n");
                return -1;
        }
        return 0;
}

extern int connectToBroker(struct mosquitto **mosq)
{
        int port = 0;
        int tC = 0;
        broker mqttBroker;
        topic *allTopics = NULL;

        if ((getBroker(&mqttBroker)) == -1) {
                fprintf(stderr, "Unable to get broker settings!\n");
                return -1;
        }

        if ((tC = getTopics("mqtt_topics", &allTopics)) == -1) {
                fprintf(stderr, "Unable to get topics for subscription!\n");
                return -1;
        }

        if (getEvents("mqtt_events", &allTopics, tC) == -1) {
                fprintf(stderr, "No events was found!\n");
        }

        if ((*mosq = mosquitto_new(NULL, true, allTopics)) == NULL) {
                fprintf(stderr, "Unable to initialize instance of broker!\n");
                return -1;
        }

        port = (int)strtol(mqttBroker.remote_port, (char **)NULL, 10);
        mosquitto_connect_callback_set(*mosq, connect_cb);
        mosquitto_message_callback_set(*mosq, message_cb);
        mosquitto_subscribe_callback_set(*mosq, subscribe_cb);
        //mosquitto_log_callback_set(*mosq, log_cb);
        if (mqttBroker.userName[0] != 0 && mqttBroker.password[0] != 0) {
                if (mosquitto_username_pw_set(*mosq, mqttBroker.userName, mqttBroker.password)) {
                        fprintf(stderr, "Can't set username and password!\n");
                        return -1;
                }
        }
        if (mqttBroker.useTls) {
                if (setupTLS(mqttBroker, &mosq) != 0) {
                        return -1;
                }
        }
        if (mosquitto_connect(*mosq, mqttBroker.remote_addr, port, 60)) {
                fprintf(stderr, "Can't connect to Mosquitto broker!\n");
                return -1;
        }
        if (topicsSubscription(&mosq, allTopics) != 0) {
                fprintf(stderr, "Can't connect to Mosquitto broker!\n");
                return -1;
        }
        return 0;
}
