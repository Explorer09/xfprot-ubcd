/*
*
* Gtk Utility routines: Text View Widget.
* Copyright (C) 2002-2009 by Tito Ragusa <tito-wolit@tiscali.it>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* Please read the COPYING and README file!!!
*
* Report bugs to <tito-wolit@tiscali.it>
*
*/

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mygtk.h"
#include "mylib.h"

/********************* Text file View Widget *******************************/

static int view_lock = 0;
static GtkWidget *view_window;
static GtkWidget *view_text;

static void text_view_quit_callback(void)
{
	if (view_lock) return;

	gtk_main_quit();
	gtk_widget_destroy(view_window);
}

static void text_view_wrap(void)
{
	if (view_lock) return;

	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(view_text),
				(gtk_text_view_get_wrap_mode (GTK_TEXT_VIEW(view_text)) != GTK_WRAP_NONE) ?
				GTK_WRAP_NONE :
				GTK_WRAP_WORD);
}

void textview_window_create(char * title, int x, int y, int border, char *filename, FILE *fd)
{
	GtkWidget *menubar;
	GtkWidget *table;
	GtkWidget *filemenu;
	GtkWidget *option_menu;
	GtkWidget *options;
	GtkWidget *toggle_wrap;
	GtkWidget *help_menu;
	GtkWidget *help;
	GtkWidget *about;
	GtkWidget *file;
	GtkWidget *quit;
	ProgressData *pdata;
	GtkAccelGroup *accel_group = NULL;
	FILE *f;
	struct stat st;

	DBG_FUNCTION_NAME;
	
	view_window = window_create(x, y, border, title, G_CALLBACK(text_view_quit_callback), NULL, MODAL);
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(view_window), accel_group);
	table =  table_create(20, 21,GTK_WIDGET(view_window), 0, HOMOGENEUS);

	menubar = gtk_menu_bar_new();
	attach_to_table (table, menubar, 0, 20, 0, 1);

	filemenu = gtk_menu_new();
	file = gtk_image_menu_item_new_with_mnemonic(_("_File"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file), gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));	

	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	g_signal_connect(G_OBJECT(quit), "activate",G_CALLBACK(text_view_quit_callback), NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

	option_menu = gtk_menu_new();
	options = gtk_image_menu_item_new_with_mnemonic(_("_Options"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(options), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));

	toggle_wrap = gtk_check_menu_item_new_with_mnemonic(_("_Wrap"));
	gtk_widget_add_accelerator(toggle_wrap, "activate", accel_group, GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 
	g_signal_connect(G_OBJECT(toggle_wrap), "activate",G_CALLBACK(text_view_wrap), NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_wrap), FALSE);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(options), option_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(option_menu), toggle_wrap);

	help_menu = gtk_menu_new();
	help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_group);
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel_group);
	g_signal_connect(G_OBJECT(about), "activate",G_CALLBACK(about_dialog), NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about);

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), options);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);

	
	view_text = textpad_create_on_table(NOT_EDITABLE, NULL, NULL, NO_WRAP, SCROLL, table, 0, 20, 1, 20);
	pdata = progressbar_create_on_table((fd) ? ACTIVITY : PULSE, table, 0, 10, 20, 21);
	gtk_widget_show_all(GTK_WIDGET(view_window));
	view_lock = 1;
	if (fd) {
		read_from_stream_and_display_text(view_text, fd, 0, NULL);
	} else if ((f = open_file(filename)) != NULL &&	stat(filename, &st) == 0 ) {
		read_from_stream_and_display_text(view_text, f, st.st_size, &pdata->value);
		xfclose_nostdin(f);
	}
	progress_destroy(pdata);
	view_lock = 0;
	gtk_main();
}

/***************End of Text file View Widget ********************************/
