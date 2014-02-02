#include <pthread.h>
#include <portaudio.h>
#include <mpg123.h>

typedef struct {
    PaStream *stream;
    int size;
    float *buffer;
    float rms;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Portaudio;

Portaudio * portaudio_open_stream(int framesPerBuffer);

void portaudio_wait(Portaudio *portaudio);

int portaudio_write_from_mpg(Portaudio *portaudio, mpg123_handle *mh);

int portaudio_start(Portaudio *portaudio);

int portaudio_stop(Portaudio *portaudio);

int portaudio_close(Portaudio *portaudio);

mpg123_handle *mpg_open_file(const char *filename);
