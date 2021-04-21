#ifndef MQTT_MAIL_H
#define MQTT_MAIL_H

#include "mqtt_sub.h"

struct upload_status {
    const char *readptr;
    size_t sizeleft;
};

extern int sendMail(struct event topicEvent, char* receivedValue);

#endif