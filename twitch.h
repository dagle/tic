typedef struct twitch_entry twitch_entry;
typedef struct stream stream;
typedef struct preview preview;
typedef struct channel channel;

struct preview {
	char small[128];
	char medium[128];
	char large[128];
	char template[128];
};

/*	A lot of this we don't need, a lot is not needed 
	to be so large, but lets just do it like this and worry about it later
	this is the version that is easiest to change */
struct channel {
	int mature;
	int abuse_reported;
	char status[64];
	char game[64];
	char display_name[64];
	int delay;
	int id;
	char name[64];
	char created[64];
	char updated[64];
	char team_name[64];
	char team_display_name[64];
	char logo[128];
	char banner[128];
	char video_banner[128];
	char background[128];
	char profile_banner[128];
	char profile_banner_bg[128];
	char url[128];
	int views;
	int followers;
	// Links links;
};

struct stream {
	int id;
	char game[64];
	int viewers;
	//char *self;
	preview preview;
	channel channel;
};

struct twitch_entry {
	char name[128];
	char self[128];
	char channel[128];
	stream *stream;
};

void update_entry(twitch_entry *entry, char *str);
