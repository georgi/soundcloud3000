#include <stdio.h>
#include <unistd.h>

#include "soundcloud3000.h"

int main() {
    Api api;
    api.host = "api.soundcloud.com";
    api.client_id = "344564835576cb4df3cad0e34fa2fe0a";

    TrackList *tracks = api_recent_tracks(&api);

    if (tracks == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
    }
  
    mpg123_init();

    int err = Pa_Initialize();
    
    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }

    Portaudio *portaudio = portaudio_open_stream(4096);

    if (portaudio == NULL) {
        fprintf(stderr, "portaudio_open_stream failed\n");
        exit(1);
    }
    
    if (portaudio_start(portaudio)) {
        fprintf(stderr, "portaudio_start failed\n");
        exit(1);
    }

    char url[4096];

    sprintf(url, "%s?client_id=%s", tracks->tracks[0].stream_url, api.client_id);

    Stream *stream = stream_open(url);

    if (stream == NULL) {
        fprintf(stderr, "stream_open failed\n");
        exit(1);
    }

    usleep(1000000);
    
    while (1) {
        int err = portaudio_write_from_stream(portaudio, stream);

        if (err != MPG123_OK && err != MPG123_NEED_MORE && err != MPG123_NEW_FORMAT) {
            fprintf(stderr, "%s", mpg123_plain_strerror(err));
            break;
        }

        portaudio_wait(portaudio);
    }

    portaudio_close(portaudio);
    stream_close(stream);

    return 0;
}
