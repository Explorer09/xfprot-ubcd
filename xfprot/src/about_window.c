/*
*
* Gtk+2.x  Utility routines: About and license dialog Window
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

/********************* About and license file viewer Widget *******************************/

/* Needs TextCommon.c */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <assert.h>
#include <stdlib.h>
#include "mygtk.h"
#include "mylib.h"

static int about_lock = 0;
static GtkWidget *about_window;
static GtkWidget *about_text;

void about_wrap_text(void)
{
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(about_text),
								(gtk_text_view_get_wrap_mode (GTK_TEXT_VIEW(about_text)) != GTK_WRAP_NONE) ?
								GTK_WRAP_NONE :
								GTK_WRAP_WORD);
}

static void license_callback(GtkWidget *widget, int *ptr_status)
{
	DBG_FUNCTION_NAME;
	assert(widget != NULL);
	
	SET2(*ptr_status,(*ptr_status == 0) ? 1 : 0);
}

static void about_quit(void)
{
	if (about_lock)
		return;

	gtk_main_quit();
	gtk_widget_destroy(about_window);
}

int about_window_create(char * title, int x, int y, int border, char *labeltext, FILE *fd, int first_time)
{
	GtkWidget *menubar;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *bbox;
	GtkWidget *check_button;
	GtkWidget *button;
	GtkWidget *option_menu;
	GtkWidget *options;
	GtkWidget *toggle_wrap;
	GtkAccelGroup *accel_group = NULL;
	char *text = NULL;
	int *ptr_status;
	int status = 0;

	DBG_FUNCTION_NAME;

	ptr_status = &status;
	
	/* Make the gui first  */
	about_window = window_create(x, y, border, title ? title : _("About"), about_quit, NULL, MODAL);
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(about_window), accel_group);
	find_toplevel_set_transient(about_window);
	table = table_create( 20, 18, about_window , 0, NOT_HOMOGENEUS);

	menubar = gtk_menu_bar_new();
	gtk_table_attach_defaults (GTK_TABLE(table), GTK_WIDGET(menubar), 0, 20, 0, 1);

	option_menu = gtk_menu_new();
	options = gtk_image_menu_item_new_with_mnemonic(_("_Options"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(options), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));

	toggle_wrap = gtk_check_menu_item_new_with_label(_("_Wrap"));
	gtk_widget_add_accelerator(toggle_wrap, "activate", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 
	g_signal_connect(G_OBJECT(toggle_wrap), "activate",G_CALLBACK(about_wrap_text), NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_wrap), FALSE);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(options), option_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(option_menu), toggle_wrap);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), options);

	frame = frame_create_on_table( NULL, table, 0, 20, 1, 4);
	if (!labeltext) {
		text = g_markup_printf_escaped(
				"<span size=\"xx-large\" weight=\"bold\">%s\n</span><span>%s\n%s\n%s %s\n%s</span>",
				PACKAGE_STRING,
				COPYRIGHT,
				HOMEPAGE,
				_("Licensed under the"), LICENSE,
				_("This program  comes with ABSOLUTELY NO WARRANTY"));
		label = gtk_label_new (NULL);
		gtk_label_set_markup (GTK_LABEL (label), text);
	} else {
		label = label_create(labeltext);	
	}
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(frame), label);	
	about_text = textpad_create_on_table(NOT_EDITABLE, NULL , NULL, NO_WRAP, SCROLL, table,  0, 20, 4, 16);
	bbox = default_button_box_create_on_table(HORIZONTAL, table, 7, 20, 16, 18);
	button = button_create_in_container(GTK_STOCK_OK, about_quit, NULL, bbox);
	
	if (first_time == FIRST_TIME) {
		check_button = check_create_on_table(_("I agree, don't show it again"),
										G_CALLBACK(license_callback),
										ptr_status, INACTIVE, table, 0, 3, 17, 18);
	}

	gtk_widget_show_all(about_window);
	about_lock = 1;	
	/* Read from fd */
	if (fd)
		read_from_stream_and_display_text(about_text, fd, 0, NULL);
	/* Function that calls fopen() should warn on error */

	about_lock = 0;
	main_loop(&about_window);
	xfree(&text);
	return status;
}

int default_about_window(FILE *fd, int first_time)
{
	return about_window_create(PACKAGE_STRING, 620, 430, 8, NULL, fd, first_time);
}

/*************** End of  About and license file viewer Widget ********************************/
