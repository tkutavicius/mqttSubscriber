#include <stdio.h>
#include <mosquitto.h>
#include <signal.h>
#include <uci.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

volatile sig_atomic_t deamonize = 1;

void term_proc(int sigterm)
{
        deamonize = 0;
}

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
        FILE *fp;
        time_t now;
        time(&now);

        struct tm *local = localtime(&now);

        int day = local->tm_mday;
        int month = local->tm_mon + 1;
        int year = local->tm_year + 1900;
        int hours = local->tm_hour;
        int minutes = local->tm_min;
        int seconds = local->tm_sec;

        if (message->payloadlen)
        {
                fp = fopen("/tmp/mqttMessages", "a");
                fprintf(fp, "[%d-%02d-%02d %02d:%02d:%02d] [%s] %s\n", year, month, day,
                        hours, minutes, seconds, message->topic, message->payload);
                fclose(fp);
        }
}

int uci_get_value(struct uci_option *o, char *option)
{
        struct uci_element *e;
        int rc = 0;
        switch (o->type)
        {
        case UCI_TYPE_STRING:
                strcpy(option, o->v.string);
                rc = 0;
                break;
        default:
                rc = 1;
                break;
        }
        return rc;
}

int get_config_entry(char *path, char *option)
{
        struct uci_context *c = uci_alloc_context();
        struct uci_ptr ptr;
        int rc = 0;
        if (uci_lookup_ptr(c, &ptr, path, true) != UCI_OK || ptr.o == NULL)
        {
                rc = 1;
        }
        else
        {
                if (uci_get_value(ptr.o, option) != 0)
                {
                        rc = 1;
                }
        }
        uci_free_context(c);
        return rc;
}

struct mosquitto *connectToBroker()
{
        struct mosquitto *mosq = NULL;
        char *entry = NULL;
        char *remote_addr = NULL;
        char *remote_port = NULL;
        char *topic = NULL;
        int cPort;
        int rc = 0;
        int keepalive = 60;
        bool clean_session = true;
        entry = malloc(sizeof(char) * 30);
        remote_addr = malloc(sizeof(char) * 20);
        remote_port = malloc(sizeof(char) * 6);
        topic = malloc(sizeof(char) * 20);
        strcpy(entry, "mqtt_sub.mqtt_sub.remote_addr");
        if (get_config_entry(entry, remote_addr) != 0)
        {
                fprintf(stderr, "Cannot parse broker address!\n");
                goto clean;
        }
        strcpy(entry, "mqtt_sub.mqtt_sub.remote_port");
        if (get_config_entry(entry, remote_port) != 0)
        {
                fprintf(stderr, "Cannot parse broker port!\n");
                goto clean;
        }
        strcpy(entry, "mqtt_sub.mqtt_sub.topic");
        if (get_config_entry(entry, topic) != 0)
        {
                fprintf(stderr, "Cannot parse topic!\n");
                goto clean;
        }

        mosq = mosquitto_new(NULL, clean_session, NULL);
        if (!mosq)
        {
                fprintf(stderr, "Out of memory!\n");
                goto clean;
        }
        else
        {
                cPort = (int)strtol(remote_port, (char **)NULL, 10);

                mosquitto_message_callback_set(mosq, message_callback);

                if (mosquitto_connect(mosq, remote_addr, cPort, keepalive))
                {
                        fprintf(stderr, "Unable to connect to broker!\n");
                        mosq = NULL;
                        goto clean;
                }

                if (mosquitto_subscribe(mosq, NULL, topic, 1))
                {
                        fprintf(stderr, "Unable to subscribe to topic '%s'!\n", topic);
                        mosq = NULL;
                        goto clean;
                }
        }

        fprintf(stdout, "Connected to '%s:%d' with topic '%s'\n", remote_addr, cPort, topic);

        clean:
                free(remote_addr);
                free(remote_port);
                free(topic);
        return mosq;
}

int main(int argc, char *argv[])
{
        struct mosquitto *mosq = NULL;
        FILE *fp;
        struct sigaction action;
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_handler = term_proc;
        sigaction(SIGTERM, &action, NULL);

        fp = fopen("/tmp/mqttMessages", "w+");
        fclose(fp);

        mosquitto_lib_init();

        if (!(mosq = connectToBroker()))
        {
                goto clean;
        }

        while (deamonize)
        {
                mosquitto_loop(mosq, -1, 1);
        }

clean:
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 0;
}
