#include <stdio.h>
#include <mosquitto.h>
#include <signal.h>
#include <uci.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <sqlite3.h>

typedef struct topic
{
        char name[60];
        int QoS;
} topic;

typedef struct broker
{
        char remote_addr[256];
        char remote_port[6];
        char userName[20];
        char password[20];
} broker;

volatile sig_atomic_t deamonize = 1;

void term_proc(int sigterm)
{
        deamonize = 0;
}

int saveMessage(char *topic, char* payload)
{
        int rc = 0;
        sqlite3 *db;
        char *err_msg = 0;
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);
        char sql[150];
        
        int day = local->tm_mday;
        int month = local->tm_mon + 1;
        int year = local->tm_year + 1900;
        int hours = local->tm_hour;
        int minutes = local->tm_min;
        int seconds = local->tm_sec;
    
        rc = sqlite3_open("/etc/mosquitto/messages.db", &db);

        if (rc != SQLITE_OK) 
        {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                goto clean;
        }

        sprintf(sql,"INSERT INTO MESSAGE (topic, payload, timestamp) VALUES ('%s', '%s', '%d-%02d-%02d %02d:%02d:%02d');\n"
                ,topic, payload, year, month, day, hours, minutes, seconds);

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

void subscribe_cb(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
        fprintf(stdout, "Subscription successful! QoS: %d\n", granted_qos[0]);
}

struct uci_package *loadConfig(char *config_name)
{
        struct uci_context *ctx;
        struct uci_package *pkg;
        if ((ctx = uci_alloc_context()) == NULL)
        {
                return NULL;
        }
        if (uci_load(ctx, config_name, &pkg) != UCI_OK)
        {
                return NULL;
        }
        return pkg;
}

int countOptions(struct uci_package *p)
{
        int c = 0;
        struct uci_element *i;
        uci_foreach_element(&p->sections, i)
        {
                c++;
        }
        return c;
}

int getTopics(char *config_name, struct topic **allTopics)
{
        struct uci_package *p = NULL;
        struct uci_element *i, *j;
        int c = 0;

        if ((p = loadConfig(config_name)) == NULL)
        {
                return -1;
        }

        if ((*allTopics = malloc(sizeof(struct topic) * countOptions(p))) == NULL)
        {
                return -1;
        }

        uci_foreach_element(&p->sections, i)
        {
                struct uci_section *section = uci_to_section(i);
                uci_foreach_element(&section->options, j)
                {
                        struct uci_option *option = uci_to_option(j);
                        char *option_name = option->e.name;

                        if (strcmp(option_name, "topicName") == 0)
                        {
                                strcpy((*allTopics)[c].name, option->v.string);
                        }
                        else if (strcmp(option_name, "qos") == 0)
                        {
                                (*allTopics)[c].QoS = (int)strtol(option->v.string, (char **)NULL, 10);
                        }
                }
                c++;
        }
        return c;
}

int getOption(char *path, char *option)
{
        struct uci_context *c = uci_alloc_context();
        struct uci_ptr ptr;
        int rc = 0;
        if (uci_lookup_ptr(c, &ptr, path, true) != UCI_OK || ptr.o == NULL)
        {
                rc = -1;
        }
        else
        {
                strcpy(option, ptr.o->v.string);
        }
        uci_free_context(c);
        return rc;
}

int getBroker(struct broker *mqttBroker)
{
        char entry[30];
        strcpy(entry, "mqtt_sub.mqtt_sub.remote_addr");
        if (getOption(entry, mqttBroker->remote_addr) != 0)
        {
                return -1;
        }
        strcpy(entry, "mqtt_sub.mqtt_sub.remote_port");
        if (getOption(entry, mqttBroker->remote_port) != 0)
        {
                return -1;
        }
        strcpy(entry, "mqtt_sub.mqtt_sub.username");
        if (getOption(entry, mqttBroker->userName) != 0)
        {
                mqttBroker->userName[0] = 0;
        }
        strcpy(entry, "mqtt_sub.mqtt_sub.password");
        if (getOption(entry, mqttBroker->password) != 0)
        {
                mqttBroker->password[0] = 0;
        }
        return 0;
}

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

int connectToBroker(struct mosquitto **mosq)
{
        int rc = 0;
        int port = 0;
        topic *allTopics = NULL;
        broker mqttBroker;

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
        if (mqttBroker.userName[0] != 0 && mqttBroker.password[0] != 0)
        {
                if (mosquitto_username_pw_set(*mosq, mqttBroker.userName, mqttBroker.password))
                {
                        fprintf(stderr, "Can't set username and password!\n");
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
        FILE *fp;
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
