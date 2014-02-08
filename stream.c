#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "soundcloud3000.h"

static void *run_thread(void *ptr)
{
    Stream *stream = ptr;
    char stream_url[4096];
    
    Response *response = http_request_url(stream->url);

    if (http_read(response) < 0 && response->status != 302) {
        fprintf(stderr, "request failed %s", stream->url); 
        return NULL;
    }

    if (http_header(response, "Location", stream_url) < 0) {
        fprintf(stderr, "expected redirect %s", stream->url); 
        return NULL;
    }

    stream->response = http_request_url(stream_url);

    http_read(stream->response);
    
    return NULL;
}

Stream *stream_open(const char *url)
{
    int err;
    Stream *stream = malloc(sizeof(Stream));

    stream->position = 0;
    stream->url = url;
    stream->response = NULL;

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

void stream_seek(Stream *stream, long position)
{
    if (stream->response != NULL) {
        stream->position = position < 0 ? 0 : (position > (long) stream->response->pos ? stream->response->pos : position);
    }
}

void stream_close(Stream *stream)
{
    free_response(stream->response);
    
    mpg123_close(stream->mpg123);
    mpg123_delete(stream->mpg123);

    pthread_cancel(stream->thread);

    free(stream);
}
