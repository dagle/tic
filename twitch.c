#include <json-c/json.h>
#include <string.h>

#include "twitch.h"
//#define GETSTR(x, y) (x = strdup(json_object_get_string(y)))
//#define GETSTR(x, y) (strncpy(x, safe_json_object_get_string(y), sizeof x))
#define GETSTR(x, y) (safe_json_object_get_string(x, sizeof x, y))

static void safe_json_object_get_string(char *str, size_t len, json_object *jobj) {
	if (json_object_get_type(jobj) == json_type_string ) {
		strncpy(str, json_object_get_string(jobj), len);
		return;
	}
	memset(str, 0, len);
}

static void parse_preview(preview *preview, json_object *jobj) {
	json_object_object_foreach(jobj, key, val) {
		if(!strcmp(key, "small")) {
			GETSTR(preview->small, val);
		}
		if(!strcmp(key, "medium")) {
			GETSTR(preview->medium, val);
		}
		if(!strcmp(key, "large")) {
			GETSTR(preview->large, val);
		}
		if(!strcmp(key, "template")) {
			GETSTR(preview->template, val);
		}
	}
}

//Go through these
static void parse_channel(channel *channel, json_object *jobj) {
	json_object_object_foreach(jobj, key, val) {
		if(!strncmp(key, "mature", 6))
			channel->mature = json_object_get_boolean(val);
		if(!strncmp(key, "abuse_reported", 14))
			channel->abuse_reported = json_object_get_boolean(val);
		if(!strncmp(key, "status", 6)) {
			GETSTR(channel->status, val);
		}
		if(!strncmp(key, "game", 6)) {
			GETSTR(channel->game, val);
		}
		if(!strncmp(key, "display_name", 12)) {
			GETSTR(channel->display_name, val);
		}
		if(!strncmp(key, "_id", 3))
			channel->id = json_object_get_int(val);
		if(!strncmp(key, "delay", 5))
			channel->delay = json_object_get_int(val);
		if(!strncmp(key, "views", 5))
			channel->delay = json_object_get_int(val);
		if(!strncmp(key, "followers", 9))
			channel->delay = json_object_get_int(val);
		if(!strncmp(key, "name", 4)) {
			GETSTR(channel->name, val);
		}
		if(!strncmp(key, "created", 7)) {
			GETSTR(channel->created, val);
		}
		if(!strncmp(key, "updated", 7)) {
			GETSTR(channel->updated, val);
		}
		if(!strncmp(key, "team_name", 9)) {
			GETSTR(channel->team_name, val);
		}
		if(!strncmp(key, "team_display_name", 17)) {
			GETSTR(channel->team_display_name, val);
		}
		if(!strncmp(key, "_channelLogo", 4)) {
			GETSTR(channel->logo, val);
		}
		if(!strncmp(key, "logo", 4)) {
			GETSTR(channel->logo, val);
		}
		if(!strncmp(key, "banner", 6)) {
			GETSTR(channel->banner, val);
		}
		if(!strncmp(key, "url", 3)) {
			GETSTR(channel->url, val);
		}
		//etc, need to check the real string values.
	}
}

static void parse_stream(twitch_entry *entry, json_object *jobj) {
	if(!jobj) {
		free(entry->stream);
		entry->stream = NULL;
		return;
	}
	if(!entry->stream)
		entry->stream = malloc(sizeof(stream));
	json_object_object_foreach(jobj, key, val) {
		if(!strncmp(key, "_id", 3))
			entry->stream->id = json_object_get_int(val);
		if(!strncmp(key, "game", 5)) {
			GETSTR(entry->stream->game, val);
		}
		if(!strncmp(key, "viewers", 7))
			entry->stream->viewers = json_object_get_int(val);
		if(!strncmp(key, "preview", 7))
			parse_preview(&entry->stream->preview, val);
		if(!strncmp(key, "channel", 7))
			parse_channel(&entry->stream->channel, val);
	}
}

static void parse_link(twitch_entry *entry, json_object *jobj) {
	json_object_object_foreach(jobj, key, val) {
		if(!strncmp(key, "channel", 7)) {
			GETSTR(entry->self, val);
		}
		if(!strncmp(key, "channel", 7)) {
			GETSTR(entry->channel, val);
		}
	}
}

void update_entry(twitch_entry *entry, char *str) {
	json_object *jobj = json_tokener_parse(str);

	json_object_object_foreach(jobj, key, val) {
		if(!strncmp(key, "_links", 6)) {
			parse_link(entry, val);
		} 
		else if(!strncmp(key, "stream", 6)) {
			parse_stream(entry, val);
		}
	}
	json_object_put(jobj);
}

