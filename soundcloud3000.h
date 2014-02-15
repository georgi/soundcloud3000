#include <pthread.h>
#include <portaudio.h>
#include <mpg123.h>

typedef struct api_config {
    const char *host;
    const char *client_id;
} api_config;

typedef struct track {
    int id;
    int duration;
    int playback_count;
    const char *title;
    const char *username;
    const char *genre;
    const char *created_at;
    const char *stream_url;
} track;

typedef struct track_list {
    track *tracks;
    int count;
} track_list;

typedef struct stream {
    mpg123_handle *mpg123;
    PaStream *pa_stream;
    char *url;
    pthread_t thread;
    int fd;
} stream;

track_list *api_recent_tracks(api_config *api);
track *api_get_track(api_config *api, int id);
track_list *api_user_tracks(api_config *api, const char *permalink);

void audio_init();

stream *stream_open(const char *url);
void stream_seek(stream *stream, long position);
void stream_close(stream *stream);
int stream_read(stream *stream, void *buffer, size_t buffer_size);
int stream_start(stream *stream);
int stream_stop(stream *stream);
int stream_is_active(stream *stream);
int stream_length(stream *stream);
