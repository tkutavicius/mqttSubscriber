#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

int getBroker(struct broker *mqttBroker);
int getTopics(char *config_name, struct topic **allTopics);
  
#endif