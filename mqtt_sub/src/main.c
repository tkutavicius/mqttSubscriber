#include "mqtt_sub.h"

volatile sig_atomic_t deamonize = 1;

void term_proc(int sigterm){
        deamonize = 0;
}

int main(int argc, char *argv[])
{
        struct mosquitto *mosq = NULL;
        struct sigaction action;
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_handler = term_proc;
        sigaction(SIGTERM, &action, NULL);

        mosquitto_lib_init();

        if (connectToBroker(&mosq) != 0){
                goto clean;
        }

        while (deamonize){
                if (mosquitto_loop(mosq, -1, 1) != MOSQ_ERR_SUCCESS){
                        fprintf(stderr, "Lost connection to Mosquitto broker!\n");
                        goto clean;
                }
        }

    clean:
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        
    return 0;
}