#include <pthread.h>
#include <portaudio.h>
#include <mpg123.h>

typedef struct Api {
    const char *host;
    const char *client_id;
} Api;

typedef struct Response {
    char *body;
    size_t size;
} Response;

typedef struct Track {
    int id;
    int duration;
    const char *title;
    const char *created_at;
    const char *stream_url;
} Track;

typedef struct TrackList {
    Track *tracks;
    int count;
} TrackList;

typedef struct {
    const char *url;
    unsigned char *body;
    size_t size;
    size_t position;
    pthread_t thread;
    mpg123_handle *mpg123;
} Stream;

typedef struct {
    PaStream *stream;
    int size;
    float *buffer;
    float rms;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Portaudio;

Response *http_request(const char *url);

TrackList *api_recent_tracks(Api *api);

Portaudio * portaudio_open_stream(int framesPerBuffer);

void portaudio_wait(Portaudio *portaudio);

int portaudio_write_from_stream(Portaudio *portaudio, Stream *stream);

int portaudio_start(Portaudio *portaudio);

int portaudio_stop(Portaudio *portaudio);

int portaudio_close(Portaudio *portaudio);

Stream *stream_open(const char *url);

void stream_close(Stream *stream);
