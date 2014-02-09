#include <stdio.h>
#include <unistd.h>

#include "termbox/src/termbox.h"
#include "soundcloud3000.h"

int main(int argc, char *argv[]) {
    Api api;
    api.host = "api.soundcloud.com";
    api.client_id = "344564835576cb4df3cad0e34fa2fe0a";
    
    if (argc != 2) {
        printf("usage: soundcloud3000 <url>\n");
        exit(1);
    }

    TrackList *list = api_user_tracks(&api, argv[1]);

    if (list == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
    }

    mpg123_init();
    
    int err = Pa_Initialize();

    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }

    for (int i = 0; i < list->count; i++) {
        char url[4096];

        sprintf(url, "%s?client_id=%s", list->tracks[i].stream_url, api.client_id);

        Stream *stream = stream_open(url);

        if (stream == NULL) {
            fprintf(stderr, "stream_open failed\n");
            exit(1);
        }

        stream_start(stream);

        err = tb_init();

        if (err) {
            fprintf(stderr, "tb_init() failed with error code %d\n", err);
            return 1;
        }

        struct tb_event ev;
        tb_select_input_mode(TB_INPUT_ESC);
        tb_clear();

        while (1) {
            if (!stream_is_active(stream)) {
                goto next_track;
            }
            if (tb_peek_event(&ev, 10) > 0) {
                switch (ev.type) {
                case TB_EVENT_KEY:
                    switch (ev.key) {
                    case TB_KEY_ARROW_DOWN:
                        goto next_track;
                    case TB_KEY_ARROW_UP:
                        if (i > 0) {
                            i -= 2;
                            goto next_track;
                        }
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
        }
    next_track:
        stream_stop(stream);
    }

 shutdown:

    tb_shutdown();

    return 0;
}
