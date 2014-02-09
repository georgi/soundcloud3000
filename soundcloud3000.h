#include <pthread.h>
#include <portaudio.h>
#include <mpg123.h>

typedef struct Api {
    const char *host;
    const char *client_id;
} Api;

typedef struct Header {
    char *name;
    char *value;
} Header;

typedef struct Response {
    int fd;
    int status;
    char *headers;
    char *body;
    size_t pos;
    size_t content_length;
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
    mpg123_handle *mpg123;
    PaStream *pa_stream;
    const char *url;
    pthread_t thread;
} Stream;

Response *http_request(const char *host, const char *path);
Response *http_request_url(const char *url);
int http_header(Response *response, const char *key, char *value);
int http_read_body(Response *response);
void free_response(Response *response);

TrackList *api_recent_tracks(Api *api);
Track *api_get_track(Api *api, int id);
TrackList *api_user_tracks(Api *api, char *permalink);

PaStream *portaudio_open_stream(int frames_per_bufffer, Stream *stream);

Stream *stream_open(const char *url);
void stream_seek(Stream *stream, long position);
void stream_close(Stream *stream);
int stream_read(Stream *stream, void *buffer, size_t buffer_size);
int stream_start(Stream *stream);
int stream_stop(Stream *stream);
int stream_is_active(Stream *stream);
