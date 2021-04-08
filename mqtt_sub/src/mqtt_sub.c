#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <malloc.h>
#include <sqlite3.h>
#include "mqtt_sub.h"
#include "mqtt_config.h"

volatile sig_atomic_t deamonize = 1;

void term_proc(int sigterm)
{
        deamonize = 0;
}

static void message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
        if (message->payloadlen)
        {
                if (saveMessage(message->topic, message->payload) != 0)
                {
                        fprintf(stderr, "The incoming message was not saved!\n");
                }
        }
}

static void connect_cb(struct mosquitto *mosq, void *userdata, int rc)
{
        if (rc)
        {
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

int topicsSubscription(struct mosquitto ***mosq, struct topic *allTopics)
{
        int tC = (malloc_usable_size(allTopics) / sizeof(topic));
        for (int i = 0; i < tC; i++)
        {
                if ((mosquitto_subscribe(**mosq, NULL, allTopics[i].name, allTopics[i].QoS)) != MOSQ_ERR_SUCCESS)
                {
                        fprintf(stderr, "Unable to subscribe to topic!\n");
                        return -1;
                }
        }
        return 0;
}

char *formatDate()
{
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);
        static char timestamp[19];

        int day = local->tm_mday;
        int month = local->tm_mon + 1;
        int year = local->tm_year + 1900;
        int hours = local->tm_hour;
        int minutes = local->tm_min;
        int seconds = local->tm_sec;

        sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds);
        return timestamp;
}

int saveMessage(char *topic, char *payload)
{
        int rc = 0;
        sqlite3 *db;
        char *err_msg = 0;
        char sql[150];

        rc = sqlite3_open("/etc/mosquitto/messages.db", &db);

        if (rc != SQLITE_OK)
        {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                goto clean;
        }

        sprintf(sql, "INSERT INTO MESSAGE (topic, payload, timestamp) VALUES ('%s', '%s', '%s');\n", topic, payload, formatDate());

        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK)
        {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                goto clean;
        }

clean:
        sqlite3_free(err_msg);
        sqlite3_close(db);

        return rc;
}

int connectToBroker(struct mosquitto **mosq)
{
        int rc = 0;
        int port = 0;
        topic *allTopics = NULL;
        broker mqttBroker;

        mosquitto_lib_init();

        if ((getBroker(&mqttBroker)) == -1)
        {
                fprintf(stderr, "Unable to get broker settings!\n");
                rc = -1;
                goto clean;
        }
        if (getTopics("mqtt_topics", &allTopics) == -1)
        {
                fprintf(stderr, "Unable to get topics for subscription!\n");
                rc = -1;
                goto clean;
        }

        if ((*mosq = mosquitto_new(NULL, true, NULL)) == NULL)
        {
                fprintf(stderr, "Unable to initialize instance of broker!\n");
                rc = -1;
                goto clean;
        }

        port = (int)strtol(mqttBroker.remote_port, (char **)NULL, 10);
        mosquitto_connect_callback_set(*mosq, connect_cb);
        mosquitto_message_callback_set(*mosq, message_cb);
        mosquitto_subscribe_callback_set(*mosq, subscribe_cb);
        //mosquitto_log_callback_set(*mosq, log_cb);
        if (mqttBroker.userName[0] != 0 && mqttBroker.password[0] != 0)
        {
                if (mosquitto_username_pw_set(*mosq, mqttBroker.userName, mqttBroker.password))
                {
                        fprintf(stderr, "Can't set username and password!\n");
                        rc = -1;
                        goto clean;
                }
        }
        if (mqttBroker.useTls)
        {
                if (strcmp(mqttBroker.tlsType, "cert") == 0)
                {
                        if (mosquitto_tls_set(*mosq, mqttBroker.caCert, NULL, mqttBroker.clientCert, mqttBroker.clientKey, NULL))
                        {
                                fprintf(stderr, "Can't set TLS/SSL certificate!\n");
                                rc = -1;
                                goto clean;
                        }
                        if (mqttBroker.insecureTls)
                        {
                                if (mosquitto_tls_insecure_set(*mosq, true))
                                {
                                        fprintf(stderr, "Can't set TLS/SSL insecure option!\n");
                                        rc = -1;
                                        goto clean;
                                }
                        }
                }
                else if (strcmp(mqttBroker.tlsType, "psk") == 0)
                {
                        if (mosquitto_tls_psk_set(*mosq, mqttBroker.psk, mqttBroker.pskIdentity, NULL))
                        {
                                fprintf(stderr, "Can't set TLS/SSL PSK!\n");
                                rc = -1;
                                goto clean;
                        }
                }
                else
                {
                        fprintf(stderr, "Unable to set TLS/SSL!\n");
                        rc = -1;
                        goto clean;
                }
                if (mosquitto_tls_opts_set(*mosq, 1, NULL, NULL))
                {
                        fprintf(stderr, "Can't set TLS/SSL options!\n");
                        rc = -1;
                        goto clean;
                }
        }
        if (mosquitto_connect(*mosq, mqttBroker.remote_addr, port, 60))
        {
                fprintf(stderr, "Can't connect to Mosquitto broker!\n");
                rc = -1;
                goto clean;
        }
        if (topicsSubscription(&mosq, allTopics) != 0)
        {
                rc = -1;
                fprintf(stderr, "Can't connect to Mosquitto broker!\n");
                goto clean;
        }
clean:
        free(allTopics);
        return rc;
}

int main(int argc, char *argv[])
{
        struct mosquitto *mosq = NULL;
        struct sigaction action;
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_handler = term_proc;
        sigaction(SIGTERM, &action, NULL);

        mosquitto_lib_init();

        if (connectToBroker(&mosq) != 0)
        {
                goto clean;
        }

        while (deamonize)
        {
                if (mosquitto_loop(mosq, -1, 1) != MOSQ_ERR_SUCCESS)
                {
                        fprintf(stderr, "Lost connection to Mosquitto broker!\n");
                        goto clean;
                }
        }

clean:
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 0;
}
