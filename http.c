#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "soundcloud3000.h"

static size_t on_response(void *contents, size_t size, size_t nresponseb, void *userp)
{
    size_t realsize = size * nresponseb;
    Response *response = (Response *)userp;

    response->body = realloc(response->body, response->size + realsize + 1);

    if (response->body == NULL) {
        printf("not enough memory");
        return 0;
    }
 
    memcpy(&(response->body[response->size]), contents, realsize);

    response->size += realsize;
    response->body[response->size] = 0;
 
    return realsize;
}

Response *http_request(const char *url)
{
    CURL *curl;
    CURLcode res;
    Response *response = malloc(sizeof(Response));
    response->body = NULL;
    response->size = 0;
    
    curl = curl_easy_init();
  
    if (curl == NULL) {
        fprintf(stderr, "could not init curl");
        return NULL;
    }
        
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    fprintf(stderr, "requesting %s\n", url);
 
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "%s\n", curl_easy_strerror(res));
        return NULL;
    }
 
    curl_easy_cleanup(curl);

    return response;
}
