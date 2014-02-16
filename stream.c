#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
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

stream *stream_new()
{
    int err;
    stream *stream = malloc(sizeof(struct stream));
    memset(stream, 0, sizeof(struct stream));

    if ((stream->mpg123 = mpg123_new(NULL, &err)) == NULL) {
        fprintf(stderr, "%s", mpg123_plain_strerror(err));
        return NULL;
    }
    
    mpg123_param(stream->mpg123, MPG123_VERBOSE, 0, 0);
    mpg123_param(stream->mpg123, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.);

    return stream;
}

int stream_open(stream *stream, const char *url)
{
    fprintf(stderr, "stream_open: %s\n", url);
    stream->url = sdsnew(url);

    response *response = resolve_stream(url);

    if (response == NULL) {
        return -1;
    }

    if (stream->fd > 3 && fd_is_valid(stream->fd)) {
        close(stream->fd);
        mpg123_close(stream->mpg123);
    }
    
    stream->fd = response->fd;
    free_response(response);

    if (mpg123_open_fd(stream->mpg123, stream->fd) != MPG123_OK) {
        return -1;
    }
    
    return 0;
}

int stream_start(stream *stream)
{
    int err = Pa_OpenDefaultStream(&stream->pa_stream,
                                   0,           /* no input channels */
                                   2,           /* stereo output */
                                   paFloat32,   /* 32 bit floating point output */
                                   44100,       /* sample rate*/
                                   1 << 14,        /* frames_per_buffer */
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
    Pa_CloseStream(stream->pa_stream);

    mpg123_close(stream->mpg123);
    mpg123_delete(stream->mpg123);

    sdsfree(stream->url);
    close(stream->fd);

    free(stream);
}
