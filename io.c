#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "twitch.h"
#include "io.h"

#define BUFFER_SIZE  (256 * 4096)
#define MAX(x, y) ((x) > (y) ? (x) : (y))

size_t curl_write(void *buf, size_t size, size_t nmemb, void *data);

static struct curl_slist *headers = NULL;

// returns 0 if the stream haven't changed, 1 if it had and -1 on error.
int fetch(CURL *chandle, twitch_entry *entry) {
	static char buf[256];
	char *data;
	int status, code;

	// seem a bit dumb, we shouldn't reallocate etc.
	data = malloc(BUFFER_SIZE);

	struct curl_result result = {
		.data = data,
		.pos = 0
	};

	snprintf(buf, sizeof buf, "%s%s", "https://api.twitch.tv/kraken/streams/", entry->name);

	curl_easy_setopt(chandle, CURLOPT_URL, buf);
	if(!headers)
		headers = curl_slist_append(headers, "User-Agent: Tic");
	curl_easy_setopt(chandle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(chandle, CURLOPT_WRITEFUNCTION, curl_write);
	curl_easy_setopt(chandle, CURLOPT_WRITEDATA, &result);
	status = curl_easy_perform(chandle);

	if(status != 0) {
		return -1;
	}

	curl_easy_getinfo(chandle, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200) {
		return -1;
	}
	data[result.pos] = '\0';
	update_entry(entry, data);
	free(data);
	return 0;
}

size_t curl_write(void *buf, size_t size, size_t nmemb, void *data) {

	struct curl_result *result = (struct curl_result*)data;

	if(result->pos + size * nmemb >= BUFFER_SIZE - 1) {
		fprintf(stderr, "error: too small buffer\n");
		return 0;
	}

	memcpy(result->data + result->pos, buf, size * nmemb);
	result->pos += size * nmemb;

	return size * nmemb;
}

// we should read this into memory until we have a full download
// this should be done in the background
void download(CURL *chandle, const char *file, const char *url) {
	FILE *fp;
	int status;
	if (!strlen(url))
		return;
	
	fp = fopen(file, "w");
	if(!headers)
		headers = curl_slist_append(headers, "User-Agent: Tic");
	curl_easy_setopt(chandle, CURLOPT_URL, url);
	curl_easy_setopt(chandle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(chandle, CURLOPT_WRITEFUNCTION, fwrite);
	curl_easy_setopt(chandle, CURLOPT_WRITEDATA, fp);
	status = curl_easy_perform(chandle);
	fclose(fp);
}

char *mkstr(const char *fmt, ...) {
	va_list ap;
	char *str;
	int i;
	va_start(ap, fmt);
	i = vasprintf(&str, fmt, ap);
	va_end(ap);
	if(i == -1)
		return NULL;
	return str;
}

//placeholder, a real solution should come 
#define LINUX
#ifdef LINUX
#include <basedir_fs.h>

char *get_config(void) {
}

// this should be safe, should fail all times but the first
void create_dir(xdgHandle *handle) {
	char *str;
	str = mkstr("%s/%s", xdgCacheHome(handle), "tic");
	xdgMakePath(str, 0777);
	free(str);
}

static xdgHandle *handle = NULL;
char *get_path(char *path) {
	static int initData = 0;

	if(!initData) {
		xdgInitHandle(handle);
		create_dir(handle);
	}

	return mkstr("%s/%s", xdgCacheHome(handle), path);
}
#endif

int update_all(CURL *chandle, twitch_list *list) {
	int update = 0;
	for(; list; list = list->next) {
		update = MAX(fetch(chandle, list->entry), update);
	}
	return update;
}

int length(twitch_list *list) {
	int i = 1;
	for(; list->next; list = list->next, i++);
	return i;
}

int list_online(twitch_list *list) {
	int i = 0;
	for(; list->next; list = list->next) {
		if(list->entry->stream)
			i++;
	}
	return i;
}

twitch_list *list_new(char *name) {
	twitch_list *list;
	list = malloc(sizeof(twitch_list));
	memset(list, 0, sizeof(twitch_list));
	list->entry = malloc(sizeof(twitch_entry));
	if (list->entry) {
		memset(list->entry, 0, sizeof(twitch_entry));
		strcpy(list->entry->name, name);
	}
	return list;
}

twitch_list *list_push(twitch_list *list, char *name) {
	twitch_list *new;
	new = list_new(name);
	new->next = list;
	list->prev = new;
	return new;
}

twitch_list *list_add(twitch_list *list, char *name) {
	twitch_list *new;
	for(; list->next; list = list->next);
	new = list_new(name);
	new->prev = list;
	list->next = new;
	return new;
}
