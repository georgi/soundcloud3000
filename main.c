#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "soundcloud3000.h"
#include "sds/sds.h"
#include "hitman/hitman.h"

static api_config api;
static stream *current_stream = NULL;

void write_error(request *request, char *body) {
    write_status(request, 500, "INTERNAL SERVER ERROR");
    write_header(request, "Content-Type", "text/html");
    write_body(request, body);
}

void handle_play(request *request) {
    int id;

    if (sscanf(request->path, "/play/%d", &id) < 1) {
        write_error(request, "invalid url");
        return;
    }

    track *track = api_get_track(&api, id);

    sds url = sdscatprintf(sdsempty(),  "%s?client_id=%s", track->stream_url, api.client_id);

    stream_stop(current_stream);
    stream_close(current_stream);

    current_stream = stream_new();

    if (stream_open(current_stream, url) != 0) {
        write_error(request, "stream open failed");
        return;
    }

    if (stream_start(current_stream) != 0) {
        write_error(request, "stream start failed");
        return;
    }

    write_status(request, 200, "OK");
    write_header(request, "Content-Type", "text/html");
    write_body(request, "OK");
}

void handle_user(request *request) {
    char permalink[512];
    sds body = sdsempty();

    if (sscanf(request->path, "/user/%512s", permalink) < 1) {
        write_error(request, "invalid url");
        return;
    }

    track_list *list = api_user_tracks(&api, permalink);

    if (list == NULL) {
        write_error(request, "soundcloud api failed");
        return;
    }

    body = sdscatprintf(body, "<h1>%s</h1>", permalink);

    for (int i = 0; i < list->count; i++) {
        body = sdscatprintf(body, "<a href='/play/%d'>%s</li>", list->tracks[i].id, list->tracks[i].title);
    }

    write_status(request, 200, "OK");
    write_header(request, "Content-Type", "text/html");
    write_body(request, body);
}

int main(int argc, char *argv[]) {
    api.host = "api.soundcloud.com";

    if (argc != 2 || strlen(argv[1]) >= CLIENT_ID_LENGTH) {
      fprintf(stderr, "Usage: %s YOUR_CLIENT_ID\n", argv[0]);
      exit(1);
    } else {
      strncpy(api.client_id, argv[1], CLIENT_ID_LENGTH);
    }

    audio_init();

    current_stream = stream_new();

    server server;
    server.handlers = NULL;
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(3000);

    add_handler(&server, "/user", handle_user);
    add_handler(&server, "/play", handle_play);

    http_serve(&server);

    return 0;
}
