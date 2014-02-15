#include <stdio.h>
#include <string.h>
#include <jansson.h>

#include "hitpoint/hitpoint.h"
#include "http-parser/http_parser.h"
#include "sds/sds.h"

#include "soundcloud3000.h"

static void read_track(track *track, json_t *data)
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

static track_list *read_track_list(json_t *data)
{
    track_list *list = malloc(sizeof(track_list));
    memset(list, 0, sizeof(track_list));
    
    list->count = json_array_size(data);
    list->tracks = malloc(sizeof(track) * list->count);

    for (int i = 0; i < list->count; i++) {
        read_track(&list->tracks[i], json_array_get(data, i));
    }

    return list;
}

json_t *request_json(api_config *api, const char *path)
{
    json_error_t error;
    
    request *request = http_new_request(HTTP_GET);
    request->host = api->host;
    request->path = path;
    request->port = 80;
    
    response *response = http_send(request);

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

track_list *api_user_tracks(api_config *api, const char *permalink) {
    sds path = sdscatprintf(sdsempty(), "/users/%s/tracks.json?client_id=%s", permalink, api->client_id);
    
    track_list *list = read_track_list(request_json(api, path));

    sdsfree(path);

    return list;
}

track *api_get_track(api_config *api, int id) {
    track *track = malloc(sizeof(track));
    sds path = sdscatprintf(sdsempty(), "/tracks/%d.json?client_id=%s", id, api->client_id);
    
    json_t *root = request_json(api, path);

    read_track(track, root);

    sdsfree(path);
    
    return track;
}

track_list *api_recent_tracks(api_config *api) {
    sds path = sdscatprintf(sdsempty(), "/tracks.json?client_id=%s", api->client_id);
    
    track_list *list = read_track_list(request_json(api, path));

    sdsfree(path);

    return list;
}
