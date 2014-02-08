#include <stdio.h>
#include <jansson.h>

#include "soundcloud3000.h"

#define BUFSIZE 4096

TrackList *api_recent_tracks(Api *api) {
    json_t *root;
    json_error_t error;
    char path[BUFSIZE];
    Track *track;

    int n = snprintf(path, BUFSIZE, "/tracks.json?client_id=%s", api->client_id);

    if (n < 0 || n > BUFSIZE) {
        fprintf(stderr, "url formatting failed: %d\n", n);
        return NULL;
    }

    Response *response = http_request(api->host, path);

    if (http_read(response) < 0) {
        fprintf(stderr, "http request failed\n");
        return NULL;
    }

    root = json_loads(response->body, 0, &error);

    free_response(response);

    if (root == NULL) {
        fprintf(stderr, "json parse error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    TrackList *list = malloc(sizeof(TrackList));
    list->count = json_array_size(root);
    list->tracks = malloc(sizeof(Track) * list->count);

    for (int i = 0; i < list->count; i++) {
        json_t *data = json_array_get(root, i);
        track = &list->tracks[i];

        track->id         = json_integer_value(json_object_get(data, "id"));
        track->duration   = json_integer_value(json_object_get(data, "duration"));
        track->title      = json_string_value(json_object_get(data, "title"));
        track->created_at = json_string_value(json_object_get(data, "created_at"));
        track->stream_url = json_string_value(json_object_get(data, "stream_url"));
    }

    return list;
}
