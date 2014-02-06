#include <stdio.h>
#include <math.h>
#include <string.h>

#include "soundcloud3000.h"

int portaudio_write_from_stream(Portaudio *portaudio)
{
    int err = 0;
    size_t done = 0;
    unsigned char *buffer = (unsigned char*) portaudio->buffer;
    size_t buffer_size = portaudio->size * sizeof(float);

    memset(portaudio->buffer, 0, buffer_size);

    if (portaudio->stream != NULL) {
        err = mpg123_read(portaudio->stream->mpg123, buffer, buffer_size, &done);

        if (err == MPG123_NEED_MORE) {
            if (portaudio->stream->size - portaudio->stream->position > 4096) {
                err = mpg123_decode(portaudio->stream->mpg123,
                                    portaudio->stream->body + portaudio->stream->position,
                                    portaudio->stream->size - portaudio->stream->position,
                                    buffer,
                                    buffer_size,
                                    &done);
            }
        }

        portaudio->stream->position += done;
    }

    return err;
}

static void *run_thread(void *ptr)
{
    Portaudio *portaudio = ptr;
    
    while (1) {
        int err = portaudio_write_from_stream(portaudio);

        if (err != MPG123_OK && err != MPG123_NEED_MORE && err != MPG123_NEW_FORMAT) {
            fprintf(stderr, "%s", mpg123_plain_strerror(err));
            break;
        }

        portaudio_wait(portaudio);
    }

    return NULL;
}

static int paCallback(const void *inputBuffer,
                      void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData )
{
    Portaudio *portaudio = (Portaudio *) userData;
    float *out = (float*) outputBuffer;

    pthread_mutex_lock(&portaudio->mutex);

    memcpy(out, portaudio->buffer, sizeof(float) * portaudio->size);

    pthread_cond_broadcast(&portaudio->cond);
    pthread_mutex_unlock(&portaudio->mutex);

    return 0;
}

Portaudio *portaudio_open_stream(int framesPerBuffer)
{
    PaError err;
    Portaudio *portaudio = (Portaudio *) malloc(sizeof(Portaudio));

    portaudio->stream = NULL;
    portaudio->thread = NULL;
    pthread_mutex_init(&portaudio->mutex, NULL);
    pthread_cond_init(&portaudio->cond, NULL);

    portaudio->size = framesPerBuffer * 2;
    portaudio->buffer = (float *) malloc(sizeof(float) * portaudio->size);
    memset(portaudio->buffer, 0, sizeof(float) * portaudio->size);

    err = Pa_OpenDefaultStream(&portaudio->pa_stream,
                               0,           /* no input channels */
                               2,           /* stereo output */
                               paFloat32,   /* 32 bit floating point output */
                               44100,       /* sample rate*/
                               framesPerBuffer,
                               paCallback,
                               (void*) portaudio);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        return NULL;
    }

    return portaudio;
}


void portaudio_wait(Portaudio *portaudio)
{
    pthread_cond_wait(&portaudio->cond, &portaudio->mutex);
}

int portaudio_start(Portaudio *portaudio)
{
    int err = Pa_StartStream(portaudio->pa_stream);

    pthread_create(&portaudio->thread, NULL, run_thread, (void *) portaudio);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        return err;
    }

    return err;
}

int portaudio_stop(Portaudio *portaudio)
{
    int err = Pa_StopStream(portaudio->pa_stream);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
    }

    return err;
}

int portaudio_close(Portaudio *portaudio)
{
    int err = Pa_CloseStream(portaudio->pa_stream);

    pthread_cond_broadcast(&portaudio->cond);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
    }

    pthread_cond_destroy(&portaudio->cond);
    pthread_mutex_destroy(&portaudio->mutex);
    pthread_kill(portaudio->thread, 9);

    if (portaudio->buffer) {
        free(portaudio->buffer);
    }

    return err;
}
