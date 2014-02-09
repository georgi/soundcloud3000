#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "soundcloud3000.h"

static int http_connect(const char *host)
{
    int fd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    if ((server = gethostbyname(host)) == NULL) {
        fprintf(stderr, "no such host %s\n", host);
        return -1;
    }
    
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "could not open socket");
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(80);

    if (connect(fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  {
        fprintf(stderr, "could not connect to %s\n", host);
        return -1;
    }

    return fd;
}

int http_header(Response *response, const char *key, char *value)
{
    char *pos = strstr(response->headers, key);

    if (pos == NULL) {
        return -1;
    }
    
    if (sscanf(pos + strlen(key), ": %s\r\n", value) != 1) {
        return -1;
    }

    return 0;
}

Response *http_request_url(const char *url)
{
    char host[256];
    char path[4096] = "/";

    if (sscanf(url, "http://%255[^/]/%4095[^\n]", host, path + 1) != 2) {
        fprintf(stderr, "could not scan url %s", url); 
        return NULL;
    }
    
    return http_request(host, path);
}

static int http_read_headers(Response *response)
{
    int newline = 0; 
    int fd = response->fd;
    char *p = response->headers;
    char content_length[4096];

    while (read(fd, p, 1) > 0) {
        if (*p == '\n') {
            if (newline == 1) {
                break;
            }
            newline = 1;
        } else if (*p != '\r') {
            newline = 0;
        }
        p += 1;
    }

    if (sscanf(response->headers, "HTTP/1.0 %d", &response->status) != 1) {
        fprintf(stderr, "could not parse status\n");
        return -1;
    }

    http_header(response, "Content-Length", content_length);

    response->content_length += atoi(content_length);

    return 0;
}

Response *http_request(const char *host, const char *path)
{
#define BUFSIZE 4096

    char sendline[BUFSIZE];
  
    Response *response = malloc(sizeof(Response));
    response->status = 0;
    response->content_length = 0;
    response->headers = malloc(1 << 16);
    response->pos = 0;
    response->body = NULL;
    response->fd = http_connect(host);

    memset(response->headers, 0, 1 << 16);

    fprintf(stderr, "%s%s", host, path);

    if (response->fd < 0) {
        free_response(response);
        return NULL;
    }

    snprintf(sendline, BUFSIZE, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent:SoundCloud3000\r\n\r\n", path, host);

    write(response->fd, sendline, BUFSIZE);

    http_read_headers(response);

    return response;
}

void free_response(Response *response)
{
    free(response->headers);
    free(response->body);
    free(response);
}

        
int http_read_body(Response *response)
{
    int count = 0;

    response->body = malloc(response->content_length + 1);
    memset(response->body, 0, response->content_length + 1);

    char *p = response->body;

    while ((count = read(response->fd, p, 4096)) > 0) {
        response->pos += count;
        p += count;
    }

    close(response->fd);

    return count;
}
