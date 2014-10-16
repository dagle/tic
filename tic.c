#include <curl/curl.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "twitch.h"
#include "io.h"

struct settings {
	char *font;
	twitch_list *list;
};

#define BUFFER_SIZE  (256 * 4096)
#define APP_ICON (TICPREFIX "twitch-icon.png")
#define APP_ICON_SMALL (TICPREFIX "twitch-icon-small.png")
#define FALLBACK (TICPREFIX "fallback.png")

CURL *chandle;
GtkWidget *menu;
GtkStatusIcon *status_icon;
GtkWidget *popup = NULL;
static gboolean updated = 1;
twitch_list *tlist;

static GtkWidget* popup_menu_new(void);

char *fetch(twitch_entry *);
void announce(twitch_entry *);

//callbacks
size_t curl_write(void *buf, size_t size, size_t nmemb, void *userp); 
static void menu_pref(GtkMenuItem *item, void *data);
static void menu_about(GtkMenuItem *item, void *data);
static void menu_quit(GtkMenuItem *item, void *data);
static void popup_menu(GtkStatusIcon *status_icon, GdkEventButton *event, void *data);

// do I only need to free the image?
// should do a callback etc
GtkWidget *get_image(twitch_entry *entry) {
	GError *err = NULL;
	GdkPixbuf *buf, *scaled;
	char *file;
	char *path;

	path = mkstr("%s/%s.png", "tic", entry->name);
	file = get_path(path);
	free(path);

	buf = gdk_pixbuf_new_from_file(file, &err);
	if(err != NULL) {
		g_error_free(err);
		err = NULL;
		if(strlen(entry->stream->channel.logo)) {
			download(chandle, file, entry->stream->channel.logo);
			buf = gdk_pixbuf_new_from_file(file, &err);
			g_error_free(err);
		} else {
			buf = gdk_pixbuf_new_from_file(FALLBACK, &err);
		}
	}
	free(file);
	scaled = gdk_pixbuf_scale_simple(buf, 40, 40, GDK_INTERP_BILINEAR);
	return gtk_image_new_from_pixbuf(scaled);
}

// ugly hack
#define TRUNK(x) ((x)[22] = '\0')
void show_entry(GtkWidget *vbox, twitch_entry *entry) {
	GtkWidget *entry_paned;
	GtkWidget *entry_box;
	GtkWidget *entry_name;
	GtkWidget *entry_title;
	GtkWidget *image;
	TRUNK(entry->stream->channel.name);
	TRUNK(entry->stream->channel.status);

	entry_paned = gtk_paned_new(0);
	entry_box = gtk_box_new(1, 0);
	entry_name = gtk_label_new(entry->stream->channel.name);
	entry_title = gtk_label_new(entry->stream->channel.status);
	image = get_image(entry);

	gtk_paned_add1(GTK_PANED(entry_paned), GTK_WIDGET(image));
	gtk_paned_add2(GTK_PANED(entry_paned), GTK_WIDGET(entry_box));
	gtk_container_add(GTK_CONTAINER(entry_box), entry_name);
	gtk_container_add(GTK_CONTAINER(entry_box), entry_title);
	//gtk_container_add(GTK_CONTAINER(vbox), entry_paned);
	gtk_box_pack_start(GTK_BOX(vbox), entry_paned, TRUE, TRUE, 0);
}

GtkWidget *new_popup_window(int items) {
	GtkWidget *popup;
	GdkRGBA rgba;
	gdk_rgba_parse(&rgba, "#FFFFFF");
	popup = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_container_set_border_width(GTK_CONTAINER(popup), 10);
	// default size should 1, either 1 streamer or displaying "no streams".
	gtk_window_set_default_size(GTK_WINDOW(popup), 220, 65*items);
	//gtk_window_set_resizable(GTK_WINDOW(popup), FALSE);
	gtk_window_set_position(GTK_WINDOW(popup), GTK_WIN_POS_CENTER);
	gtk_widget_override_background_color(GTK_WIDGET(popup), GTK_STATE_FLAG_NORMAL, &rgba);
	return popup;
}

// lets leave it like this for the moment
void window_populate(GtkContainer *container, int reverse) {
	twitch_list *tmp;
	GtkWidget *vbox;
	GtkWidget *scrolled;

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	//gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

	vbox = gtk_box_new(TRUE, 1);
	if(reverse) {
		for(tmp = tlist; tmp != NULL; tmp = tmp->next);
		for(; tmp != NULL; tmp = tmp->prev) {
			if(tmp->entry->stream) {
				show_entry(vbox, tmp->entry);
			}
		}
	} else {
		for(tmp = tlist; tmp != NULL; tmp = tmp->next) {
			if(tmp->entry->stream) {
				show_entry(vbox, tmp->entry);
			}
		}
	}
	gtk_container_add(GTK_CONTAINER(container), scrolled);
	gtk_container_add(GTK_CONTAINER(scrolled), vbox);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(gtk_bin_get_child(GTK_BIN((scrolled)))), 
											GTK_SHADOW_NONE);
}

void announce(twitch_entry *entry) {
}

int hide_popup(void *data) {
	GtkWidget *widget = (GtkWidget *)data;
	gtk_widget_destroy(GTK_WIDGET(widget));
	return TRUE;
}

void create_notification(int items, int native) {
	GtkWidget *timed;
	int id;
	timed = new_popup_window(items);
	window_move_absolute(popup, 0, 0);
	window_populate(GTK_CONTAINER(timed), 0);

	gtk_window_present(GTK_WINDOW(timed));
	gtk_widget_show_all(GTK_WIDGET(timed));
	id = g_timeout_add(1500, hide_popup, timed);
}

void window_move(GtkWidget *win, GtkStatusIcon *status_icon, GdkEventButton *event) {
	int x, y;
	int wx, wy;
	gtk_window_get_size(GTK_WINDOW(popup), &wx, &wy);
	
	x = event->x_root - wx / 2;
	y = event->y_root - wy;

	if(gtk_status_icon_is_embedded(status_icon)) {
		GdkRectangle area;
		gtk_status_icon_get_geometry(status_icon, NULL, &area, NULL);
		x = area.x + area.width / 2 - wx + 10;
		if(area.y > wy) // popup up
			y = area.y - wy;
		else // popup down
			y = area.y + area.height;

	}
	gtk_window_move(GTK_WINDOW(win), x ,y);
}

void window_move_absolute(GtkWidget *win, int top, int left) {
	int x, y;
	int wx, wy;
	int posx, posy;
	gtk_window_get_size(GTK_WINDOW(popup), &wx, &wy);
	GdkScreen * screen = gdk_screen_get_default();
	x = gdk_screen_get_width(screen);
	y = gdk_screen_get_height(screen);
	if(left) {
		posx = 0;
	} else {
		posx = x-wx;
	}
	if(top) {
		posy = 15;
	} else {
		posy = y - 15;
	}

	gtk_window_move(GTK_WINDOW(win), posx, posy);
}

static gboolean icon_press(GtkStatusIcon *status_icon, GdkEventButton *event, void *data){
	int i;
	if(event->button == 1) {
		if(popup) {
			if(gtk_widget_get_visible(GTK_WIDGET(popup))){
				gtk_widget_hide(GTK_WIDGET(popup));
				return TRUE;
			}
		}
		if(!updated) {
			window_move(popup, status_icon, event);
			gtk_widget_show_all(GTK_WIDGET(popup));
			return TRUE;
		}
		if (popup)
			gtk_widget_destroy(GTK_WIDGET(popup));
		i = list_online(tlist);
		popup = new_popup_window(i);
		window_move(popup, status_icon, event);
		//window_move_absolute(popup);
		window_populate(GTK_CONTAINER(popup), 0);

		gtk_window_present(GTK_WINDOW(popup));
		gtk_widget_show_all(GTK_WIDGET(popup));

		//updated = FALSE;
		return TRUE;
	}
	if(event->button == 3) {
		popup_menu(status_icon, event, data);
	}
	return TRUE;
}

static GtkStatusIcon* status_icon_new(void) {
	status_icon = gtk_status_icon_new();
	//gtk_status_icon_set_from_icon_name(status_icon, "audio-volume-muted-panel");
	gtk_status_icon_set_from_file(status_icon, APP_ICON_SMALL);
	g_signal_connect(G_OBJECT(status_icon), "button_press_event",
		G_CALLBACK(icon_press), NULL);
	return status_icon;
}

static void menu_pref(GtkMenuItem *item, void *data) {
	//static GtkWidget *window;
}

// do not like
static void menu_about(GtkMenuItem *item, void *data) {
	GtkWidget * aboutDialog = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), "tic");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), VERSION);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutDialog),
		gdk_pixbuf_new_from_file(APP_ICON, NULL));
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutDialog), "Copyright (c) Per Odlund 2014");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutDialog), "Twitch swytemtray monitor");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutDialog),	"http://alephnull.se/software/tic");
	gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(aboutDialog), GTK_LICENSE_MIT_X11);
	gtk_dialog_run(GTK_DIALOG(aboutDialog));
	gtk_widget_destroy(aboutDialog);
}

static void menu_quit(GtkMenuItem *item, void *data) {
	//destroy all widgets (if needed)
	gtk_main_quit();
}

static GtkWidget* popup_menu_new(void) {
	GtkWidget *menu = gtk_menu_new();

	GtkWidget *pref = gtk_menu_item_new_with_mnemonic(("_Preferences"));
	GtkWidget *about= gtk_menu_item_new_with_mnemonic(("_About"));
	GtkWidget *quit = gtk_menu_item_new_with_mnemonic(("_Quit"));

	g_signal_connect(G_OBJECT(pref), "activate", G_CALLBACK(menu_pref), NULL);
	g_signal_connect(G_OBJECT(about), "activate", G_CALLBACK(menu_about), NULL);
	g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(menu_quit), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), pref);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), about);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit);

	return menu;
}

static void popup_menu(GtkStatusIcon *status_icon, GdkEventButton *event, void *data){
	gtk_widget_show_all(menu);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
	gtk_status_icon_position_menu, status_icon,
		 event->button, event->time);
}

static gboolean update_worker(void *data) {
	update_all(chandle, tlist);
	return TRUE;
}

int main(int argc, char *argv[]){
	chandle = curl_easy_init();
	gtk_init(&argc, &argv);
	tlist = list_new("Twoeasy");
	tlist = list_push(tlist, "khezzu");
	tlist = list_push(tlist, "TaKeTVRed");
	tlist = list_push(tlist, "WagamamaTV");
	update_all(chandle, tlist);

	menu = popup_menu_new();
	status_icon = status_icon_new();
	
	gdk_threads_add_timeout(1000*60*5, update_worker, NULL);
	gtk_main();


	// something icon
	return 0;
}
