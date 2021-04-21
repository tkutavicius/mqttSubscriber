#ifndef MQTT_SUB_H
#define MQTT_SUB_H

#include <stdlib.h>
#include <string.h>
#include <mosquitto.h> 
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>

typedef struct event {
        char topicName[64];
        char attributeType[10];
        char attribute[128];
        char decimalComparator[1];
        char stringComparator[15];
        char attributeValue[128];
        char recipientEmail[128];
        char smtpIP[128];
        char smtpPort[6];
        char userName[128];
        char password[64];
        char senderEmail[128];
        char secure[1];
} event;
  
typedef struct topic {
        char name[60];
        int QoS;
        int eC;
        event topicEvents[20];
} topic;

typedef struct broker {
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

extern int connectToBroker(struct mosquitto **mosq);
  
#endif