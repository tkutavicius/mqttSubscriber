#include "mqtt_sub.h"
#include <time.h>
#include <sqlite3.h>

static char *formatDate()
{
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);
        static char timestamp[19];

        int day = local->tm_mday;
        int month = local->tm_mon + 1;
        int year = local->tm_year + 1900;
        int hours = local->tm_hour;
        int minutes = local->tm_min;
        int seconds = local->tm_sec;

        sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds);
        return timestamp;
}

extern int saveMessage(char *topic, char *payload)
{
        int rc = 0;
        sqlite3 *db;
        char *err_msg = 0;
        char *sql = NULL;
        sql = malloc((strlen(topic) + strlen(payload) + 90) * sizeof(char));
        if (sql == NULL) {
                goto clean;
                rc = -1;
        }

        rc = sqlite3_open("/etc/mosquitto/messages.db", &db);
        if (rc != SQLITE_OK) {
                fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                goto clean;
        }

        sprintf(sql, "INSERT INTO MESSAGE (topic, payload, timestamp) VALUES ('%s', '%s', '%s');\n", topic, payload, formatDate());

        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
        }

        clean:
                sqlite3_free(err_msg);
                sqlite3_close(db);
                free(sql);

        return rc;
}