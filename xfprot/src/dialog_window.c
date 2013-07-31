/*
*
* Gtk+2.x  Utility routines: Dialog Window
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
#include "mygtk.h"
#include "mylib.h"

/*######################### Start of dialog_window Widget ##########################################################*/

static void dialog_ok_callback(GtkWidget *widget,  Dialog_data* data)
{
	DBG_FUNCTION_NAME;
	assert(widget != NULL);

	SET2(*data->ptr_dialog_retval, 0);
	gtk_main_quit();
	gtk_widget_destroy(data->window);
}

static void dialog_no_callback(GtkWidget *widget,  Dialog_data* data)
{
	DBG_FUNCTION_NAME;
	assert(widget != NULL);

	SET2(*data->ptr_dialog_retval, 1);
	gtk_main_quit();
	gtk_widget_destroy(data->window);
}

static void dialog_third_callback(GtkWidget *widget, Dialog_data* data)
{
	DBG_FUNCTION_NAME;
	assert(widget != NULL);

	SET2(*data->ptr_dialog_retval, 2);
	gtk_main_quit();
	gtk_widget_destroy(data->window);
}

static void dialog_check_callback(GtkWidget *widget, int *ptr_check_retval )
{
	DBG_FUNCTION_NAME;
	assert(widget != NULL);

	SET2(*ptr_check_retval, (*ptr_check_retval == 0) ? 3 : 0);
}

int dialog_window(const char *title, int type, char *third_button_label, int def_button, const char *fmt, ...)
{
	/*
	int type is:
	#define	ONE_BUTTON				((int) 1)
	#define	ONE_BUTTON_ONE_CHECK	((int) 2)
	#define	TWO_BUTTONS				((int) 3)
	#define	TWO_BUTTONS_ONE_CHECK	((int) 4)
	#define	THREE_BUTTONS			((int) 5)

	int def_button = button number from left to right, starting with 1.
	If def_button = 0 no button is activated.
	On ONE_BUTTON and ONE_BUTTON_ONE_CHECK dialog windows the button 
	is activated by default unless you set def_button to 0.
	
	Returns 	int=0 for ok/yes
				int=1 for no
				int=2 for third button
				int=3 for ok/yes (with checkbutton ACTIVE )
				int=4 for no (with checkbutton ACTIVE )
	*/
		
	GtkWidget *button1;
	GtkWidget *button2;
	GtkWidget *button3;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *frame;
	GtkWidget *bbox;
	GtkWidget *hbox;
	GtkWidget *image = NULL;
	GtkWidget *checkbutton;
	int dialog_retval = -1;
	int check_retval = 0;
	int *ptr_check_retval;
	char *label_text = NULL;
	int size = 100;
	int n = 0;
	va_list ap;
	struct Dialog_data* data;

	DBG_FUNCTION_NAME;
	
	debug_err_msg("%s: %s def_button value: %d", __FUNCTION__,
				(def_button < 0 || def_button > 3) ? "illegal" : "", def_button);
	
	data = (Dialog_data *)xmalloc(sizeof(Dialog_data));
	data->ptr_dialog_retval = &dialog_retval;
	ptr_check_retval = &check_retval;

	if (strcmp(title, GTK_STOCK_DIALOG_INFO) == 0) {
		image =   gtk_image_new_from_icon_name(title, GTK_ICON_SIZE_DIALOG);
		title = _("Info");
	} else 	if (strcmp(title, GTK_STOCK_DIALOG_WARNING) == 0) {
		image =   gtk_image_new_from_icon_name(title, GTK_ICON_SIZE_DIALOG);
		title = _("Warning");
		gdk_beep();
	} else 	if (strcmp(title, GTK_STOCK_DIALOG_ERROR) == 0) {
		image =   gtk_image_new_from_icon_name(title, GTK_ICON_SIZE_DIALOG);
		title = _("Error");
		gdk_beep();
	} else 	if (strcmp(title, GTK_STOCK_DIALOG_QUESTION) == 0) {
		image =   gtk_image_new_from_icon_name(title, GTK_ICON_SIZE_DIALOG);
		title = _("Question");
	}  else if (strcmp(title, GTK_STOCK_ABOUT) == 0) {
		image =   gtk_image_new_from_icon_name(title, GTK_ICON_SIZE_DIALOG);
		title = _("About");
		label_text = g_markup_printf_escaped(
			"<span size=\"xx-large\" weight=\"bold\">%s\n</span><span>%s\n%s\n%s %s\n%s</span>",
			PACKAGE_STRING,
			COPYRIGHT,
			HOMEPAGE,
			_("Licensed under the"), LICENSE,
			_("This program comes with ABSOLUTELY NO WARRANTY"));
	} else if (!title) {
		title = _("Dialog Window");
	}

	data->window = window_create(380, 180, 2,(title == NULL) ? _("Dialog Window") : title,
								 (def_button > 0 && def_button <= 3 )?
								 G_CALLBACK(gtk_main_quit):
								 NULL,
								 NULL, MODAL);
	find_toplevel_set_transient(data->window);
	table = table_create( 6, 6, data->window, 0, NOT_HOMOGENEUS);

	frame = frame_create_on_table(NULL, table, 0, 6, 0, 5);
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	if (image) {
		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	}

	if (!label_text) {
		/* From xmake_message */
		label_text = (char *)xmalloc(size);
		while (1) {
			/* Try to print in the allocated space. */
			va_start(ap, fmt);
			n = vsnprintf (label_text, size, fmt, ap);
			va_end(ap);
			/* If that worked, return the string. */
			if (n > -1 && n < size) {
				break;
			}
			/* Else try again with more space. */
			if (n > -1) {		/* glibc 2.1 */
				size = n + 1;	/* precisely what is needed */
			} else {			/* glibc 2.0 */
				size *= 2;		/* twice the old size */
			}
			label_text = (char *)xrealloc (label_text, size);
		}
		label = label_create(label_text);
	} else {
		label = gtk_label_new (NULL);
 		gtk_label_set_markup (GTK_LABEL (label), label_text);
	}	
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	gtk_misc_set_padding (GTK_MISC(label), 8, 8);
	gtk_box_pack_end (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	bbox = default_button_box_create_on_table(HORIZONTAL, table, 0, 6, 5, 6);

	/* ONE_BUTTON: this button is always created. */
	button1 = button_create_in_container((type == ONE_BUTTON || type == ONE_BUTTON_ONE_CHECK) ? GTK_STOCK_OK : GTK_STOCK_YES,
														G_CALLBACK(dialog_ok_callback), data , bbox);
	
	if (type == ONE_BUTTON || type == ONE_BUTTON_ONE_CHECK) {
			gtk_widget_grab_focus(button1);
	}
	
	if (type == TWO_BUTTONS || type == TWO_BUTTONS_ONE_CHECK || type == THREE_BUTTONS) {
		button2 = button_create_in_container(GTK_STOCK_NO, G_CALLBACK(dialog_no_callback), data, bbox);
		if (def_button == 2) {
			gtk_widget_grab_focus(button2);
		}
	}

	if (type == THREE_BUTTONS) {
		debug_err_msg("%s: third button label: %s",__FUNCTION__,
					(third_button_label) ? third_button_label : "missing");
		button3 = button_create_in_container(third_button_label, G_CALLBACK(dialog_third_callback), data, bbox);
		if (def_button == 3) {
			gtk_widget_grab_focus(button3);
		}
	}

	if (type == ONE_BUTTON_ONE_CHECK || type == TWO_BUTTONS_ONE_CHECK) {
		debug_err_msg("%s: checkbox label: %s",__FUNCTION__,
					(third_button_label) ? third_button_label : "missing");	
		checkbutton = check_create_on_table(third_button_label, G_CALLBACK(dialog_check_callback),
											ptr_check_retval, INACTIVE, table, 0,3,5,6);
	}

	gtk_widget_show_all(data->window);
	//if (def_button == 0) {
		/* With GTK 2 button 1 is focused by default even if def_button is zero,
		this is a workaround */
		//GTK_WIDGET_SET_FLAGS(frame, GTK_CAN_FOCUS);
		//gtk_widget_grab_focus(frame);
	//}
	
	main_loop(&data->window);
	
	if (dialog_retval == -1) {
		/* NO button clicked, use default button action,
		if no def button is set the window close callback is disabled
		and the user is forced to use a button */
		debug_err_msg("dialog: using default button action"); 
		/* Default action is set so return it */	
		dialog_retval = (def_button - 1);
	}
	xfree(&label_text);
	xfree(&data);
	debug_err_msg("dialog window: returning %d", dialog_retval+check_retval);

	return (dialog_retval + check_retval);
}

/*######################### End of dialod_window Widget   ##########################################################*/

/*######################### Standard about window widget  ##########################################################*/

void about_dialog(void)
{
	dialog_window(GTK_STOCK_ABOUT, ONE_BUTTON, NULL, 1, "%s", "");
}

/*######################### End of Standard about window widget  ###################################################*/

/*######################### Standard about window widget  ##########################################################*/

void error_dialog(const char *message)
{
	dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, "%s", message);
}

/*######################### End of Standard about window widget  ###################################################*/

/*######################### Standard not implemented dialog ########################################################*/
#if 0  /* UNUSED */
void not_implemented(void)
{
	DBG_FUNCTION_NAME;
	
	dialog_window(PACKAGE_STRING, ONE_BUTTON, NULL, 1, "Not implemented yet!");
}
#endif
/*######################### Standard not implemented dialog ########################################################*/


