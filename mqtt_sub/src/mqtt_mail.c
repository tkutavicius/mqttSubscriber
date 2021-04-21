#include <curl/curl.h>
#include "mqtt_mail.h"

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;

  size_t bytes_to_copy = upload_ctx->sizeleft < size * nmemb ? upload_ctx->sizeleft : size * nmemb;
  bytes_to_copy = upload_ctx->sizeleft < 10 ? upload_ctx->sizeleft : 10;

  if (size * nmemb < 1)
    return 0;

  if (upload_ctx->sizeleft)
  {
    memcpy((char *)ptr, (char *)upload_ctx->readptr, bytes_to_copy);
    upload_ctx->readptr += bytes_to_copy;
    upload_ctx->sizeleft -= bytes_to_copy;
    return bytes_to_copy;
  }

  return 0;
}

static int setupSMTP(struct event topicEvent, char **smtp_from, char **smtp_username, char **smtp_password, char **smtp_server_url)
{
  *smtp_from = malloc(sizeof(topicEvent.senderEmail) * sizeof(char) + 17);
  if (*smtp_from == NULL) {
    fprintf(stderr, "Cannot allocate memory to SMTP sender email!");
    return -1;
  }

  *smtp_username = malloc(sizeof(topicEvent.userName) * sizeof(char) + 1);
  if (*smtp_username == NULL) {
    fprintf(stderr, "Cannot allocate memory to SMTP server username!");
    return -1;
  }

  *smtp_password = malloc(sizeof(topicEvent.password) * sizeof(char) + 1);
  if (*smtp_password == NULL) {
    fprintf(stderr, "Cannot allocate memory to SMTP server password!");
    return -1;
  }

  *smtp_server_url = malloc((sizeof(topicEvent.smtpIP) + sizeof(topicEvent.smtpPort)) * sizeof(char) + 10);
  if (*smtp_server_url == NULL) {
    fprintf(stderr, "Cannot allocate memory to SMTP server address!");
    return -1;
  }

  sprintf(*smtp_from, "TeltonikaRUTX10 %s", topicEvent.senderEmail);
  sprintf(*smtp_username, "%s", topicEvent.userName);
  sprintf(*smtp_password, "%s", topicEvent.password);
  sprintf(*smtp_server_url, "smtps://%s:%s", topicEvent.smtpIP, topicEvent.smtpPort);
  return 0;
}

static int setupCurl(CURL **curl, char *smtp_username, char *smtp_password, char *smtp_server_url,
  char* secure, char* smtp_from, struct curl_slist *recipients)
{
  curl_easy_setopt(*curl, CURLOPT_USERNAME, smtp_username);
  curl_easy_setopt(*curl, CURLOPT_PASSWORD, smtp_password);
  curl_easy_setopt(*curl, CURLOPT_URL, smtp_server_url);
  if (strcmp(secure, "1") == 0) {
    curl_easy_setopt(*curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(*curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(*curl, CURLOPT_SSL_VERIFYHOST, 0L);
  }
  curl_easy_setopt(*curl, CURLOPT_MAIL_FROM, smtp_from);
  curl_easy_setopt(*curl, CURLOPT_MAIL_RCPT, recipients);
  curl_easy_setopt(*curl, CURLOPT_READFUNCTION, payload_source);
  return 0;
}

extern int sendMail(struct event topicEvent, char* receivedValue)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
  char *smtp_from = NULL;
  char *smtp_username = NULL;
  char *smtp_password = NULL;
  char *smtp_server_url = NULL;

  char payload_template[] =
    "To: %s\r\n"
    "From: %s\r\n"
    "Subject: MQTT Event Information\r\n\r\n"
    "An event was captured for topic's '%s' attribute '%s'! Captured value was '%s', attribute value is '%s'. \r\n\r\n";

  if (setupSMTP(topicEvent, &smtp_from, &smtp_username, &smtp_password, &smtp_server_url) != 0)
  {
    fprintf(stderr, "Cannot setup SMTP server!");
    res = CURLE_FAILED_INIT;
    goto clean;
  }

  curl = curl_easy_init();
  if (curl) {
    size_t payload_text_len = strlen(payload_template) +
      strlen(topicEvent.recipientEmail) +
      strlen(smtp_from) +
      strlen(topicEvent.topicName) +
      strlen(topicEvent.attribute) +
      strlen(receivedValue) +
      strlen(topicEvent.attributeValue) + 1;

    char *payload_text = malloc(payload_text_len);

    memset(payload_text, 0, payload_text_len);

    sprintf(payload_text, payload_template, topicEvent.recipientEmail,
            smtp_from, topicEvent.topicName, topicEvent.attribute, receivedValue,
            topicEvent.attributeValue);

    upload_ctx.readptr = payload_text;
    upload_ctx.sizeleft = (long)strlen(payload_text);

    recipients = curl_slist_append(recipients, topicEvent.recipientEmail);

    setupCurl(&curl, smtp_username, smtp_password, smtp_server_url, topicEvent.secure, smtp_from,
    recipients);

    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed %s!\n", curl_easy_strerror(res));
    }
    else {
      fprintf(stderr, "Email was sent!\n");
    }
    
    free(payload_text);
  }
  clean:
    free(smtp_from);
    free(smtp_username);
    free(smtp_password);
    free(smtp_server_url);
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  return (int)res;
}