#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

static struct uci_package *loadConfig(char *config_name);
static int countOptions(struct uci_package *p);
static int getOption(char *path, char *option);
int getBroker(struct broker *mqttBroker);
int getTopics(char *config_name, struct topic **allTopics);
  
#endif