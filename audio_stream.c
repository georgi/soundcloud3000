#include <pthread.h>
#include <string.h>
#include <curl/curl.h>

#include "soundcloud3000.h"

static size_t on_response(void *contents, size_t size, size_t nresponseb, void *userp)
{
    size_t realsize = size * nresponseb;
    Stream *stream = (Stream *)userp;
 
    stream->body = realloc(stream->body, stream->size + realsize + 1);

    if (stream->body == NULL) {
        printf("not enough memory");
        return 0;
    }
 
    memcpy(&(stream->body[stream->size]), contents, realsize);

    stream->size += realsize;

    return realsize;
}

static void *run_thread(void *ptr)
{
    CURL *curl;
    Stream *stream = ptr;
 
    curl = curl_easy_init();

    if (curl == NULL) {
        fprintf(stderr, "could not init curl");
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, stream->url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "%s\n", curl_easy_strerror(res));
        return NULL;
    }
 
    curl_easy_cleanup(curl);
 
    return NULL;
}

Stream *stream_open(const char *url)
{
    int err;
    Stream *stream = malloc(sizeof(Stream));

    stream->position = 0;
    stream->url = url;
    stream->body = NULL;
    stream->size = 0;

    fprintf(stderr, "stream_open: %s\n", url);

    if ((stream->mpg123 = mpg123_new(NULL, &err)) == NULL) {
        fprintf(stderr, "%s", mpg123_plain_strerror(err));
        return NULL;
    }
    
    mpg123_param(stream->mpg123, MPG123_VERBOSE, 2, 0);
    mpg123_param(stream->mpg123, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.);
    mpg123_open_feed(stream->mpg123);

    pthread_create(&stream->thread, NULL, run_thread, (void *) stream);
    
    return stream;
}

void stream_close(Stream *stream)
{
    mpg123_close(stream->mpg123);
    mpg123_delete(stream->mpg123);
}
