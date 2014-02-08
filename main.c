#include <stdio.h>
#include <unistd.h>

#include "termbox/src/termbox.h"
#include "soundcloud3000.h"

int main() {
    int err;
    Api api;
    struct tb_event ev;

    api.host = "api.soundcloud.com";
    api.client_id = "344564835576cb4df3cad0e34fa2fe0a";

    TrackList *tracks = api_recent_tracks(&api);

    if (tracks == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
    }

    mpg123_init();
    
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }

    Portaudio *portaudio = portaudio_open_stream(1 << 15);

    if (portaudio == NULL) {
        fprintf(stderr, "portaudio_open_stream failed\n");
        exit(1);
    }
    
    if (portaudio_start(portaudio)) {
        fprintf(stderr, "portaudio_start failed\n");
        exit(1);
    }

    char url[4096];

    for (int i = 0; i < 10; i++) {
        if (tracks->tracks[i].stream_url != NULL) {
            sprintf(url, "%s?client_id=%s", tracks->tracks[i].stream_url, api.client_id);
        }
    }

    portaudio->stream = stream_open(url);

    if (portaudio->stream == NULL) {
        fprintf(stderr, "stream_open failed\n");
        exit(1);
    }

    while (1) usleep(1000);

    /* err = tb_init(); */

    /* if (err) { */
    /*     fprintf(stderr, "tb_init() failed with error code %d\n", err); */
    /*     return 1; */
    /* } */

    /* tb_select_input_mode(TB_INPUT_ESC); */
    /* tb_clear(); */

    /* while (tb_poll_event(&ev)) { */
    /*     switch (ev.type) { */
    /*     case TB_EVENT_KEY: */
    /*         if (ev.key == TB_KEY_CTRL_C) { */
    /*             goto shutdown; */
    /*         } */
    /*     } */
    /* } */

 shutdown:

    tb_shutdown();
    portaudio_close(portaudio);
    stream_close(portaudio->stream);

    return 0;
}
