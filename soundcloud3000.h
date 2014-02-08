#include <pthread.h>
#include <portaudio.h>
#include <mpg123.h>

typedef struct Api {
    const char *host;
    const char *client_id;
} Api;

typedef struct Response {
    int fd;
    int status;
    char *headers;
    char *body;
    size_t pos;
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

typedef struct Stream {
    const char *url;
    Response *response;
    size_t position;
    pthread_t thread;
    mpg123_handle *mpg123;
} Stream;

typedef struct Portaudio {
    PaStream *pa_stream;
    Stream *stream;
    size_t size;
    void *buffer;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Portaudio;

Response *http_request(const char *host, const char *path);
Response *http_request_url(const char *url);
int http_header(Response *response, const char *key, char *value);
int http_read(Response *response);
void free_response(Response *response);

TrackList *api_recent_tracks(Api *api);

Portaudio * portaudio_open_stream(int framesPerBuffer);
void portaudio_wait(Portaudio *portaudio);
int portaudio_start(Portaudio *portaudio);
int portaudio_stop(Portaudio *portaudio);
int portaudio_close(Portaudio *portaudio);

Stream *stream_open(const char *url);
void stream_seek(Stream *stream, long position);
void stream_close(Stream *stream);
