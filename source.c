// gtklock-userinfo-module
// Copyright (c) 2024 Erik Reider, Jovan Lanik

// User info module

#include <act/act.h>

#include "gtklock-module.h"

#define MODULE_DATA(x) (x->module_data[self_id])
#define USERINFO(x) ((struct userinfo *)MODULE_DATA(x))

extern void config_load(const char *path, const char *group, GOptionEntry entries[]);

struct userinfo {
	GtkWidget *user_revealer;
	GtkWidget *user_box;
	GtkWidget *user_icon;
	GtkWidget *user_name;
};

const gchar module_name[] = "userinfo";
const guint module_major_version = 4;
const guint module_minor_version = 0;

static int self_id;

static ActUserManager *act_manager = NULL;
static ActUser *act_user = NULL;

static gboolean no_round_image = FALSE;
static gboolean horizontal_layout = FALSE;
static gboolean under_clock = FALSE;
static gint image_size = 96;

GOptionEntry module_entries[] = {
	{ "no-round-image", 0, 0, G_OPTION_ARG_NONE, &no_round_image, NULL, NULL },
	{ "horizontal-layout", 0, 0, G_OPTION_ARG_NONE, &horizontal_layout, NULL, NULL },
	{ "under-clock", 0, 0, G_OPTION_ARG_NONE, &under_clock, NULL, NULL },
	{ "image-size", 0, 0, G_OPTION_ARG_INT, &image_size, NULL, NULL },
	{ NULL },
};

static void window_set_userinfo(ActUser* user, struct Window *ctx) {
	gboolean loaded = FALSE;
	g_object_get(user, "is-loaded", &loaded, NULL);
	if(loaded != TRUE) return;

	const char *name = act_user_get_real_name(user);
	if(name == NULL) {
		g_warning("userinfo-module: User name not found");
		name = act_user_get_user_name(user);
	}
	gtk_label_set_text(GTK_LABEL(USERINFO(ctx)->user_name), name);

	if(image_size < 0) {
		g_warning("userinfo-module: Invalid image size: %d, using default value", image_size);
		image_size = 96;
	}

	GdkPixbuf *pixbuf = NULL;

	const char *path = act_user_get_icon_file(user);
	if(path != NULL) {
		GError *error = NULL;
		pixbuf = gdk_pixbuf_new_from_file_at_size(path, image_size, image_size, &error);
		if(error != NULL) {
			g_warning("userinfo-module: User image error: %s", error->message);
			g_error_free(error);
		}
	} else g_warning("userinfo-module: User image not found");

	if(pixbuf == NULL) {
		GtkIconTheme *theme = gtk_icon_theme_get_default();
		GtkIconInfo *icon =
			gtk_icon_theme_lookup_icon(theme, "avatar-default-symbolic", image_size, GTK_ICON_LOOKUP_FORCE_SIZE);
		GError *error = NULL;
		pixbuf = gtk_icon_info_load_icon(icon, &error);
		if(error != NULL) {
			g_warning("userinfo-module: User icon error: %s", error->message);
			g_error_free(error);
		}
	}

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, image_size, image_size);
	cairo_t *cr = cairo_create(surface);
	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);

	// Makes the image circular
	if(!no_round_image) {
		const int half_size = image_size / 2;
		cairo_arc(cr, half_size, half_size, half_size, 0, 2 * G_PI);
		cairo_clip(cr);
	}

	cairo_paint(cr);
	gtk_image_set_from_surface(GTK_IMAGE(USERINFO(ctx)->user_icon), surface);
}

static void gtklock_set_userinfo(ActUser* user, struct GtkLock *gtklock) {
	if(gtklock->focused_window != NULL) window_set_userinfo(user, gtklock->focused_window);
}

static void gtklock_user_loaded(ActUser* user, GParamSpec *spec, struct GtkLock *gtklock) {
	if(gtklock->focused_window != NULL) window_set_userinfo(user, gtklock->focused_window);
}

static void init_user_manager(struct GtkLock *gtklock) {
	if(act_manager == NULL) act_manager = act_user_manager_get_default();
	if(act_user_manager_no_service(act_manager)) {
		g_warning("userinfo-module: AccountsService is not running");
		return;
	} else if(act_user == NULL) {
		const char *username = g_get_user_name();
		act_user = act_user_manager_get_user(act_manager, username);
		g_signal_connect(act_user, "notify::is-loaded", G_CALLBACK(gtklock_user_loaded), gtklock);
		g_signal_connect(act_user, "changed", G_CALLBACK(gtklock_set_userinfo), gtklock);
	}
}

static void setup_userinfo(struct Window *ctx) {
	if(MODULE_DATA(ctx) != NULL) {
		gtk_widget_destroy(USERINFO(ctx)->user_revealer);
		g_free(MODULE_DATA(ctx));
		MODULE_DATA(ctx) = NULL;
	}
	MODULE_DATA(ctx) = g_malloc(sizeof(struct userinfo));

	USERINFO(ctx)->user_revealer = gtk_revealer_new();
	gtk_widget_set_halign(USERINFO(ctx)->user_revealer, GTK_ALIGN_CENTER);
	gtk_widget_set_name(USERINFO(ctx)->user_revealer, "user-revealer");
	gtk_revealer_set_reveal_child(GTK_REVEALER(USERINFO(ctx)->user_revealer), TRUE);
	gtk_revealer_set_transition_type(GTK_REVEALER(USERINFO(ctx)->user_revealer), GTK_REVEALER_TRANSITION_TYPE_NONE);
	gtk_container_add(GTK_CONTAINER(ctx->info_box), USERINFO(ctx)->user_revealer);

	if(!under_clock) gtk_box_reorder_child(GTK_BOX(ctx->info_box), USERINFO(ctx)->user_revealer, 0);

	//if(under_clock) g_object_set(USERINFO(ctx)->user_revealer, "margin-top", 10, NULL);

	GtkOrientation o = horizontal_layout ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL ;
	USERINFO(ctx)->user_box = gtk_box_new(o, 5);
	gtk_widget_set_halign(USERINFO(ctx)->user_box, GTK_ALIGN_CENTER);
	gtk_widget_set_name(USERINFO(ctx)->user_box, "user-box");
	gtk_container_add(GTK_CONTAINER(USERINFO(ctx)->user_revealer), USERINFO(ctx)->user_box);

	// Profile picture
	USERINFO(ctx)->user_icon = gtk_image_new();
	gtk_widget_set_name(USERINFO(ctx)->user_icon, "user-image");
	gtk_container_add(GTK_CONTAINER(USERINFO(ctx)->user_box), USERINFO(ctx)->user_icon);

	// Profile name
	USERINFO(ctx)->user_name = gtk_label_new(NULL);
	gtk_widget_set_name(USERINFO(ctx)->user_name, "user-name");
	gtk_container_add(GTK_CONTAINER(USERINFO(ctx)->user_box), USERINFO(ctx)->user_name);

	window_set_userinfo(act_user, ctx);
	gtk_widget_show_all(USERINFO(ctx)->user_revealer);
}

void g_module_unload(GModule *m) {
	g_object_unref(act_user);
	g_object_unref(act_manager);
}

void on_activation(struct GtkLock *gtklock, int id) {
	self_id = id;

	init_user_manager(gtklock);

	GtkCssProvider *provider = gtk_css_provider_new();
	GError *err = NULL;
	const char css[] =
		"#user-name {"
		"font-size: 18pt;"
		"}"
		;

	gtk_css_provider_load_from_data(provider, css, -1, &err);
	if(err != NULL) {
		g_warning("Style loading failed: %s", err->message);
		g_error_free(err);
	} else {
		gtk_style_context_add_provider_for_screen(
			gdk_screen_get_default(),
			GTK_STYLE_PROVIDER(provider),
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
		);
	}

	g_object_unref(provider);
}

void on_focus_change(struct GtkLock *gtklock, struct Window *win, struct Window *old) {
	setup_userinfo(win);
	if(gtklock->hidden)
		gtk_revealer_set_reveal_child(GTK_REVEALER(USERINFO(win)->user_revealer), FALSE);
	if(old != NULL && win != old)
		gtk_revealer_set_reveal_child(GTK_REVEALER(USERINFO(old)->user_revealer), FALSE);
}

void on_window_destroy(struct GtkLock *gtklock, struct Window *ctx) {
	if(MODULE_DATA(ctx) != NULL) {
		gtk_widget_destroy(USERINFO(ctx)->user_revealer);
		g_free(MODULE_DATA(ctx));
		MODULE_DATA(ctx) = NULL;
	}
}

void on_idle_hide(struct GtkLock *gtklock) {
	if(gtklock->focused_window) {
		GtkRevealer *revealer = GTK_REVEALER(USERINFO(gtklock->focused_window)->user_revealer);	
		gtk_revealer_set_reveal_child(revealer, FALSE);
	}
}

void on_idle_show(struct GtkLock *gtklock) {
	if(gtklock->focused_window) {
		GtkRevealer *revealer = GTK_REVEALER(USERINFO(gtklock->focused_window)->user_revealer);	
		gtk_revealer_set_reveal_child(revealer, TRUE);
	}
}

