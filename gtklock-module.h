// gtklock-userinfo-module
// Copyright (c) 2022 Jovan Lanik

// Module header

#include "gtk/gtk.h"

struct Window {
	GdkMonitor *monitor;

	GtkWidget *window;
	GtkWidget *window_box;
	GtkWidget *body;
	GtkWidget *input_box;
	GtkWidget *input_label;
	GtkWidget *input_field;
	GtkWidget *message_box;
	GtkWidget *unlock_button;
	GtkWidget *error_label;
	GtkWidget *clock_label;

	gulong enter_notify_handler;

	void *module_data;
};

struct GtkLock {
	GtkApplication *app;
	GArray *windows;
	GArray *messages;
	GArray *errors;

	struct Window *focused_window;
	gboolean hidden;
	guint idle_timeout;

	guint draw_clock_source;
	guint idle_hide_source;

	gboolean use_layer_shell;
	gboolean use_input_inhibit;
	gboolean use_idle_hide;

	char *time;
	char *time_format;
	char *config_path;
};

const gchar *g_module_check_init(GModule *m);
void g_module_unload(GModule *m);
void on_activation(struct GtkLock *gtklock);
void on_output_change(struct GtkLock *gtklock);
void on_focus_change(struct GtkLock *gtklock, struct Window *win, struct Window *old);
void on_window_empty(struct GtkLock *gtklock, struct Window *ctx);
void on_body_empty(struct GtkLock *gtklock, struct Window *ctx);

/*

MIT Licence

Copyright (c) 2022 Jovan Lanik <jox969@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
MIT Licence

*/

