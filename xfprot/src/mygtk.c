/*
*
* Gtk+2.x Utility routines: Base widgets
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


void find_toplevel_set_transient(GtkWidget *window)
{
	GtkWindow *parent = NULL;
	GList *tmplist = gtk_window_list_toplevels ();

	while (tmplist && tmplist->next) {
		if (tmplist->data
			&& tmplist->data != window
			&& GTK_IS_WIDGET(tmplist->data)
			&& GTK_IS_WINDOW((tmplist->data))
			&& gtk_window_is_active(GTK_WINDOW(tmplist->data))
		) {
			debug_err_msg("toplevel parent %s", gtk_widget_get_name(tmplist->data));
			parent = tmplist->data;
		}
		tmplist = tmplist->next;
	}
	if (parent) {
		gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
		gtk_window_set_position(GTK_WINDOW(GTK_WIDGET(window)), GTK_WIN_POS_CENTER_ON_PARENT);
	} else {
		gtk_window_set_position(GTK_WINDOW(GTK_WIDGET(window)), GTK_WIN_POS_CENTER);
	}
	g_list_free(tmplist);
}

void main_loop(GtkWidget ** window)
{
	if ((GTK_IS_WIDGET (*window))) {
		gtk_widget_show_all(*window);
	}
	gtk_main();
	if ((GTK_IS_WIDGET (*window))) {
		 gtk_widget_destroy(*window);
	}
	*window = NULL;
}

void refresh_gui(void)
{
	while (gtk_events_pending ()) {
		gtk_main_iteration();
	}
}

void attach_to_table(GtkWidget * table, GtkWidget * widget,
					int start_col, int end_col, int start_row, int end_row)
{
	gtk_table_attach_defaults(GTK_TABLE(table), GTK_WIDGET(widget), start_col, end_col, start_row, end_row);
}

/********************* Text box widget ***************************************/

GtkWidget *default_text_box_create_on_table(char *text, int editable, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *entry_box;

	entry_box = text_box_create(text, editable);
	attach_to_table(table, entry_box, start_col, end_col, start_row, end_row);
	return entry_box;
}

GtkWidget *text_box_create_on_table(char *text, int editable, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row,
					GtkAttachOptions xoptions,  GtkAttachOptions yoptions,  
					 guint xpadding, guint ypadding, int maxlen, int width)
{
	GtkWidget *entry_box;
	
	entry_box = text_box_create(text, editable);
	gtk_table_attach (GTK_TABLE (table), entry_box, start_col, end_col, start_row, end_row,
						(GtkAttachOptions) xoptions,
						(GtkAttachOptions) yoptions, xpadding, ypadding);
	if (maxlen > 0) {
		gtk_entry_set_max_length (GTK_ENTRY (entry_box), maxlen);
	}
	if (width > 0) {
		gtk_entry_set_width_chars (GTK_ENTRY (entry_box), width);
	}
	return entry_box;
}

void my_gtk_entry_set_text(GtkEntry *entry, char *data)
{
	char *tmp;

	if (!data) data = "";

	if (g_utf8_validate(data, -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(data, -1, NULL, NULL, NULL); 
		gtk_entry_set_text(entry, tmp);
		xfree(&tmp);
	} else {
		gtk_entry_set_text(entry, data);
	}
}

GtkWidget *text_box_create(char *text, int editable)
{
	GtkWidget *entry_box;

	entry_box = gtk_entry_new();
	my_gtk_entry_set_text(GTK_ENTRY(entry_box), text);
	gtk_editable_set_editable(GTK_EDITABLE (entry_box), (editable == EDITABLE) ? TRUE : FALSE);
	return entry_box;
}
#if 0  /* UNUSED */
char *my_gtk_entry_get_text(GtkEntry *entry)
{
	char *tmp;
	
	tmp = (char *)gtk_entry_get_text(entry);
	trim(tmp);
	return (tmp) ? tmp : "";
}

/********************* End of Text box widget ********************************/

/********************* Standard radio button widget **************************/
#endif
GtkWidget *radio_create(GtkWidget * last_of_group, char *label,
			GCallback func, gpointer func_data, int active)
{
	GtkWidget *button;
	char *tmp;

	if (!label) label = "";

	if (g_utf8_validate(label, -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(label, -1, NULL, NULL, NULL);
		button = gtk_radio_button_new_with_label((last_of_group == NULL) ?
									NULL :
									gtk_radio_button_get_group(GTK_RADIO_BUTTON(last_of_group)),
									tmp);
		xfree(&tmp); 
	} else {
		button = gtk_radio_button_new_with_label((last_of_group == NULL) ?
									NULL :
									gtk_radio_button_get_group(GTK_RADIO_BUTTON(last_of_group)),
									label);
	}

	if (func) {
		g_signal_connect(GTK_OBJECT(button), "toggled", G_CALLBACK(func), func_data);
	}

	if (active == ACTIVE) {
		/* gtk_toggle_button_set_active is not needed on first button (last_of_group==NULL)
			because it is set active by default, but to be sure.......
			*/
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
		gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(button));
	}
	return button;
}

/****************End of Standard radio button widget *************************/

/********************* Radio button on table widget **************************/

GtkWidget *radio_create_on_table(GtkWidget * last_of_group, char *label, GCallback func, gpointer func_data,
					int active, GtkWidget * table, int start_col, int end_col,
					int start_row, int end_row)/*window_close*/
{
	GtkWidget *button;

	button = radio_create(last_of_group, label, func, func_data, active);
	attach_to_table(table, button, start_col, end_col, start_row, end_row);
	return button;
}

#if 0  /* UNUSED */
GtkWidget *default_radio_create_on_table(GtkWidget * last_of_group, char *label, GCallback func,
					int active, GtkWidget * table, int start_col, int end_col,
					int start_row, int end_row)
{
	GtkWidget *button;
	
	button = radio_create(last_of_group, label, func, label, active);
	attach_to_table(table, button, start_col, end_col, start_row, end_row);
	return button;
}

/****************End of radio button on table widget *************************/

/********************* Check button widgets **************************/
#endif
GtkWidget *check_create(char *label, GCallback func, gpointer func_data, int active)
{
	GtkWidget *button;
	char *tmp;

	if (!label) label = "";

	if (g_utf8_validate(label, -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(label, -1,NULL,NULL,NULL);
		button = gtk_check_button_new_with_label(tmp);
		xfree(&tmp);
	} else  {
		button = gtk_check_button_new_with_label(label);
	}
	if (func) {
		g_signal_connect(GTK_OBJECT(button), "toggled", G_CALLBACK(func), func_data);
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), (active == ACTIVE) ? TRUE : FALSE);
	return button;
}

GtkWidget *check_create_on_table(char *label, GCallback func, gpointer func_data, int active,
					GtkWidget * table, int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *button;
	
	button = check_create(label, func, func_data, active);
	attach_to_table(table, button, start_col, end_col, start_row, end_row);
	return button;
}
#if 0  /* UNUSED */
GtkWidget *default_check_create_on_table(char *label, GCallback func, int active,
						GtkWidget * table, int start_col, int end_col, int start_row,
						int end_row)
{
	GtkWidget *button;
	
	button = check_create(label, func, label, active);
	attach_to_table(table, button, start_col, end_col, start_row, end_row);
	return button;
}

/***************End of Check button widgets **********************************/

/********************* Standard button widget ********************************/
#endif
GtkWidget *button_create(char *label, GCallback func, gpointer func_data)
{
	GtkWidget *button;
	char *tmp;
	GtkStockItem *item = xmalloc(sizeof(GtkStockItem));
	
	if (!label) label = "";

	if (gtk_stock_lookup(label, item) == TRUE) {
		button = gtk_button_new_from_stock(label);
	} else {
		if (g_utf8_validate(label, -1, NULL) != TRUE) {
			tmp = g_locale_to_utf8(label, -1,NULL,NULL,NULL);
			button = gtk_button_new_with_label(tmp);
			xfree(&tmp);
		} else {
			button = gtk_button_new_with_label(label);
		}
	}
	if (func) {
		g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(func), func_data);
	}
	return button;
}

/****************End of Standard button widget *******************************/

/********************* Button on table widget ********************************/

GtkWidget *button_create_on_table(char *label, GCallback func, gpointer func_data,
					GtkWidget * table, int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *button;

	button = button_create(label, func, func_data);
	attach_to_table(table, button, start_col, end_col, start_row, end_row);
	return button;
}

/****************End of button on table widget ******************************/

/********************* Button in container widget ***************************/

GtkWidget *button_create_in_container(char *label, GCallback func, gpointer func_data,
						GtkWidget * container)
{
	GtkWidget *button;

	button = button_create(label, func, func_data);
	gtk_container_add(GTK_CONTAINER(container), button);
	return button;
}

/*************End of Button in container widget *****************************/

/********************* Buttonbox widget *************************************/

GtkWidget *button_box_create(int orientation, int layout, int border, int spacing)
{
	GtkWidget *bbox;

	if (orientation == HORIZONTAL) {
		bbox = gtk_hbutton_box_new();
	} else {         /* VERTICAL*/
		bbox = gtk_vbutton_box_new();
	}

	gtk_container_set_border_width(GTK_CONTAINER(bbox), border);
	/*
	GTK_BUTTONBOX_DEFAULT_STYLE  Default packing.
	GTK_BUTTONBOX_SPREAD Buttons are evenly spread across the ButtonBox.
	GTK_BUTTONBOX_EDGE   Buttons are placed at the edges of the ButtonBox.
	GTK_BUTTONBOX_START  Buttons are grouped towards the start of box, (on the left for a HBox,
                         or the top for a VBox).
	GTK_BUTTONBOX_END    Buttons are grouped towards the end of a box, (on the right for a HBox,
                         or the bottom for a VBox). */
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), layout);
	gtk_box_set_spacing(GTK_BOX(bbox), spacing);
	return bbox;
}

GtkWidget *default_button_box_create(int orientation)
{
	GtkWidget *bbox;

	/* These default values look good to me. If you don't like it
		use button_box_createto set your own */
	return bbox = button_box_create(orientation, GTK_BUTTONBOX_END, 10, 20);
}

/***************End of Buttonbox widget *************************************/

/********************* Buttonbox on table widget ****************************/

GtkWidget *button_box_create_on_table(int orientation, int layout, int border,
					int spacing, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *bbox;

	bbox = button_box_create(orientation, layout, border, spacing);
	attach_to_table(table, bbox, start_col, end_col, start_row, end_row);
	return bbox;
}

GtkWidget *default_button_box_create_on_table(int orientation, GtkWidget * table,
							int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *bbox;

	/* These default values look good to me. If you don't like it
		use button_box_create_on_table()     to set your own */
	return bbox = button_box_create_on_table(orientation, GTK_BUTTONBOX_END, 10, 20,
						table, start_col, end_col, start_row, end_row);
}

/***************End of Buttonbox on table widget ****************************/

/****************************** Label widget ********************************/

GtkWidget *label_create(char *label_text)
{
	GtkWidget *label;
	char *tmp;

	if (!label_text) label_text = "";

	if (g_utf8_validate(label_text, -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(label_text, -1,NULL,NULL,NULL);
		label = gtk_label_new(tmp);
		xfree(&tmp);
	} else {
		label = gtk_label_new(label_text);		
	}
	return label;
}

/********************* Label on table widget ********************************/

GtkWidget *default_label_create_on_table(char *label_text, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *label;
	
	label = label_create(label_text);
	attach_to_table(table, label, start_col, end_col, start_row, end_row);
	return label;
}

#if 0  /* UNUSED */
GtkWidget *label_create_on_table(char *label_text, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row,
					 GtkAttachOptions xoptions, GtkAttachOptions yoptions, 
					 guint xpadding, guint ypadding, gfloat xalign, gfloat yalign)
{
	GtkWidget *label;


	label = label_create(label_text);	
	gtk_table_attach (GTK_TABLE (table), label, start_col, end_col, start_row, end_row,
										xoptions, yoptions, xpadding, ypadding);
	gtk_misc_set_alignment (GTK_MISC (label), xalign, yalign);
	return label;	
}

/***************End of Label on table widget ********************************/
#endif
/********************* Standard table widget ********************************/
GtkWidget *table_create(int cols, int rows, GtkWidget * window, int border, int homogeneus)
{
	GtkWidget *table;

	table = gtk_table_new(rows, cols, TRUE);
	gtk_table_set_homogeneous(GTK_TABLE(table), homogeneus);
	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_container_set_border_width(GTK_CONTAINER(table), border);
	return table;
}
#if 0  /* UNUSED */
	GtkWidget *table_create_on_table(int cols, int rows, GtkWidget * table, int border,
					int homogeneus, int start_col, int end_col, int start_row, int end_row)
{
		GtkWidget *new_table;

		new_table = gtk_table_new(rows, cols, TRUE);
		gtk_table_set_homogeneous(GTK_TABLE(new_table), homogeneus);
		gtk_table_attach_defaults(GTK_TABLE(table), GTK_WIDGET(new_table), 
									start_col, end_col, start_row, end_row);
		gtk_container_set_border_width(GTK_CONTAINER(table), border);
		return new_table;
	}

/********************* Standard table widget ********************************/

/********************* Standard window widget ********************************/
#endif
GtkWidget *window_create(int x, int y, int border, const char *title, 
						GCallback func, void *func_data, int modal)
{
	GtkWidget *window;
	GdkScreen *screen;
	char *tmp;
	int screen_height;
	int screen_width;
	
	/* get a handle to the screen for its measurements */
	screen = gdk_screen_get_default ();
	/* using the gdk library functions, get the height and width */
	screen_height = gdk_screen_get_height (screen);
	screen_width = gdk_screen_get_width (screen);
	/* leave some buffer room around our main window */
	/* allowing room for things like taskbars that may be on the screen */
	screen_height -= SCREEN_MARGIN;
	screen_width -= SCREEN_MARGIN;
	
	if (x > 0 ) {
	/* window requests explicitely a size, so check that it is not bigger than the screen .*/	
		if (x > screen_width)
			x = screen_width;
	} else if (x == 0) {
	/* no size request use the default if it is not bigger than the screen */
		if (screen_width > 540)
			x = 540;
	} else if (x < 0) {
	/* negative size is flag for full-screen window */
		x = screen_width;
	}

	if (y > 0 ) {
	/* window requests explicitely a size, so check that it is not bigger than the screen .*/	
		if (y > screen_height)
			y = screen_height;
	} else if (y == 0) {
	/* no size request use the default if it is not bigger than the screen */
		if (screen_height > 320)
			y = 320;
	} else if (y < 0) {
	/* negative size is flag for full-screen window */
		x = screen_height;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* Sets the default size of a window. If the window's "natural" size (its size request) */
	/* is larger than the default, the default will be ignored. */
	gtk_window_set_default_size(GTK_WINDOW(window), x, y);
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	if (func) {
		g_signal_connect(GTK_OBJECT(window), "delete_event", G_CALLBACK(func), func_data);
	} else {
		g_signal_connect(GTK_OBJECT(window), "delete_event", G_CALLBACK(gtk_true), func_data);
	}

	if (!title)
		title = "";

	if (g_utf8_validate(title , -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(title , -1,NULL,NULL,NULL);	
		gtk_window_set_title(GTK_WINDOW(window), tmp);
		xfree(&tmp);
	} else {
		gtk_window_set_title(GTK_WINDOW(window), title);
	}
	gtk_container_set_border_width(GTK_CONTAINER(window), border);
	gtk_window_set_modal(GTK_WINDOW(window), (modal == MODAL) ? TRUE : FALSE);
	return window;
}

/********************* Standard window widget ********************************/

/******************************* Frame widget ********************************/

GtkWidget *frame_create(char *label)
{
	GtkWidget *frame;
	char *tmp;

	if (!label) label = "";
	
	if (g_utf8_validate(label , -1, NULL) != TRUE) {
		tmp = g_locale_to_utf8(label, -1,NULL,NULL,NULL);	
		frame = gtk_frame_new(tmp);
		xfree(&tmp);
	} else {
		frame = gtk_frame_new(label);
	}
	return frame;
}

/******************************* Frame widget ********************************/

/********************* Frame on table widget ********************************/

GtkWidget *frame_create_on_table(char *label, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *frame;

	frame = frame_create(label);
	attach_to_table(table, frame, start_col, end_col, start_row, end_row);
	return frame;
}

/**************End of Frame on table widget ********************************/
#if 0  /* UNUSED */
/************** Create a separator *****************************************/

GtkWidget *separator_create_on_table(int orientation, GtkWidget * table, int start_col, int end_col,
						int start_row, int end_row) {
	GtkWidget *separator;

	if (orientation == HORIZONTAL) {
		separator = gtk_hseparator_new();
	} else {        /* VERTICAL) */
		separator = gtk_vseparator_new();
	}
	attach_to_table(table, separator, start_col, end_col, start_row, end_row);
	return separator;
}

/************** End of Create a separator **********************************/

/************** Create a Multi column scrolled list ************************/

static void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
	GtkWidget *menu, *menu_items;
	struct StringListPopupMenuEntry *popupmenu = userdata;

	assert(treeview != NULL);

	menu = gtk_menu_new();

	while(popupmenu->label) {
		/* Create a new menu-item with a name... */
		menu_items = gtk_menu_item_new_with_label (popupmenu->label);

		/* ...and add it to the menu. */
		gtk_menu_shell_append  ((GtkMenuShell *)(menu), menu_items);

		/* Do something interesting when the menuitem is selected */
		g_signal_connect_swapped (G_OBJECT (menu_items), "activate",
									G_CALLBACK (popupmenu->callbackf), 
									&(popupmenu->id));
		popupmenu++;
	}
	/* Show the widget */
	gtk_widget_show_all(menu);
	/* Note: event can be NULL here when called from view_onPopupMenu;
	* gdk_event_get_time() accepts a NULL argument */
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
	(event != NULL) ? event->button : 0,
	gdk_event_get_time((GdkEvent*)event));
	return;
}

gboolean view_onButtonPressed(GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
	GtkTreeSelection *selection;
	GtkTreePath *path;

	/* single click with the right mouse button? */
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		/* select row if no row is selected */
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		/* Note: gtk_tree_selection_count_selected_rows() does not
		* exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
		if (gtk_tree_selection_count_selected_rows(selection) <= 1) {
		/* Get tree path for row that was clicked */
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
				(gint) event->x,
				(gint) event->y,
				&path, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
				gtk_tree_path_free(path);
			}
		}	
		debug_err_msg("Single right click on the tree view");
		view_popup_menu(treeview, event, userdata);
		return TRUE; /* we handled this */
	}
	return FALSE; /* we did not handle this */
}

void string_list_popupmenu(GtkWidget *list, gpointer popupmenu)
{
	g_signal_connect(list, "button-press-event", (GCallback)view_onButtonPressed, popupmenu);
}

void clear_list(GtkWidget *list)
{
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(list))));
}

void add_data_to_list(GtkWidget *tree, char *data_string[], int n_columns, int autoscroll)
{
	GtkTreeIter iter;
	GtkListStore *store;
	GtkTreePath *path;
	GtkTreeModel* model;
	char *data;
	int i;
	
	DBG_FUNCTION_NAME;

	store = (GtkListStore *)gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
	gtk_list_store_append(GTK_LIST_STORE(store), &iter);
	

	for (i = 0; i < n_columns; i++) {

		if (!data_string || !*data_string) *data_string = "";

		if (g_utf8_validate(*data_string, -1, NULL) != TRUE) {
			if ((data = g_locale_to_utf8(*data_string, -1, NULL, NULL, NULL))) {
				gtk_list_store_set(GTK_LIST_STORE(store), &iter, i, data, -1);
				xfree(&data);
			}
		} else {
			gtk_list_store_set(GTK_LIST_STORE(store), &iter, i, *data_string, -1);
		}
		data_string++;
	}
	if (autoscroll) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));	
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(model), &iter);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree), path, NULL, TRUE, 0.0, 0.0);
		gtk_tree_path_free(path);
	}
}

static void list_edited(GtkCellRendererText * cell, gchar * path_string, gchar * new_text, gpointer user_data)
{
	GtkTreeView *treeview = (GtkTreeView *)user_data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	guint column;
	
	/* Column number is passed as renderer object data */
	gpointer columnptr = g_object_get_data(G_OBJECT(cell), "column");
	
	column = GPOINTER_TO_UINT(columnptr);

	/* Get the iterator */
	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_from_string(model, &iter, path_string);
	
	/* Update the model */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, new_text, -1);
}

static void text_editing_started (GtkCellRenderer ATTRIBUTE_#if 0  /* UNUSED */ *cell, GtkCellEditable *editable,
                      const gchar *path, GCallback data)
{
	debug_err_msg("%s: started", __FUNCTION__);

	if (GTK_IS_ENTRY (editable)) {
		GtkEntry *entry = GTK_ENTRY (editable);
		GCallback cb_func = data;
		g_signal_connect(GTK_OBJECT(entry), "activate", (GCallback)cb_func, (char *)xstrdup(path));
	}
}


GtkWidget *string_list_create(int num, GCallback func, int show_hide, int editable,...)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;
	GtkWidget *tree;
	GtkTreeSelection *selection;
	GType *types;
	va_list titles;
	int i;
	char *tmp;
	char *label;
	double align;

	types = (GType *) malloc((num + 1) * sizeof(GType));
	for (i = 0; i < num; i++) {
		types[i] = G_TYPE_STRING;
	}

	store = gtk_list_store_newv(num, types);
	xfree(&types);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);
	/* Setup the selection handler */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	if (func) {
		g_signal_connect(selection, "changed", G_CALLBACK(func), NULL);
	}
	/* The view now holds a reference.  We can get rid of our own reference */
	g_object_unref(G_OBJECT(store));

	va_start(titles, editable);
	for (i = 0; i < num; i++) {
		/* Create a cell renderer */
		renderer = gtk_cell_renderer_text_new();

		tmp = va_arg(titles, char *);
		align = va_arg(titles, double);

		if (editable == EDITABLE || (editable == CUSTOM && va_arg(titles, int) == EDITABLE)) {
			g_object_set(renderer,"editable", TRUE, NULL);
			g_object_set_data(G_OBJECT(renderer), "column", GUINT_TO_POINTER(i));
			g_signal_connect(GTK_OBJECT(renderer), "edited", G_CALLBACK(list_edited), tree);
			if (editable == CUSTOM) {
				g_signal_connect(GTK_OBJECT(renderer), "editing-started", G_CALLBACK(text_editing_started),
				(gpointer) va_arg(titles, void*));
			}
		}

		if (!tmp || show_hide == HIDE )  tmp = "";

		if (g_utf8_validate(tmp, -1, NULL) != TRUE) {
			label = g_locale_to_utf8(tmp , -1, NULL, NULL, NULL);
			/* Create a column, associating the "text" attribute of the  cell_renderer to the column of the model */
			column = gtk_tree_view_column_new_with_attributes(label, renderer, "text", i, NULL);
			xfree(&label);
		} else {
			column = gtk_tree_view_column_new_with_attributes(tmp, renderer, "text", i, NULL);
		}
		g_object_set (renderer,"xalign", align, NULL);
		/* Add the column to the view. */
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	}
	va_end(titles);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), (show_hide == HIDE) ? FALSE : TRUE);
	return tree;
}


GtkWidget *string_list_create_on_table(int num, GCallback func,
						GtkWidget * table, int start_col, int end_col,
						int start_row, int end_row, int show_hide, int editable,...)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;
	GtkWidget *scrolled_window;
	GtkWidget *tree;
	GtkTreeSelection *selection;
	GType *types;
	va_list titles;
	int i;
	char *tmp;
	char *label;
	double align;


	/* This is the scrolled window to put the cList widget inside */
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_ALWAYS,
						GTK_POLICY_ALWAYS);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 8);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_ETCHED_IN);

	types = (GType *) xmalloc(num * sizeof(GType));
	for (i = 0; i < num; i++) {
		types[i] = G_TYPE_STRING;
	}
	
	store = gtk_list_store_newv(num, types);
	xfree(&types);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);
	/* Setup the selection handler */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	if (func) {
		g_signal_connect(selection, "changed", G_CALLBACK(func), NULL);
	}

	gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(tree));

	if (table) {
		attach_to_table(table, GTK_WIDGET(scrolled_window), start_col, end_col, start_row, end_row);
	}

	/* The view now holds a reference.  We can get rid of our own reference */
	g_object_unref(G_OBJECT(store));

	va_start(titles, editable);
	for (i = 0; i < num; i++) {
		/* Create a cell renderer */
		renderer = gtk_cell_renderer_text_new();
	
		tmp = va_arg(titles, char *);
		align = va_arg(titles, double);	
	
		if (editable == EDITABLE || (editable == CUSTOM && va_arg(titles, int) == EDITABLE)) {
			g_object_set(renderer,"editable", TRUE, NULL);
			g_object_set_data(G_OBJECT(renderer), "column", GUINT_TO_POINTER(i));	
			g_signal_connect(GTK_OBJECT(renderer), "edited", G_CALLBACK(list_edited), tree);
			if (editable == CUSTOM) {
				g_signal_connect(GTK_OBJECT(renderer), "editing-started", G_CALLBACK(text_editing_started),
				(gpointer) va_arg(titles, void*));
			}
		}

		if (!tmp || show_hide == HIDE ) tmp = "";

		/* Create a column, associating the "text" attribute of the  cell_renderer to the column of the model */
		if (g_utf8_validate(tmp, -1, NULL) != TRUE) {
			label = g_locale_to_utf8(tmp, -1, NULL, NULL, NULL);
			column = gtk_tree_view_column_new_with_attributes(label, renderer, "text", i, NULL);
			xfree(&label);
		} else {
			column = gtk_tree_view_column_new_with_attributes(tmp, renderer, "text", i, NULL);
		}
		g_object_set (renderer,"xalign", align, NULL);
		/* Add the column to the view. */
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	}
	va_end(titles);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), (show_hide == HIDE) ? FALSE : TRUE);
	return tree;
}

/******** End of Create a Multi column scrolled list ************************/

/************** Create a statusbar **********************************/

GtkWidget *statusbar_create_on_table(GtkWidget *table, GtkWidget **label, char *labeltext, int start_col,
						int end_col, int start_row, int end_row)
{

	GtkWidget *frame;

	frame = frame_create_on_table(NULL, table, start_col, end_col, start_row, end_row);

	/* Allowed values for shadow type are:
		GTK_SHADOW_NONE,
		GTK_SHADOW_IN,
		GTK_SHADOW_OUT,
		GTK_SHADOW_ETCHED_IN,
		GTK_SHADOW_ETCHED_OUT
		*/

	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	*label = label_create(labeltext);
	gtk_misc_set_alignment((GtkMisc *) * label, 0.5, 0.5);
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(*label));
	return frame;
}

/************** End of Create a statusbar **********************************/
#endif
/******************* Create a Textpad **************************************/

GtkWidget *textpad_create(int editable, GCallback func, void *func_arg, int wrap)
{
	GtkWidget *text;

	text = gtk_text_view_new();

	if (editable == EDITABLE) {
		gtk_text_view_set_editable(GTK_TEXT_VIEW(text), TRUE);
		/* It's best to connect a signal here to have Gtk tell us if the text gets modified! */
		/* but only for text editing widgets so also NULL is allowed */
		if (func) {
			g_signal_connect(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), "changed",
						G_CALLBACK(func), func_arg);
		}
	} else {
		gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	}

	if (wrap == WRAP) {
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
	}
	return text;
}

GtkWidget *textpad_create_on_table(int editable, GCallback func, void *func_arg, int wrap, int scroll,
					GtkWidget * table, int start_col, int end_col, int start_row, int end_row)
{
	GtkWidget *text, *frame, *sw;

	text = textpad_create(editable, func, func_arg, wrap);
	frame = gtk_frame_new(NULL);
	attach_to_table(table, frame, start_col, end_col, start_row, end_row);
	if (scroll == SCROLL) {
		sw = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
		gtk_container_set_border_width(GTK_CONTAINER(sw), 8);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
							GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
		gtk_container_add(GTK_CONTAINER(frame), sw);
		gtk_container_add(GTK_CONTAINER(sw), text);
	} else {
		gtk_container_add(GTK_CONTAINER(frame), text);
	}
	return text;
}

/************ End of Create a Textpad **************************************/

/************ Create a Main Menubar   **************************************/

GtkWidget *main_menu_create(GtkWidget *window, MenuEntry *menu_items, const gchar *info, gpointer data)
{
	GtkUIManager *ui;
	GtkActionGroup *actions;
	GError *error = NULL;
	int i;

	actions = gtk_action_group_new("Actions");

	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	for (i = 0;; i++) {
		if (menu_items[i].simple_entries != NULL) {
		/* translates string automagically */
		gtk_action_group_add_actions(actions, menu_items[i].simple_entries,
								menu_items[i].num_entries, data);
		} else if (menu_items[i].toggle_entries != NULL) {
			/* translates string automagically */
			gtk_action_group_add_toggle_actions(actions, menu_items[i].toggle_entries,
								menu_items[i].num_entries, data);
		} else if (menu_items[i].radio_entries != NULL) {
			/* translates string automagically */
			gtk_action_group_add_radio_actions(actions, menu_items[i].radio_entries,
								menu_items[i].num_entries,
								menu_items[i].active_radio_entry,
								G_CALLBACK(menu_items[i].radio_entry_callback),
								data);
		} else {
			break;
		}
		
	}
	ui = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui, actions, 0);
	gtk_window_add_accel_group(GTK_WINDOW(window), gtk_ui_manager_get_accel_group(ui));

	if (gtk_ui_manager_add_ui_from_string(ui, info, -1, &error) == 0) {	/* something bad happened */
		debug_err_msg("%s() building menus failed: %s", error->message, __FUNCTION__);
		g_error_free(error);
	}
	gtk_ui_manager_ensure_update(ui);
	return gtk_ui_manager_get_widget(ui, "/MenuBar");	/* Returns NULL on failure */
}

/************ End of Create a Main Menubar**********************************/

/************ Progressbar widget  ******************************************/

static gboolean progress_timeout(gpointer data )
{
	ProgressData *pdata = (ProgressData *)data;
	gdouble val; 
	char percent[5];

	if (!GTK_IS_WIDGET(pdata->pbar)) return FALSE;

	/* one more step */		
	val = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (pdata->pbar));
	
	if (!pdata->activity_mode) {
		val += 0.01;
		if ( val > 1.0) {
			/* 100 % done, stop it */
			return FALSE;
		}
		/* Set the new value */
		snprintf(percent,5,"%.0f%%",(val * 100));
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pdata->pbar),  percent);
	} else {
		val += 0.05;
		if (val > 1.0) {
			val = 0.0;
			gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR (pdata->pbar), 
					(gtk_progress_bar_get_orientation (GTK_PROGRESS_BAR (pdata->pbar)) == GTK_PROGRESS_RIGHT_TO_LEFT) ?
					GTK_PROGRESS_LEFT_TO_RIGHT : GTK_PROGRESS_RIGHT_TO_LEFT);
		}
	}
	
	/* Set the new value */
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pdata->pbar), val);

	/* As this is a timeout function, return TRUE so that it
	* continues to get called */
	return TRUE;
}

void progress_destroy(ProgressData *pdata)
{
	DBG_FUNCTION_NAME;

	if (pdata == NULL) {
		return;
	}
	if (GTK_IS_WIDGET(pdata->pbar)) {
		gtk_widget_hide(pdata->pbar);
	}
	if (pdata->timer > 0) {
		g_source_remove(pdata->timer);
	}
	pdata->timer = 0;
	if (GTK_IS_WIDGET(pdata->pbar)) {
		gtk_widget_destroy(pdata->pbar);
		pdata->pbar = NULL;
	}
	if (pdata->alive > 0 && GTK_IS_WIDGET(pdata->window)) {
		gtk_widget_destroy(pdata->window);
		pdata->window = NULL;
	}
	pdata->value = 0;
	xfree(&pdata);
}

ProgressData *progressbar_create_on_table(int mode, GtkWidget * table,
						int start_col, int end_col, int start_row, int end_row)
{
	ProgressData *pdata = progressbar_create(mode);
	attach_to_table(table, pdata->pbar, start_col, end_col, start_row, end_row);
	return pdata;
}

ProgressData *progressbar_create(int mode)
{
	ProgressData *pdata;


	/* Allocate memory for the data that is passed to the callbacks */
	pdata = xmalloc(sizeof(ProgressData));

	/* Initialize */
	pdata->window = NULL;
	/* destroy when done */
	pdata->alive = 0;

	/* Create the GtkProgressBar */
	pdata->pbar = gtk_progress_bar_new();

	/* Add a timer callback to update the value of the progress bar */
	pdata->timer = g_timeout_add(100, progress_timeout, pdata);
	pdata->activity_mode = mode;
	pdata->value = 0.0;
	return pdata;
}

/************ Progressbar widget  ******************************************/
#if 0  /* UNUSED */
/************** Create a Pack box  ************************************/

GtkWidget *box_create(int orientation, int homogeneous, int spacing)
{
	GtkWidget *box;
	
	DBG_FUNCTION_NAME;

	if (orientation == VERTICAL) {
		/* VERTICAL */
		box = gtk_vbox_new((homogeneous == HOMOGENEUS) ? TRUE : FALSE, spacing);
	} else {
		/* HORIZONTAL */
		box = gtk_hbox_new((homogeneous == HOMOGENEUS) ? TRUE : FALSE, spacing);
	}
	return box;
}

GtkWidget *default_box_create_on_table (int orientation, int homogeneous, GtkWidget *table,
				int spacing, int start_col, int end_col, int start_row, int end_row)
{
	DBG_FUNCTION_NAME;
	GtkWidget *box;
	box = box_create(orientation, homogeneous, spacing);
	attach_to_table(table, box, start_col, end_col, start_row, end_row);
	return box;
}

GtkWidget *box_create_on_table(int orientation, int homogeneous, GtkWidget * table,
				int spacing, int start_col, int end_col, int start_row, int end_row,
				GtkAttachOptions xoptions,  GtkAttachOptions yoptions,  
				guint xpadding, guint ypadding)
{
	DBG_FUNCTION_NAME;
	GtkWidget *box;
	box = box_create(orientation, homogeneous, spacing);
	gtk_table_attach (GTK_TABLE (table), box, start_col, end_col, start_row, end_row,
										xoptions, yoptions, xpadding, ypadding);
	return box;
}

/************ Create a pack box  ********************************/
#endif
/************ Set busy cursor  ********************************/

void setbusycursor(GtkWidget *widget, int flag)
{
	GdkCursor *busyCursor = gdk_cursor_new(GDK_WATCH);
	
	if (flag == TRUE) {
		if(gtk_widget_get_visible(widget))
			gdk_window_set_cursor(widget->window, busyCursor);
	} else {
		if(gtk_widget_get_visible(widget))
			gdk_window_set_cursor(widget->window, NULL);
	}
}

/************ Set busy cursor  ********************************/
