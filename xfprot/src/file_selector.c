/*
*
* Gtk+2.x  Utility routines: File selection widget.
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

/*#################################### File Selection Widget #####################################################*/

#include <stdio.h>
#include <gtk/gtk.h>
#include "mylib.h"
#include "mygtk.h"

/*
GTK_FILE_CHOOSER_ACTION_OPEN,
GTK_FILE_CHOOSER_ACTION_SAVE,
GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
*/

char *file_chooser(const char *title, GtkFileChooserAction fs_action, gboolean local_only, gboolean show_hidden, 
					const char *name, const char *folder, const char *shortcut_folder)
{
	GtkWidget *dialog;
	char *tmp;
	char *filename = NULL;;

	tmp = g_locale_to_utf8((title) ? title : "", -1,NULL,NULL,NULL);

	dialog = gtk_file_chooser_dialog_new (tmp,
						NULL, /*parent_window*/
						fs_action,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
	xfree(&tmp);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), local_only);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), show_hidden);
	/* Returns a GsList *, not a char * */
	/*gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), select_multiple);*/

	if (name) {
		tmp = g_locale_to_utf8((name), -1, NULL, NULL, NULL);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), tmp);
		xfree(&tmp);	
	}
	
	if (folder) {
		tmp = g_locale_to_utf8((folder), -1, NULL, NULL, NULL);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), tmp);
		xfree(&tmp);
	}

	if (shortcut_folder) {
		tmp = g_locale_to_utf8((shortcut_folder), -1, NULL, NULL, NULL);
		gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), tmp, NULL);
		xfree(&tmp);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
	}
	if (G_IS_OBJECT (dialog)) {
		gtk_widget_destroy (dialog);
	}
	return filename;
}

char *default_file_chooser(const char *title, GtkFileChooserAction fs_action)
{
	return file_chooser(title, fs_action, FALSE, TRUE, NULL, NULL, NULL);
}

/*############################### End of File Selection Widget #####################################################*/
