#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#define MAX_HTTP_RECV_BUFFER 2048
#define MAX_HTTP_OUTPUT_BUFFER 2048

void http_data_event_callback(char* response, int length);
void http_error_event_callback();

void http_post(const char* url, const char* payload);

void http_get(const char* url);

#endif