/*
*
* Gtk+2.x  Utility routines: Text Editor Window
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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include "mygtk.h"
#include "mylib.h"

static int edit_lock = 0;
static GtkWidget *edit_window;
static GtkWidget *edit_table;	
static GtkWidget *edit_text;
static struct stat sourcefileinfo;
static int text_modified = 0;
static char *path = NULL;

/********************* Text file edit Widget *******************************/

void edit_wrap_text (void)
{
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(edit_text),
								(gtk_text_view_get_wrap_mode (GTK_TEXT_VIEW(edit_text)) != GTK_WRAP_NONE) ?
								GTK_WRAP_NONE :
								GTK_WRAP_WORD);
}

static void save_file(char *filename, GtkWidget *text)
{
	/*   Inspired by files.c of nano editor 1.2.2                            *
	*                                                                        *
	*   Copyright (C) 1999-2003 Chris Allegretta GPL                         *
	*   realeased under the terms of the GPL                                 */

	GtkTextBuffer	*buffer;
	GtkTextIter		start;
	GtkTextIter		end;
	char *data = NULL;
	int flag;
	int fd;
	mode_t mode = 0600;
	FILE *file = NULL;
	struct stat fileinfo;

	if (filename[0] == '\0' || filename == NULL ) {
		error_dialog(_("No file name"));
		return;
	}

	if (stat(filename, &fileinfo) == 0) {
		/* File exists */
		if (S_ISDIR(fileinfo.st_mode) || S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode)) {
			if(S_ISDIR(fileinfo.st_mode)) {
				dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("'%s' is a directory"), filename);
			} else {
				dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("'%s' is a device file"), filename);
			}
			return;
		}
		mode = fileinfo.st_mode;
		flag = O_WRONLY| O_TRUNC ;
	} else {
		flag = O_WRONLY| O_CREAT;
	}

	if ((fd = open(filename, flag, mode)) == -1) {
		dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, "'%s': %s", filename, strerror(errno));
		return;
	}
	if ((file = fdopen(fd, "wb"))) {
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
		gtk_text_buffer_get_bounds (buffer, &start, &end);
		data = g_locale_from_utf8(gtk_text_buffer_get_text(buffer, &start, &end,
									/*include_hidden_chars*/ FALSE), -1, NULL, NULL, NULL);
		if (data) {
			fprintf(file,"%s", data);
			xfree(&data);
		} else {
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("Cannot save '%s': unknown error"), filename);
		}
		fclose(file);
	} else {
		dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, "Cannot write file '%s': %s", filename, strerror(errno));
	}
	close(fd);
}

static void text_changed_callback(void)
{
	text_modified = 1;
}

static int overwrite(char *msg_str)
{
	DBG_FUNCTION_NAME;

	if (dialog_window(GTK_STOCK_DIALOG_WARNING, TWO_BUTTONS, NULL, 2, "%s", msg_str) == 1) {
		return 1;
	}
	return 0;
}

static void save_common(void)
{
	mode_t mask;
	struct stat destfileinfo;
		
	/* File exists */
	/* g_locale_from_utf8(path,-1, NULL, NULL, NULL);*/
	if (stat(path, &destfileinfo) == 0) {
		if (overwrite(_("The destination file exists.\nDo you want to overwrite it ?")) == 1) {
			return;
		}
		mask = destfileinfo.st_mode;
		/* File is newer */
		if (sourcefileinfo.st_mtime!=0 && sourcefileinfo.st_mtime > destfileinfo.st_mtime ) {
			if (overwrite(_("The destination file was modified\nafter we opened it.\n"
							"Do you want to overwrite\nthis more recent version ?")) == 1)
				return;
		}
	} else {
		if (sourcefileinfo.st_mode != 0) {
			mask = sourcefileinfo.st_mode;
		} else {
			mask = 0600; /* Use default file permissions */
		}
	}
	/* write file*/ 
	save_file(path, edit_text);
	text_modified = 0;
	/* Set permission */
	if (chmod(path, mask) == -1) {
		dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1,
								_("Cannot set file permissions on\n'%s':\n%s"),
								path, strerror(errno));
	}
	/* Set ownership is not needed as it is set to that of the current process */
	gtk_window_set_title(GTK_WINDOW(edit_window), path);
}

static void save_as_cb(void)
{
	char *new_path;
	
	if ((new_path = default_file_chooser(_("Save as"), GTK_FILE_CHOOSER_ACTION_SAVE)) == NULL) {
		return;
	}
	xfree(&path);
	path = new_path;
	save_common();
}

static void save_cb(void)
{
	if (text_modified == 0) {
		return;
	}
	if (path == NULL) {
		/* No file name: ask for it */ 
		if ((path = default_file_chooser( _("Save as"), GTK_FILE_CHOOSER_ACTION_SAVE)) == NULL) {
			/* Cancel button */
			return;
		}
	}
	save_common();
}

static void open_common(void)
{
	ProgressData * pdata = NULL;
	FILE *file = NULL;
	GtkTextIter		iter;
	char *title = NULL;

	DBG_FUNCTION_NAME;

	/* During file reading delete_event and quit signals are ignored */
	edit_lock = 1;
	gtk_text_buffer_get_iter_at_offset (gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit_text)), &iter, 0);

	if (path && g_utf8_validate(path , -1, NULL) != TRUE) {
		title = g_locale_to_utf8(path, -1, NULL, NULL, NULL);
	} else {
		title = xstrdup(path);
	}

	if (path && stat(path, &sourcefileinfo) == 0) {
		if ((file = open_file(path)) != NULL) {
			/* Errors in opening the file are showed by open_file itself */
			gtk_window_set_title(GTK_WINDOW(edit_window), title);
			pdata = progressbar_create_on_table(PULSE, edit_table, 0, 10, 20, 21 );
			gtk_widget_show(pdata->pbar);
			read_from_stream_and_display_text(edit_text, file, sourcefileinfo.st_size, &pdata->value);
			xfclose_nostdin(file);
			progress_destroy(pdata);
		} else {
			/* Some error occurred in opening the file: 
			S_ISDIR, S_ISCHR, S_ISBLK, so create create an untitled empty edit window */
			gtk_window_set_title(GTK_WINDOW(edit_window), _("Untitled") );
		}
	} else {
		/*create an empty edit window titled or untitled */
		gtk_window_set_title(GTK_WINDOW(edit_window),(path) ? title : _("Untitled"));
	}
	xfree(&title);
	text_modified = 0;
	edit_lock = 0;
}

static void new_cb(void)
{
	if (!edit_lock) {
		if (text_modified){
			if (!path) {
				/* No file name: ask for it */ 
				if ((path = default_file_chooser(_("Save as"), GTK_FILE_CHOOSER_ACTION_SAVE)) ==NULL) {
					/* Cancel button */
					return;
				}
			}	
			save_common();
		}
		gtk_window_set_title(GTK_WINDOW(edit_window), _("Untitled"));
		clear_the_buffer(edit_text, &text_modified);
		xfree(&path);
		text_modified = 0;
		sourcefileinfo.st_mtime = 0;
		sourcefileinfo.st_mode = 0;
	}
}

static void open_cb(void)
{
	if (!edit_lock) {
		new_cb();
		if ((path = default_file_chooser(_("Open"), GTK_FILE_CHOOSER_ACTION_OPEN))) {
			open_common();
		}
	}
}

static void quit_cb(void)
{	
	DBG_FUNCTION_NAME;

	if (!edit_lock) {
		if (text_modified
		&& dialog_window(_("Save file?"), TWO_BUTTONS, NULL, 2, 
						_("The File has been modified.\nDo you want to save it?")) == 0) {
			if (!path) {
				/* No file name: ask for it */ 
				if ((path = default_file_chooser(_("Save as"), GTK_FILE_CHOOSER_ACTION_SAVE)) == NULL) {
					/* Cancel button */
					return;
				}
			}	
			save_common();
		}
		gtk_main_quit();
		gtk_widget_destroy(edit_window);
	}
}

void textedit_window_create(char * title, int x, int y, int border, char *filename)
{
	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *option_menu;
	GtkWidget *options;
	GtkWidget *toggle_wrap;
	GtkWidget *help_menu;
	GtkWidget *help;
	GtkWidget *about;
	GtkWidget *file;
	GtkWidget *new;
	GtkWidget *do_open;
	GtkWidget *save;
	GtkWidget *save_as;
	GtkWidget *sep;
	GtkWidget *quit;
	GtkAccelGroup *accel_group = NULL;

	DBG_FUNCTION_NAME;
	path = xstrdup(filename);
	edit_window = window_create( x, y, border, title,  G_CALLBACK (quit_cb), NULL,  MODAL);
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(edit_window), accel_group);

	edit_table = table_create( 20, 21,GTK_WIDGET(edit_window), 0, HOMOGENEUS);
	menubar = gtk_menu_bar_new();
	gtk_table_attach_defaults (GTK_TABLE(edit_table),GTK_WIDGET(menubar), 0, 20, 0, 1);

	filemenu = gtk_menu_new();
	file = gtk_image_menu_item_new_with_mnemonic(_("_File"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file), gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	
	new = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, accel_group);
	g_signal_connect(G_OBJECT(new), "activate",G_CALLBACK(new_cb), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);

	do_open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, accel_group);
	g_signal_connect(G_OBJECT(do_open), "activate",G_CALLBACK(open_cb), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), do_open);

	save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, accel_group);
	g_signal_connect(G_OBJECT(save), "activate",G_CALLBACK(save_cb), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);

	save_as = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, accel_group);
	g_signal_connect(G_OBJECT(save_as), "activate",G_CALLBACK(save_as_cb), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save_as);

	sep = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep);

	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	g_signal_connect(G_OBJECT(quit), "activate",G_CALLBACK(quit_cb), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

	option_menu = gtk_menu_new();
	options = gtk_image_menu_item_new_with_mnemonic(_("_Options"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(options), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));

	toggle_wrap = gtk_check_menu_item_new_with_mnemonic(_("_Wrap"));
	gtk_widget_add_accelerator(toggle_wrap, "activate", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 
	g_signal_connect(G_OBJECT(toggle_wrap), "activate",G_CALLBACK(edit_wrap_text), NULL);
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

	edit_text = textpad_create_on_table(EDITABLE, G_CALLBACK(text_changed_callback),
									&text_modified, NO_WRAP, SCROLL, edit_table,  0, 20, 1, 20);
	gtk_widget_show_all(GTK_WIDGET(edit_window));
	open_common();
	main_loop(&edit_window);
	xfree(&path);
}

/***************End of Text file edit Widget ********************************/
