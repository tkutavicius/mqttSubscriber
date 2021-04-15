#include <json-c/json.h>
#include "mqtt_sub.h"

#define lt 0
#define gt 1
#define ge 2
#define le 3
#define eq 4
#define ne 5

static int processStringValue(struct event topicEvent, char *value)
{
        int rc = 0;
        if (strcmp(topicEvent.stringComparator, "Equal To") == 0) {
                if (strcmp(topicEvent.attributeValue, value) == 0) {
                        rc = 1;
                }
        }
        else if (strcmp(topicEvent.stringComparator, "Not Equal To") == 0) {
                if (strcmp(topicEvent.attributeValue, value) == 0) {
                        rc = 1;
                }
        }
        return rc;
}

static int processDecimalValue(struct event topicEvent, char *value)
{
        int rc = 0;
        float eventValue = strtof(topicEvent.attributeValue, NULL);
        float jsonValue = strtof(value, NULL);
        int v = (int)strtol(topicEvent.decimalComparator, (char **)NULL, 10);
        switch(v){
                case lt:
                        if (jsonValue > eventValue) {
                                rc = 1;
                        }
                        break;
                case gt:
                        if (jsonValue < eventValue) {
                                rc = 1;
                        }
                        break;
                case ge:
                        if (jsonValue >= eventValue) {
                                rc = 1;
                        }
                        break;
                case le:
                        if (jsonValue <= eventValue) {
                                rc = 1;
                        }
                        break;
                case eq:
                        if (jsonValue == eventValue) {
                                rc = 1;
                        }
                        break;
                case ne:
                        if (jsonValue != eventValue) {
                                rc = 1;
                        }
                        break;
                default:
                        break;
        }
        return rc;
}

static int sendMail()
{
        printf("Siunciamas message\n");
        return 0;
}

static int checkValues(struct event topicEvent, char *value)
{
        int rc = 0;
        if (strcmp(topicEvent.attributeType, "Decimal") == 0) {
                rc = processDecimalValue(topicEvent, value);
        }
        else if (strcmp(topicEvent.attributeType, "String") == 0) {
                rc = processStringValue(topicEvent, value);
        }
        return rc;
}

static char *getJSONValue(struct json_object *val)
{
        enum json_type type;
        char *value = NULL;
        value = malloc(sizeof(val) * sizeof(char));
        if (value == NULL) {
                return "";
        }
        type = json_object_get_type(val);
        switch (type) {
                case json_type_string:
                        sprintf(value, "%s", json_object_get_string(val));
                        break;
                case json_type_boolean:
                        sprintf(value, "%s", json_object_get_boolean(val) ? "true" : "false");
                        break;
                case json_type_double:
                        sprintf(value, "%f", json_object_get_double(val));
                        break;
                case json_type_int:
                        sprintf(value, "%d", json_object_get_int(val));
                        break;
                default:
                        fprintf(stderr, "Cannot parse value from JSON object!");
                        strcpy(value, "");
        }
        return value;
}

extern int handleEvents(struct event *topicEvents, int eC, char *payload)
{
        struct json_object *pJ;
        char *value = NULL;

        if ((pJ = json_tokener_parse(payload)) == NULL) {
                fprintf(stderr, "Message is not in JSON format!\n");
                return -1;
        }

        json_object_object_foreach(pJ, key, val) {
                for (int i = 0; i < eC; i++) {
                        if (strcmp(key, topicEvents[i].attribute) != 0) {
                                continue;
                        }
                        
                        value = getJSONValue(val);
                        
                        if (strcmp(value, "") != 0) {
                                if (checkValues(topicEvents[i], value) == 1) {
                                        sendMail();
                                }
                        }
                        else {
                                return -1;
                        }
                        free(value);
                }
        }
        return 0;
}