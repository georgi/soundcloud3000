#include <stdio.h>
#include <jansson.h>

#include "soundcloud3000.h"

static void read_track(Track *track, json_t *data)
{
        track->id             = json_integer_value(json_object_get(data, "id"));
        track->duration       = json_integer_value(json_object_get(data, "duration"));
        track->genre          = json_string_value(json_object_get(data, "genre"));
        track->username       = json_string_value(json_object_get(json_object_get(data, "user"), "username"));
        track->playback_count = json_integer_value(json_object_get(data, "playback_count"));
        track->title          = json_string_value(json_object_get(data, "title"));
        track->created_at     = json_string_value(json_object_get(data, "created_at"));
        track->stream_url     = json_string_value(json_object_get(data, "stream_url"));
}

static TrackList *read_track_list(json_t *data)
{
    TrackList *list = malloc(sizeof(TrackList));
    list->count = json_array_size(data);
    list->tracks = malloc(sizeof(Track) * list->count);

    for (int i = 0; i < list->count; i++) {
        read_track(&list->tracks[i], json_array_get(data, i));
    }

    return list;
}

json_t *request_json(Api *api, char *path)
{
    json_error_t error;
    Response *response = http_request(api->host, path);

    if (http_read_body(response) < 0) {
        fprintf(stderr, "http request failed\n");
        return NULL;
    }

    json_t *root = json_loads(response->body, 0, &error);

    free_response(response);

    if (root == NULL) {
        fprintf(stderr, "json parse error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    return root;
}

TrackList *api_user_tracks(Api *api, char *permalink) {
    char path[4096];

    sprintf(path, "/users/%s/tracks.json?client_id=%s", permalink, api->client_id);
    
    return read_track_list(request_json(api, path));
}

Track *api_get_track(Api *api, int id) {
    char path[4096];
    Track *track = malloc(sizeof(Track));

    sprintf(path, "/tracks/%d.json?client_id=%s", id, api->client_id);
    
    json_t *root = request_json(api, path);

    read_track(track, root);
    
    return track;
}

TrackList *api_recent_tracks(Api *api) {
    char path[4096];

    sprintf(path, "/tracks.json?client_id=%s", api->client_id);
    
    return read_track_list(request_json(api, path));
}
