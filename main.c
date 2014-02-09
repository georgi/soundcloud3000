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

    Track *track = api_get_track(&api, 131539027);

    if (track == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
    }

    mpg123_init();
    
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }

    char url[4096];

    sprintf(url, "%s?client_id=%s", track->stream_url, api.client_id);

    Stream *stream = stream_open(url);

    if (stream == NULL) {
        fprintf(stderr, "stream_open failed\n");
        exit(1);
    }

    err = tb_init();

    if (err) {
        fprintf(stderr, "tb_init() failed with error code %d\n", err);
        return 1;
    }

    tb_select_input_mode(TB_INPUT_ESC);
    tb_clear();

    while (tb_poll_event(&ev)) {
        switch (ev.type) {
        case TB_EVENT_KEY:
            switch (ev.key) {
            case TB_KEY_ARROW_RIGHT:
                mpg123_seek(stream->mpg123, 44100 * 1, SEEK_CUR);
                break;
            case TB_KEY_ARROW_LEFT:
                mpg123_seek(stream->mpg123, -44100 * 1, SEEK_CUR);
                break;
            case TB_KEY_CTRL_C:
                goto shutdown;
            }
        }
    }

 shutdown:

    tb_shutdown();
    stream_close(stream);

    return 0;
}
