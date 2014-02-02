#include <stdio.h>
#include <math.h>
#include <string.h>

#include "soundcloud3000.h"

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

    pthread_mutex_init(&portaudio->mutex, NULL);
    pthread_cond_init(&portaudio->cond, NULL);

    portaudio->size = framesPerBuffer * 2;
    portaudio->buffer = (float *) malloc(sizeof(float) * portaudio->size);

    err = Pa_OpenDefaultStream(&portaudio->stream,
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

float rms(float *v, int n)
{
    int i;
    float sum = 0.0;

    for (i = 0; i < n; i++) {
        sum += v[i] * v[i];
    }

    return sqrt(sum / n);
}

int portaudio_write_from_mpg(Portaudio *portaudio, mpg123_handle *mh)
{
    int err;
    size_t done = 0;

    err = mpg123_read(mh, (unsigned char *) portaudio->buffer, portaudio->size * sizeof(float), &done);

    portaudio->rms = rms(portaudio->buffer, portaudio->size);

    return err;
}

int portaudio_start(Portaudio *portaudio)
{
    int err = Pa_StartStream(portaudio->stream);

    pthread_cond_broadcast(&portaudio->cond);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        return err;
    }

    return err;
}

int portaudio_stop(Portaudio *portaudio)
{
    int err = Pa_StopStream(portaudio->stream);

    pthread_cond_broadcast(&portaudio->cond);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
    }

    return err;
}

int portaudio_close(Portaudio *portaudio)
{
    int err = Pa_CloseStream(portaudio->stream);

    pthread_cond_broadcast(&portaudio->cond);

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
    }

    pthread_cond_destroy(&portaudio->cond);
    pthread_mutex_destroy(&portaudio->mutex);

    if (portaudio->buffer) {
        free(portaudio->buffer);
    }

    return err;
}
