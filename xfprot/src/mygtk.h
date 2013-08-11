/*
 *
 * Gtk+2.x Utility routines
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

#ifndef	__HAVE_MYGTK_
#define	__HAVE_MYGTK_ 1

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define DEF_BUFFER_SIZE		 512
#define NOT_EDITABLE	((int) 0)
#define EDITABLE		((int) 1)
#define CUSTOM			((int)-1)
#define ACTIVE 			((int) 0)
#define INACTIVE		((int) 1)
#define HORIZONTAL		((int) 0)
#define VERTICAL		((int) 1)
#define	NOT_MODAL		((int) 0)
#define	MODAL			((int) 1)
#define SHOW			((int) 0)
#define HIDE			((int) 1)
#define WRAP			((int) 0)
#define NO_WRAP			((int) 1)
#define SCROLL			((int) 0)
#define NO_SCROLL		((int) 1)
#define AUTOSCROLL		((int) 1)
#define NO_AUTOSCROLL	((int) 0)
#define NO_SEPARATORS	((int) 0)
#define WITH_SEPARATORS	((int) 1)
#define HIDE_HIDDEN_FILES		((int) 0)
#define SHOW_HIDDEN_FILES		((int) 1)
#define ACTION_OPEN				((int) 0)
#define ACTION_SAVE				((int) 1)
#define ACTION_OPEN_FOLDER		((int) 2)
#define ACTION_OPEN_FILE		((int) 3)
#define	SHOW_FILE_OP_BUTTONS	((int) 0)
#define	HIDE_FILE_OP_BUTTONS	((int) 1)

#define	ONE_BUTTON				((int) 1)
#define	ONE_BUTTON_ONE_CHECK	((int) 2)
#define	TWO_BUTTONS				((int) 3)
#define	TWO_BUTTONS_ONE_CHECK	((int) 4)
#define	THREE_BUTTONS			((int) 5)

#define	NOT_HOMOGENEUS	((int) 0)
#define HOMOGENEUS		((int) 1)

#define	NOT_FIRST_TIME	((int) 0)
#define FIRST_TIME		((int) 1)

#ifdef DEBUG
#define RESET(x)		err_msg(#x": %d",(x)=0)
#define SET(x) 			err_msg(#x": %d",(x)=1)
#define SET2(x,y) 		err_msg(#x": %d",(x)=(y))
#else
#define RESET(x) 		(x)=0
#define SET(x) 			(x)=1
#define SET2(x,y)		(x)=(y)
#endif

#define IS_SET(x)		(x)==1
#define IS_NOT_SET(x)	(x)==0

#define ACTIVITY		TRUE
#define PULSE			FALSE

#define SCREEN_MARGIN	80

# define ATTRIBUTE_UNUSED __attribute__ ((__unused__))

/* Backward-compatibility when gtk_widget_get_window() is not available (i.e.
   older than 2.14). In this case, direct access to the member is needed. */
#if !defined(USE_NEW_GET_WINDOW)
#define gtk_widget_get_window(widget) ((widget)->window)
#endif

/* Backward-compatibility with GTK_WIDGET_VISIBLE() (deprecated since 2.20)
   gtk_widget_get_visible() is available since 2.18. */
#if !defined(USE_NEW_GET_VISIBLE)
#define gtk_widget_get_visible(widget) GTK_WIDGET_VISIBLE(widget)
#endif

/* Backward-compatibility with GDK_<keyname> macros (deprecated since 2.22).
   In case the header <gdk/gdkkeysyms-compat.h> is not available. */
#if (!defined(GDK_KEY_w) && defined(GDK_w))
#define GDK_KEY_w GDK_w
#endif
#if (!defined(GDK_KEY_F1) && defined(GDK_F1))
#define GDK_KEY_F1 GDK_F1
#endif
#if (!defined(GDK_KEY_F2) && defined(GDK_F2))
#define GDK_KEY_F2 GDK_F2
#endif
#if (!defined(GDK_KEY_F3) && defined(GDK_F3))
#define GDK_KEY_F3 GDK_F3
#endif
#if (!defined(GDK_KEY_F4) && defined(GDK_F4))
#define GDK_KEY_F4 GDK_F4
#endif
#if (!defined(GDK_KEY_F5) && defined(GDK_F5))
#define GDK_KEY_F5 GDK_F5
#endif
#if (!defined(GDK_KEY_F6) && defined(GDK_F6))
#define GDK_KEY_F6 GDK_F6
#endif
#if (!defined(GDK_KEY_F7) && defined(GDK_F7))
#define GDK_KEY_F7 GDK_F7
#endif
#if (!defined(GDK_KEY_F8) && defined(GDK_F8))
#define GDK_KEY_F8 GDK_F8
#endif

void find_toplevel_set_transient(GtkWidget *window);
void main_loop(GtkWidget **window);
void refresh_gui(void);
void attach_to_table(GtkWidget *table, GtkWidget *widget, int start_col, int end_col, int start_row, int end_row);

/********************* Text box widget ***************************************/
GtkWidget *text_box_create(char *text, int editable);
GtkWidget *default_text_box_create_on_table(char *text, int editable, GtkWidget *table,
								 int start_col, int end_col, int start_row, int end_row);
GtkWidget *text_box_create_on_table(char *text, int editable, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row,
					GtkAttachOptions xoptions,  GtkAttachOptions yoptions,  
					 guint xpadding, guint ypadding, int maxlen, int width);
#if 0  /* UNUSED */
void my_gtk_entry_set_text(GtkEntry *entry, char *data);
#endif
char *my_gtk_entry_get_text(GtkEntry *entry);
/********************* End of Text box widget ********************************/

/********************* Standard radio button widget **************************/

GtkWidget *radio_create(GtkWidget *last_of_group, char *label,
			 GCallback func, gpointer func_data, int active );

/****************End of Standard radio button widget *************************/

/********************* Radio button on table widget **************************/

GtkWidget *radio_create_on_table(GtkWidget *last_of_group, char *label, GCallback func, gpointer func_data,
										int active, GtkWidget *table, int start_col, int end_col,
										int start_row, int end_row );
#if 0  /* UNUSED */
GtkWidget *default_radio_create_on_table(GtkWidget *last_of_group, char *label, GCallback func,
										int active, GtkWidget *table, int start_col, int end_col,
										int start_row, int end_row );
#endif
/****************End of radio button on table widget *************************/

/********************* Check button widgets **************************/

GtkWidget *check_create(char *label, GCallback func, gpointer func_data, int active );
GtkWidget *check_create_on_table(char *label, GCallback func, gpointer func_data, int active,
			 GtkWidget *table, int start_col, int end_col, int start_row, int end_row );
#if 0  /* UNUSED */
GtkWidget *default_check_create_on_table(char *label, GCallback func, int active,
			 GtkWidget *table, int start_col, int end_col, int start_row, int end_row );
#endif
/***************End of Check button widgets **********************************/

/********************* Standard button widget ********************************/

GtkWidget *button_create(char *label, GCallback func, gpointer func_data );

/****************End of Standard button widget *******************************/

/********************* Button on table widget ********************************/

GtkWidget *button_create_on_table(char *label, GCallback func, gpointer func_data,
			 GtkWidget *table, int start_col, int end_col, int start_row, int end_row );

/****************End of button on table widget ******************************/

/********************* Button in container widget ***************************/

GtkWidget *button_create_in_container(char *label, GCallback func, gpointer func_data,
			GtkWidget *container);

/*************End of Button in container widget *****************************/

/********************* Buttonbox widget **************************************/
GtkWidget *button_box_create(int orientation, int layout, int border, int spacing);

GtkWidget *default_button_box_create(int orientation);

GtkWidget *button_box_create_on_table(int orientation, int layout,  int border, int spacing, GtkWidget *table,
										 int start_col, int end_col, int start_row, int end_row );
GtkWidget *default_button_box_create_on_table(	int orientation, GtkWidget *table,
										 		int start_col, int end_col,
												int start_row, int end_row );

/*************** End of Buttonbox widget ************************************/

/********************* Label on table widget ********************************/

GtkWidget *default_label_create_on_table(char *label_text,GtkWidget *table,
				int start_col, int end_col, int start_row, int end_row );
#if 0  /* UNUSED */
GtkWidget *label_create_on_table(char *label_text, GtkWidget * table,
					int start_col, int end_col, int start_row, int end_row,
					 GtkAttachOptions xoptions, GtkAttachOptions yoptions, 
					 guint xpadding, guint ypadding, gfloat xalign, gfloat yalign);
#endif
GtkWidget *label_create(char *label_text);

/***************End of Label on table widget ********************************/

/********************* Standard table widget ********************************/

GtkWidget *table_create(int cols, int rows , GtkWidget* window , int border, int homogeneus);
#if 0  /* UNUSED */
GtkWidget *table_create_on_table(int cols , int rows,  GtkWidget* table, int border, int homogeneus,
									int start_col, int end_col, int start_row, int end_row);
#endif
/********************* Standard table widget ********************************/

/********************* Standard window widget ********************************/

GtkWidget *window_create( int x, int y, int border, const char *title, GCallback func,
			void *func_data, int modal);

/********************* Standard window widget ********************************/

/********************* Frame on table widget ********************************/

GtkWidget *frame_create_on_table(char *label, GtkWidget *table,
											 int start_col,int end_col, int start_row, int end_row );
GtkWidget *frame_create(char *label);

/**************End of Frame on table widget ********************************/

/********************* Dialog window widget **********************************/
typedef	struct	Dialog_data			Dialog_data;
struct Dialog_data {
	GtkWidget	*window;
	int *ptr_dialog_retval;
};

int  dialog_window(const char *title, int type, char *third_button_label,
					int def_button, const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));
void about_dialog(void);
void error_dialog(const char *message);
#if 0  /* UNUSED */
void not_implemented(void);
#endif
/**************End of Dialog window widget **********************************/

/********************* For the file selection widget *************************/

char *file_chooser(const char *title, GtkFileChooserAction fs_action, gboolean local_only, gboolean show_hidden,
					const char *filename, const char *folder, const char *shortcut_folder);
char *default_file_chooser(const char *title, GtkFileChooserAction fs_action);
char *my_file_selector_create(char *label, int hidden_files, int action, char *path, char **filter_str);
/*************End of the file selection widget********************************/

/************** Create a separator on table   ********************************/
#if 0  /* UNUSED */
GtkWidget *separator_create_on_table(int direction, GtkWidget *table,
									 int start_col,int end_col, int start_row, int end_row );

/**********End of  Create a separator on table********************************/

/************** Create a Multi column scrolled list ************************/

typedef struct StringListPopupMenuEntry {
	gpointer label;						/* Menu Label */
	int id;								/* Menu id */
	GCallback callbackf;				/* Callback */
} StringListPopupMenuEntry;

gboolean view_onButtonPressed(GtkWidget *treeview, GdkEventButton *event, gpointer userdata);
void string_list_popupmenu(GtkWidget *list, gpointer popupmenu);
void clear_list(GtkWidget *list);
GtkWidget *string_list_create(int num,GCallback func, int show_hide, int editable,...);
GtkWidget *string_list_create_on_table(int num,GCallback func,
										GtkWidget *table, int start_col, int end_col,
										int start_row, int end_row ,int show_hide, int editable, ...);
void add_data_to_list(GtkWidget * tree, char *data_string[], int n_columns, int autoscroll);
/******** End of Create a Multi column scrolled list ************************/

/************** Create a statusbar **********************************/

GtkWidget * statusbar_create_on_table(GtkWidget *table, GtkWidget **label, char* labeltext,
								int start_col, int end_col, int start_row, int end_row );

/******* End of Create a statusbar **********************************/

/************** Create a Permissions window **********************************/
typedef	struct	Perm_data			Perm_data;
struct Perm_data {
	GtkWidget	*window;
	int *ptr_dialog_retval;
	GtkWidget	*user_entry;
	GtkWidget	*group_entry;
	char* user;
	char* group;
	long uid;
	long gid;
};

int PermissionWindow(char *filename);
#endif
/*****************End of Permissions window **********************************/

/************** Text window Common  functions ********************************/

FILE *open_file(char *filename );
int read_and_convert(char * temp_ptr, GtkTextBuffer *buffer);
void read_from_stream_and_display_text(GtkWidget *text, FILE *fd, unsigned long filesize, gdouble *progress);
void clear_the_buffer(GtkWidget *text, int *text_modified);

/************** Text window Common  functions ********************************/

/********************* Text file viewer Widget *******************************/

GtkWidget *textpad_create(	int editable, GCallback func, void *func_arg, int wrap);
GtkWidget *textpad_create_on_table(	int editable, GCallback func, void *func_arg,
									int wrap, int scroll, GtkWidget *table,
									int start_col,int end_col, int start_row, int end_row );
void textview_window_create(char * title, int x, int y, int border, char *filename, FILE *fd);

/**************End of Text file viewer Widget ********************************/

/************** Create a Text Edit window **********************************/

void textedit_window_create(char * title, int x, int y, int border, char *filename);

/************** End of a Text Edit window **********************************/

/************ Create a Main Menubar   **************************************/

typedef	struct	MenuEntry			MenuEntry;
struct MenuEntry {
	gpointer simple_entries;			/*GtkActionEntry*/
	gpointer toggle_entries;			/*GtkToggleActionEntry*/
	gpointer radio_entries;				/*GtkRadioActionEntry*/ 
	int		 num_entries;
	int		 active_radio_entry;		/* Number starting from 0, none = -1 */
	GCallback radio_entry_callback;		/* Radio entry callback */
};

GtkWidget * main_menu_create(GtkWidget *window, MenuEntry *menu_items, const gchar *info, gpointer data);

/************ Create a Main Menubar   **************************************/

/************** Create a Text Edit window **********************************/

int about_window_create(char * title,int x, int y, int border, char *labeltext, FILE *fd, int first_time);
int default_about_window(FILE *fd, int first_time);

/************** Create a Text Edit window **********************************/

/************** Create a Progressbar **********************************/

typedef struct _ProgressData {
	GtkWidget *window;
	GtkWidget *pbar;
	int timer;
	gboolean activity_mode;
	gdouble value;
	int alive;
} ProgressData;

void progress_destroy(ProgressData *pdata);
ProgressData *progressbar_create(int mode);
ProgressData *progressbar_create_on_table(int mode, GtkWidget *table,
											int start_col, int end_col, 
											int start_row, int end_row);
#if 0  /* UNUSED */
ProgressData * progressbar_window(char* title, int mode, int modal);

/************** Create a Progressbar **********************************/

/************** Create a File System Info Window **********************/

void df_window(void);

/************** Create a File System Info Window **********************/

/************** Create a Process Manager Window  **********************/

void ps_window(void);
void kill_pid(GtkWidget *widget);
void refresh_ps(GtkWidget *widget, GtkWidget *pslist);
void ps_main(GtkWidget *pslist);
void pslist_f(GtkWidget *tree);

/************** Create a Process Manager Window  **********************/

/************** Create a Image Viewer Window  *************************/

void ViewImageWindow(char *filename);

/************** Create a Image Viewer Window  *************************/

/************** Create a Pack box  ************************************/

GtkWidget *box_create( int orientation, int homogeneous, int spacing);
GtkWidget *default_box_create_on_table( int orientation, int homogeneous, GtkWidget * table,
							int spacing, int start_col, int end_col, int start_row, int end_row);
GtkWidget *box_create_on_table(int orientation, int homogeneous, GtkWidget * table,
				int spacing, int start_col, int end_col, int start_row, int end_row,
				GtkAttachOptions xoptions,  GtkAttachOptions yoptions,  
				 guint xpadding, guint ypadding);

/************** Create a Pack box  ************************************/

/************** Create a Calendar  ************************************/

char *Calendar_Window(char *title, int flag);

/************** Create a Calendar  ************************************/
#endif
/************** Misc stuff  *******************************************/

void setbusycursor(GtkWidget *widget, int flag);

/************** Misc stuff  *******************************************/
#endif /* __HAVE_MYGTK_  */

