#include <stdio.h>
#include <unistd.h>

#include "soundcloud3000.h"
#include "sds/sds.h"

int main(int argc, char *argv[]) {
    api api;
    api.host = "api.soundcloud.com";
    api.client_id = "344564835576cb4df3cad0e34fa2fe0a";
    
    if (argc != 2) {
        printf("usage: soundcloud3000 <url>\n");
        exit(1);
    }

    audio_init();

    track_list *list = api_user_tracks(&api, argv[1]);

    if (list == NULL) {
        fprintf(stderr, "soundcloud api failed\n");
        return 1;
    }

    for (int i = 0; i < list->count; i++) {
        sds url = sdscatprintf(sdsempty(),  "%s?client_id=%s", list->tracks[i].stream_url, api.client_id);
        stream *stream = stream_open(url);

        if (stream == NULL) {
            fprintf(stderr, "stream_open failed\n");
            exit(1);
        }

        stream_start(stream);

        while (stream_is_active(stream)) {
            usleep(10000);
        }

        stream_stop(stream);
        sdsfree(url);
    }

    return 0;
}
