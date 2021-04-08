#ifndef MQTT_SUB_H
#define MQTT_SUB_H

#include <stdlib.h>
#include <string.h>
#include <mosquitto.h> 
#include <stdbool.h>
  
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
        bool useTls;
        bool insecureTls;
        char tlsType[5];
        char pskIdentity[50];
        char psk[256];
        char clientCert[50];
        char clientKey[50];
        char caCert[50];
} broker;

void term_proc(int sigterm);
int saveMessage(char *topic, char* payload);
int topicsSubscription(struct mosquitto ***mosq, struct topic *allTopics);
int connectToBroker(struct mosquitto **mosq);
  
#endif