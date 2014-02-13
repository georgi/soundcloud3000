#include <stdio.h>
#include <unistd.h>

#include "termbox/src/termbox.h"
#include "soundcloud3000.h"

void display_string(int fg, int bg, const char *p, int x, int y, int len)
{
    struct tb_cell cell;
    cell.bg = bg;
    cell.fg = fg;

    int max_x = x + len;

    for (; *p != 0; p++, x++) {
        tb_utf8_char_to_unicode(&cell.ch, p);
        tb_put_cell(x, y, &cell);
    }

    for (; x < max_x; x++) {
        tb_utf8_char_to_unicode(&cell.ch, " ");
        tb_put_cell(x, y, &cell);
    }
}

void display_track(Track *track, int y, int is_current)
{
    int bg = is_current ? TB_WHITE : TB_BLACK;
    display_string(TB_BLUE, bg, track->title, 0, y, 60);
    display_string(TB_CYAN, bg, track->username, 60, y, 20);
    display_string(TB_GREEN, bg, track->created_at, 80, y, 20);
}

int main(int argc, char *argv[]) {
    Api api;
    api.host = "api.soundcloud.com";
    api.client_id = "344564835576cb4df3cad0e34fa2fe0a";
    
    if (argc != 2) {
        printf("usage: soundcloud3000 <url>\n");
        exit(1);
    }

    audio_init();

    int err = tb_init();

    if (err) {
        fprintf(stderr, "tb_init() failed with error code %d\n", err);
        return 1;
    }

    struct tb_event ev;
    tb_select_input_mode(TB_INPUT_ESC);
    tb_clear();

    TrackList *list = api_user_tracks(&api, argv[1]);

    if (list == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
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

        while (1) {
            for (int y = 0; y < list->count; y++)
                display_track(&list->tracks[y], y, y == i);

            tb_present();
            
            if (tb_peek_event(&ev, 100) > 0) {
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

            if (!stream_is_active(stream)) {
                goto next_track;
            }
        }
    next_track:
        stream_stop(stream);
    }

 shutdown:

    tb_shutdown();

    return 0;
}
