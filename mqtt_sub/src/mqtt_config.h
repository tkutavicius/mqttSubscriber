#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

extern int getBroker(struct broker *mqttBroker);
extern int getTopics(char *config_name, struct topic **allTopics);
extern int getEvents(char *config_name, struct topic **allTopics, int tC);
  
#endif