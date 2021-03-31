#ifndef MQTT_SUB_H
#define MQTT_SUB_H

#include <stdlib.h>
#include <string.h>
  
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

void term_proc(int sigterm);
int saveMessage(char *topic, char* payload);
static void message_cb(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
static void connect_cb(struct mosquitto *mosq, void *userdata, int rc);
static void subscribe_cb(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
int topicsSubscription(struct mosquitto ***mosq, struct topic *allTopics);
int connectToBroker(struct mosquitto **mosq);
  
#endif