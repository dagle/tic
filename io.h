// most import twitch.h and curl to use this
typedef struct twitch_list twitch_list;
typedef struct settings settings;

struct twitch_list {
	twitch_entry *entry;
	twitch_list *next;
	twitch_list *prev;
};

struct curl_result {
	char *data;
	int pos;
};

int fectch(CURL *chandle, twitch_entry *entry);
void download(CURL *chandle, const char *file, const char *url);
int update_all(CURL *chandle, twitch_list *list);
char *get_path(char *str);

twitch_list *list_new(char *name);
twitch_list *list_push(twitch_list *list, char *name);
twitch_list *list_add(twitch_list *list, char *name);
int length(twitch_list *list);
int list_online(twitch_list *list);

char *mkstr(const char *fmt, ...);
