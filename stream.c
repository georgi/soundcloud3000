#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "hitpoint/hitpoint.h"
#include "sds/sds.h"
#include "soundcloud3000.h"

void audio_init()
{
    mpg123_init();
    
    int err = Pa_Initialize();

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }
}

static int portaudio_callback(const void *input_buffer,
                              void *output_buffer,
                              unsigned long frames_per_buffer,
                              const PaStreamCallbackTimeInfo* timeinfo,
                              PaStreamCallbackFlags status_flags,
                              void *user_data )
{
    stream *stream = user_data;
    stream_read(stream, output_buffer, sizeof(float) * frames_per_buffer * 2);
    return 0;
}

static response *resolve_stream(const char *url)
{
    request *request = http_get(url);
    response *response = http_send(request);

    if (http_read_body(response) < 0 && response->status != 302) {
        fprintf(stderr, "request failed %s", url); 
        return NULL;
    }

    sds stream_url = sdsnew(http_header(response, "Location"));

    free_response(response);

    request = http_get(stream_url);
    response = http_send(request);
    
    sdsfree(stream_url);

    return response;
}

stream *stream_open(const char *url)
{
    int err;
    stream *stream = malloc(sizeof(stream));

    stream->url = url;

    fprintf(stderr, "stream_open: %s\n", url);

    if ((stream->mpg123 = mpg123_new(NULL, &err)) == NULL) {
        fprintf(stderr, "%s", mpg123_plain_strerror(err));
        return NULL;
    }
    
    mpg123_param(stream->mpg123, MPG123_VERBOSE, 0, 0);
    mpg123_param(stream->mpg123, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.);

    response *response = resolve_stream(url);

    mpg123_open_fd(stream->mpg123, response->fd);

    free_response(response);
    
    return stream;
}

int stream_start(stream *stream)
{
    int err = Pa_OpenDefaultStream(&stream->pa_stream,
                                   0,           /* no input channels */
                                   2,           /* stereo output */
                                   paFloat32,   /* 32 bit floating point output */
                                   44100,       /* sample rate*/
                                   512,         /* frames_per_buffer */
                                   portaudio_callback,
                                   (void*) stream);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        return err;
    }

    err = Pa_StartStream(stream->pa_stream);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        return err;
    }

    return 0;
}

int stream_read(stream *stream, void *buffer, size_t buffer_size)
{
    size_t done;
    int err = 0;

    err = mpg123_read(stream->mpg123, buffer, buffer_size, &done);

    if (err == MPG123_DONE) {
        Pa_StopStream(stream->pa_stream);
    }

    if (err != MPG123_OK) {
        memset(buffer, 0, buffer_size);
    }

    return err;
}

int stream_length(stream *stream)
{
    return mpg123_length(stream->mpg123);
}


int stream_is_active(stream *stream)
{
    return Pa_IsStreamActive(stream->pa_stream);
}

int stream_stop(stream *stream)
{
    return Pa_StopStream(stream->pa_stream);
}

void stream_close(stream *stream)
{
    mpg123_close(stream->mpg123);
    mpg123_delete(stream->mpg123);
    Pa_CloseStream(stream->pa_stream);

    pthread_cancel(stream->thread);

    free(stream);
}
