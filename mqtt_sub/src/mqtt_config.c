#include "mqtt_sub.h"
#include <uci.h>

static struct uci_package *loadConfig(char *config_name)
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

static int countOptions(struct uci_package *p)
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

static int getOption(char *path, char *option)
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
        char entry[40];
        char tls[1];
        char insecure[1];
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
        strcpy(entry, "mqtt_sub.mqtt_sub.tls");
        if (getOption(entry, tls) != 0)
        {
                mqttBroker->useTls = false;
        }
        else
        {
                mqttBroker->useTls = true;
        }
        if (mqttBroker->useTls)
        {
                strcpy(entry, "mqtt_sub.mqtt_sub.tls_type");
                if (getOption(entry, mqttBroker->tlsType) != 0)
                {
                        return -1;
                }
                if (strcmp(mqttBroker->tlsType, "cert") == 0)
                {
                        strcpy(entry, "mqtt_sub.mqtt_sub.certfile");
                        if (getOption(entry, mqttBroker->clientCert) != 0)
                        {
                                return -1;
                        }
                        strcpy(entry, "mqtt_sub.mqtt_sub.cafile");
                        if (getOption(entry, mqttBroker->caCert) != 0)
                        {
                                return -1;
                        }
                        strcpy(entry, "mqtt_sub.mqtt_sub.keyfile");
                        if (getOption(entry, mqttBroker->clientKey) != 0)
                        {
                                return -1;
                        }
                        strcpy(entry, "mqtt_sub.mqtt_sub.tls_insecure");
                        if (getOption(entry, insecure) != 0)
                        {
                                mqttBroker->insecureTls = false;
                        }
                        else
                        {
                                mqttBroker->insecureTls = true;
                        }
                }
                else if (strcmp(mqttBroker->tlsType, "psk") == 0)
                {
                        strcpy(entry, "mqtt_sub.mqtt_sub.identity");
                        if (getOption(entry, mqttBroker->pskIdentity) != 0)
                        {
                                return -1;
                        }
                        strcpy(entry, "mqtt_sub.mqtt_sub.psk");
                        if (getOption(entry, mqttBroker->psk) != 0)
                        {
                                return -1;
                        }
                }
                else
                {
                        return -1;
                }
        }
        return 0;
}