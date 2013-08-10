/*
 * Basic file/folder selection widget
 * Copyright (C) 2008 - 2009  by Tito Ragusa <tito-wolit@tiscali.it>
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
 * Based on the Demos code in the gtk+-2.10.14 source:
 * Icon View/Icon View Basics
 *
 * The GtkIconView widget is used to display and manipulate icons.
 * It uses a GtkTreeModel for data storage, so the list store 
 * example might be helpful.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef DEBUG
#include <sys/time.h>
#endif
#include "mylib.h"
#include "mygtk.h"

#define ICON_SIZE			((int) 32)

enum
{
  COL_PATH,
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  COL_IS_DIRECTORY,
  NUM_COLS
};

static GdkPixbuf *file_pixbuf   = NULL;
static GdkPixbuf *folder_pixbuf = NULL;
static GdkPixbuf *link_pixbuf   = NULL;
static GdkPixbuf *link_folder_pixbuf   = NULL;
static GdkPixbuf *exec_pixbuf   = NULL;
static int show_hidden_files = 0;
static GtkWidget *my_chooser_button_label;
static GtkWidget *file_label;
static char *parent;
static GtkListStore *store;
static int filter_flag = 0;
static GtkFileFilter *extra_filter;
static 	GtkWidget *window;

/* Loads the images for the demo and returns whether the operation succeeded */
static void load_pixbufs(void)
{
	GtkIconTheme *theme;
	GtkIconInfo *icon_info;
	GdkPixbuf *src;
	int size = ICON_SIZE;
	theme = gtk_icon_theme_get_default();

	/* We try differnt icons in order of preference hoping to get at least one */

	/* Default file icon */
	if ((icon_info = gtk_icon_theme_lookup_icon(theme, GTK_STOCK_FILE, size, 0))) {
		debug_err_msg(GTK_STOCK_FILE);
		file_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else 	if ((icon_info = gtk_icon_theme_lookup_icon(theme, "gnome-fs-regular", size, 0))) {
		debug_err_msg("gnome-fs-regular");
		file_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else 	if ((icon_info = gtk_icon_theme_lookup_icon(theme, "document-new", size, 0))) {
		/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
		folder_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else {
		file_pixbuf = NULL;
		debug_err_msg("failed to load regular file icon");
	}

	/* Folder icon */
	/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
	if ((icon_info = gtk_icon_theme_lookup_icon(theme, "folder", size, 0))) {
		folder_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else 	if ((icon_info = gtk_icon_theme_lookup_icon(theme, "gnome-fs-directory", size, 0))) {
		folder_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else  if ((icon_info = gtk_icon_theme_lookup_icon(theme, GTK_STOCK_DIRECTORY, size, 0))) {
		folder_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info); 
	} else {
		folder_pixbuf = NULL;
		debug_err_msg("failed to load icon folder");
	}

	/* Overlay for symbolic links */
	/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
	if ((icon_info = gtk_icon_theme_lookup_icon(theme, "emblem-symbolic-link", size, 0))) {
		src = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
		if (file_pixbuf && src) {
			link_pixbuf = gdk_pixbuf_copy(file_pixbuf);
			gdk_pixbuf_composite(src, link_pixbuf, 0, 0, size/2 , size/2, 0.0, 0.0, 0.5, 0.5, GDK_INTERP_BILINEAR, 255);
			/*g_object_unref(src);*/
		} else {
			link_pixbuf = file_pixbuf;
			debug_err_msg("failed compositing icon emblem-symbolic-link and regular file icon");
		}
		if (folder_pixbuf && src) {
			link_folder_pixbuf = gdk_pixbuf_copy(folder_pixbuf);
			gdk_pixbuf_composite(src, link_folder_pixbuf,
					0, 0, size/2 , size/2, 0.0, 0.0, 0.5, 0.5, GDK_INTERP_BILINEAR, 255);
			/*g_object_unref(src);*/
		} else {
			link_folder_pixbuf = folder_pixbuf;
			debug_err_msg("failed compositing icon emblem-symbolic-link and regular folder icon");
		}
		if (src) {
			g_object_unref(src);
		}
	} else {
		link_pixbuf = file_pixbuf;
		debug_err_msg("failed to load link icon");
	}

	/* Executable icon */
	/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
	if ((icon_info = gtk_icon_theme_lookup_icon(theme,  "application-x-executable", size, 0))) {
		exec_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else if ((icon_info = gtk_icon_theme_lookup_icon(theme,  "gnome-fs-executable", size, 0))) {
		exec_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else 	if ((icon_info = gtk_icon_theme_lookup_icon(theme, GTK_STOCK_EXECUTE, size, 0))) {
		/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
		folder_pixbuf = gdk_pixbuf_new_from_file_at_size(gtk_icon_info_get_filename(icon_info), size, size, NULL);
		gtk_icon_info_free(icon_info);
	} else {
		exec_pixbuf = NULL;
		debug_err_msg("failed to load executable icon");
	}
}

static int filesort_func(const void *a, const void *b)
{
	int ret;
	const char *name_a = g_build_filename(parent, (*(const struct dirent **) a)->d_name, NULL);
	const char *name_b = g_build_filename(parent, (*(const struct dirent **) b)->d_name, NULL);
	gboolean is_dir_a = g_file_test(name_a, G_FILE_TEST_IS_DIR);
	gboolean is_dir_b = g_file_test(name_b, G_FILE_TEST_IS_DIR);

	/* We need this function because we want to sort
	* folders before files.
	*/
	if (!is_dir_a && is_dir_b) {
		ret = 1;
	} else if (is_dir_a && !is_dir_b) {
		ret = -1;
	} else {
		/* This are not UTF-8 encoded, strcoll is faster than strcmp on my system */
		ret = strcoll((*(const struct dirent **) a)->d_name, (*(const struct dirent **) b)->d_name);
	}

	xfree(&name_a);
	xfree(&name_b);

	return ret;
}

static int compare_func(const struct dirent *d)
{
	if (d->d_name[0] == '/' && !d->d_name[1])
		return 0;
	if (d->d_name[0] == '.') {
		if(!d->d_name[1])
			return 0;
		if (d->d_name[1] == '.' && !d->d_name[2])
			return 0;
	}
	return 1;
}

static gint sort_func(GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b ,
							 __attribute__ ((__unused__)) gpointer user_data)
{
	gboolean is_dir_a, is_dir_b;
	gchar *name_a, *name_b;
	int ret;

	/* We need this function because we want to sort
	* folders before files.
	*/
	gtk_tree_model_get(model, a,
				COL_IS_DIRECTORY, &is_dir_a,
				COL_DISPLAY_NAME, &name_a,
				-1);
	
	gtk_tree_model_get(model, b,
				COL_IS_DIRECTORY, &is_dir_b,
				COL_DISPLAY_NAME, &name_b,
				-1);
	
	if (!is_dir_a && is_dir_b)
		ret = 1;
	else if (is_dir_a && !is_dir_b)
		ret = -1;
	else
		ret = g_utf8_collate(name_a, name_b);
	
	xfree(&name_a);
	xfree(&name_b);
	
	return ret;
}

static gint dummy_sort_func( __attribute__ ((__unused__)) GtkTreeModel *model,
							 __attribute__ ((__unused__)) GtkTreeIter  *a,
							 __attribute__ ((__unused__)) GtkTreeIter  *b ,
							 __attribute__ ((__unused__)) gpointer user_data)
{
	/* Does nothing, used just to fool our list store */
	return FALSE;
}

/* We need to define _BSD_SOURCE for scandir so instead
 * we use this implementation from uClibc-0.9.30-rc1
 */

/* These macros extract size information from a `struct dirent *'.
   They may evaluate their argument multiple times, so it must not
   have side effects.  Each of these may involve a relatively costly
   call to `strlen' on some systems, so these values should be cached.

   _D_EXACT_NAMLEN (DP)	returns the length of DP->d_name, not including
   its terminating null character.

   _D_ALLOC_NAMLEN (DP)	returns a size at least (_D_EXACT_NAMLEN (DP) + 1);
   that is, the allocation size needed to hold the DP->d_name string.
   Use this macro when you don't need the exact length, just an upper bound.
   This macro is less likely to require calling `strlen' than _D_EXACT_NAMLEN.
   */

/* Needed by Opensolaris, is slower when loading big directories */

#ifndef _D_ALLOC_NAMLEN
#define _D_ALLOC_NAMLEN(d) (strlen((d)->d_name) + 1)
#endif

int my_scandir(const char *dir, struct dirent ***namelist,
	int (*selector) (const struct dirent *),
	int (*compar) (const void *, const void *))
{
	DIR *dp = opendir (dir);
	struct dirent *current;
	struct dirent **names = NULL;
	size_t names_size = 0, pos;
	int save;

	if (dp == NULL)
	return -1;

	save = errno;
	errno = 0;

	pos = 0;
	while ((current = readdir (dp)) != NULL) {
		if (selector == NULL || (*selector) (current)) {
			struct dirent *vnew;
			size_t dsize;
	
			/* Ignore errors from selector or readdir */
			errno = 0;
	
			if (pos == names_size) {
				struct dirent **new;
				if (names_size == 0)
					names_size = 10;
				else
					names_size *= 2;
				new = (struct dirent **) realloc (names, names_size * sizeof (struct dirent *));
				if (new == NULL)
					break;
				names = new;
			}
	
			dsize = &current->d_name[_D_ALLOC_NAMLEN (current)] - (char *) current;
			vnew = (struct dirent *) malloc (dsize);
			if (vnew == NULL)
				break;
	
			names[pos++] = (struct dirent *) memcpy (vnew, current, dsize);
		}
	}

	if (errno != 0) {
		save = errno;
		closedir (dp);
		while (pos > 0)
			free (names[--pos]);
		free (names);
		errno = save;
		return -1;
	}

	closedir (dp);
	errno  = save;

	/* Sort the list if we have a comparison function to sort with.  */
	if (compar != NULL) {
		qsort (names, pos, sizeof (struct dirent *), compar);
	}
	*namelist = names;
	return pos;
}

#ifdef DEBUG
static unsigned long long monotonic_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000ULL + tv.tv_usec;
}
#endif

static void fill_store(int clear_file_label)
{
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	char *path;
	char *display_name;
	gboolean is_dir;
	GtkFileFilterInfo filter_info;
	int i;
	int n;
	struct dirent  **namelist;
#ifdef DEBUG
	long long t1;
	long long t2;
#endif
	/* First clear the store */
	gtk_list_store_clear(store);
	/* First clear the file name box, not needed if show/hide hidden files event */
	if (clear_file_label == TRUE) {
		if (GTK_IS_ENTRY(file_label)) {
			gtk_entry_set_text(GTK_ENTRY(file_label), "");
		} else 	if (GTK_IS_LABEL(file_label)) {
			gtk_label_set_text(GTK_LABEL(file_label), "");
		}
	}
	
#ifdef DEBUG
	t1 = monotonic_us();
#endif
	/* scandir is faster than g_read_dir, and does faster sorting than the iconview */
	if ((n = my_scandir(parent, &namelist, compare_func,  filesort_func)) < 0)
	//if ((n = scandir(parent, &namelist, compare_func,  NULL)) < 0)
		return;

	setbusycursor(window, TRUE);

	/* Disable the default sort func because it is slow */
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), dummy_sort_func, NULL, NULL);

	debug_err_msg("fill_store %s", parent);

	/* Fill the Combo box */
	gtk_label_set_text(GTK_LABEL(my_chooser_button_label), parent);

	for( i = 0; i < n; i++ ) {
		debug_err_msg("%s", namelist[i]->d_name);
		/* Show/Hide hidden files */
		if (!show_hidden_files && namelist[i]->d_name[0] == '.') {
			xfree(&namelist[i]);
			continue;
		}

		/* UTF-8 encoded path for GTK internal use */
		display_name = g_filename_to_utf8(namelist[i]->d_name, -1, NULL, NULL, NULL);

		if (filter_flag) {
			filter_info.contains = GTK_FILE_FILTER_FILENAME|GTK_FILE_FILTER_DISPLAY_NAME;
			filter_info.filename = namelist[i]->d_name;
			filter_info.display_name = display_name;
			filter_info.uri = NULL;
			filter_info.mime_type = NULL;
			if (gtk_file_filter_filter(extra_filter, &filter_info) == FALSE) {
				xfree(&display_name);
				xfree(&namelist[i]);
				continue;
			}
		}

		/* Local encoded path */
		path = g_build_filename(parent, namelist[i]->d_name, NULL);
		/* Default */
		pixbuf = file_pixbuf;

		if ((is_dir = g_file_test(path, G_FILE_TEST_IS_DIR))) {
				pixbuf = folder_pixbuf;
			if (g_file_test(path, G_FILE_TEST_IS_SYMLINK) == TRUE)
				pixbuf = link_folder_pixbuf;
		} else if (g_file_test(path, G_FILE_TEST_IS_SYMLINK) == TRUE)
				pixbuf = link_pixbuf;
		else if (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE) == TRUE)
				pixbuf = exec_pixbuf;

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
						COL_PATH, path,
						COL_DISPLAY_NAME, display_name,
						COL_IS_DIRECTORY, is_dir,
						COL_PIXBUF, pixbuf,
						-1);
		//xfree(&namelist[i]);
		xfree(&path);
		xfree(&display_name);
	}
	//xfree(&namelist);
	setbusycursor(window, FALSE);
	/* Enable the default sort func in case we add a new folder later */
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), sort_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),
						GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
						GTK_SORT_ASCENDING);
#ifdef DEBUG
	t2 = monotonic_us();
	printf("used usec %ld\n", t2 - t1);
#endif
}

static void entry_activate(GtkEntry *entry)
{
	const char *path = gtk_entry_get_text(entry);
	char *full_path;

	if (path) {
		full_path = g_build_filename(parent, path, NULL);
		if (mkdir(full_path, 0700))
			error_dialog(_("Cannot create directory!"));
		xfree(&full_path);
	}
	gtk_main_quit();
	fill_store(FALSE);
}

static void new_folder_f(GtkWidget *icon_view)
{
	gint x;
	gint margin;
	gint y;
	GtkWidget *entry;
	GtkWidget *dir_popup;
	GtkTreeIter iter;
	GdkDisplay* display;
	GdkWindow* parent_win;
	GtkAllocation *allocation = g_new0(GtkAllocation, 1);

	debug_err_msg("new_folder_f");

	gtk_list_store_append(store, &iter);
	/* The sort algoritm will place this at the first place */
	gtk_list_store_set(store, &iter,
				COL_PATH, "",
				COL_DISPLAY_NAME, "",
				COL_IS_DIRECTORY, TRUE,
				COL_PIXBUF, folder_pixbuf,
				-1);
	/* Doesn't work with GTK_WINDOW_POPUP */
	dir_popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	entry = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
	g_signal_connect(G_OBJECT (entry), "activate", G_CALLBACK (entry_activate), NULL);
	gtk_container_add(GTK_CONTAINER(dir_popup), entry);
	/* Place the entry on the first icon */
	parent_win = gtk_widget_get_parent_window(GTK_WIDGET(icon_view));
	gdk_window_get_origin(parent_win, &x, &y);
	margin = gtk_icon_view_get_margin(GTK_ICON_VIEW(icon_view));
	gtk_widget_get_allocation(icon_view, allocation);
	x += allocation->x + margin;
	y += allocation->y + margin + ICON_SIZE;
	gtk_window_move(GTK_WINDOW(dir_popup), x, y);
	/* To emulate a popup, in the worst case the window will be decorated */
	gtk_window_set_decorated(GTK_WINDOW(dir_popup), FALSE);
	gtk_widget_show_all(dir_popup);
	/* Grab */	
	gdk_pointer_grab(parent_win, TRUE,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_POINTER_MOTION_MASK,
		NULL, NULL, gtk_get_current_event_time());
	gdk_keyboard_grab(parent_win, TRUE, gtk_get_current_event_time());

	main_loop(&dir_popup);

	/* Ungrab */
	display = gtk_widget_get_display(GTK_WIDGET(icon_view));
	gdk_display_pointer_ungrab(display, gtk_get_current_event_time());
	gdk_display_keyboard_ungrab(display, gtk_get_current_event_time());
	/* Reload contents: useful if Cancel was clicked and to alphasort a newly created  folder */
	fill_store(TRUE);
	g_free(allocation);
}

static void item_activated(GtkIconView *iconview, GtkTreePath *tree_path, gpointer user_data)
{
	gchar *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean is_dir;

	model = gtk_icon_view_get_model(iconview);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model),
				&iter, tree_path);
	gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
				COL_PATH, &path,
				COL_IS_DIRECTORY, &is_dir,
				-1);
	
	if (!is_dir) {
		xfree(&path);
		return;
	}
	/* Replace parent with path and re-fill the model*/
	xfree(&parent);
	parent = path;
	fill_store(TRUE);

	/* Sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(user_data), TRUE);
}

static void item_selected(GtkIconView *view, __attribute__ ((__unused__)) gpointer user_data)
{
	GtkTreeModel *model;
	GList *list;
	GtkTreeIter iter;
	char *display_name;
	gboolean is_dir;

	debug_err_msg("item_selected");
	model = gtk_icon_view_get_model(view);
	
	list = gtk_icon_view_get_selected_items(view);

	if (list == NULL) {
		return;
	}

	/* if (g_list_length(list) == 1) { */
		/* Single selection */
		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
						/* COL_PATH, &path, */
						COL_DISPLAY_NAME, &display_name,
						COL_IS_DIRECTORY, &is_dir,
						-1);
		if (!is_dir) {
			debug_err_msg("selected : %s", display_name);
			if (GTK_IS_ENTRY(file_label)) {
				gtk_entry_set_text(GTK_ENTRY(file_label), display_name);
			} else 	if (GTK_IS_LABEL(file_label)) {
				gtk_label_set_text(GTK_LABEL(file_label), display_name);
			}
		} else {
			/* It's a dir but we need to clean the file name box of stale leftovers */
			if (GTK_IS_ENTRY(file_label)) {
				gtk_entry_set_text(GTK_ENTRY(file_label), "");
			} else if (GTK_IS_LABEL(file_label))  {
				gtk_label_set_text(GTK_LABEL(file_label), "");
			}
		}
		xfree(&display_name);
		/* Don't free is_dir */
	/* } else { TODO Multiple selection } */

	/* Clean up */
	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}

static void up_clicked(gpointer data)
{
	gchar *dir_name;
	
	dir_name = g_path_get_dirname(parent);
	xfree(&parent);
	parent = dir_name;
	fill_store(TRUE);
	/* Maybe de-sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(data), strcmp(parent, "/") != 0);
}

static void root_clicked(gpointer data)
{
	xfree(&parent);
	parent = xstrdup("/");
	fill_store(TRUE);
	/* De-sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
}

static void home_clicked(gpointer data)
{
	const char *home;	

	home = g_get_home_dir();
	if (!home)
		return;

	xfree(&parent);
	parent = xstrdup(home);
	fill_store(TRUE);
	/* Sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
}

static void desktop_clicked(gpointer data)
{
	const char *home;

	home = g_get_home_dir();
	if (!home)
		return;
	xfree(&parent);
	parent = xstrdup(g_build_filename(home, "Desktop", NULL));
	fill_store(TRUE);
	/* Sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
}

static void clean_up(void)
{
	if (file_pixbuf)
		g_object_unref(file_pixbuf);
	file_pixbuf = NULL;

	if (folder_pixbuf)	
		g_object_unref(folder_pixbuf);
	folder_pixbuf = NULL;
	
	if (link_pixbuf)
		g_object_unref(link_pixbuf);
	file_pixbuf = NULL;
	
	if (exec_pixbuf)
		g_object_unref(exec_pixbuf);
	exec_pixbuf = NULL;
}

static void file_sel_ok(GtkWidget *widget, int * filesel_ptr_ok)
{
	DBG_FUNCTION_NAME;
	assert(widget!= NULL);

	SET2(*filesel_ptr_ok, 1);
	gtk_main_quit();
}

static void popup_chdir(gpointer user_data)
{
	xfree(&parent);
	debug_err_msg("popup_chdir: activated : %s", (char *)user_data);
	parent = g_filename_from_utf8(user_data, -1, NULL, NULL, NULL);
	fill_store(TRUE);
}

static void menu_set_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in,  gpointer user_data)
{
	int wx;
	int wy;

	GtkWidget *button = GTK_WIDGET(user_data);
	GtkAllocation *allocation = g_new0(GtkAllocation, 1);

	gtk_menu_set_screen(menu, gtk_widget_get_screen(button));
	gdk_window_get_origin(gtk_widget_get_parent_window(button), &wx, &wy);
	gtk_widget_get_allocation(button, allocation);
	gtk_widget_set_size_request(GTK_WIDGET(menu), allocation->width, -1);
	*x = wx + allocation->x;
	*y = wy + allocation->y + allocation->height;
	*push_in = FALSE;
	g_free(allocation);
}

static void set_up_button( __attribute__ ((__unused__)) GtkMenuItem *menuitem, gpointer user_data)
{
	/* Maybe de-sensitize the up button */
	gtk_widget_set_sensitive(GTK_WIDGET(user_data), strcmp(parent, "/") != 0);
}

static void popup_create(GtkWidget *widget, gpointer up_button)
{
	GtkWidget* menu;
	GtkWidget* menu_items;
	char *tmp;

	if (strcmp(parent, "/") != 0) {
		menu =  gtk_menu_new();
	
		/* Don't add parent to the list as it will be added to the combo button */
		tmp = xstrdup(parent);
	
		while (1) {
			/* Move back one '/' */
			*(strrchr(tmp, '/')) = '\0';
			if (*tmp) {
				debug_err_msg("menu %s", tmp);
				/* Create a new menu-item with a name... */
				menu_items = gtk_menu_item_new_with_label(tmp);
				gtk_menu_shell_append((GtkMenuShell *)(menu), menu_items);
				g_signal_connect_swapped(G_OBJECT (menu_items), "activate", G_CALLBACK (popup_chdir), xstrdup(tmp));
				g_signal_connect_after(G_OBJECT (menu_items), "activate", G_CALLBACK(set_up_button), up_button);
			} else
				break;
		}

		/* Create a new menu-item with a name '/' */
		debug_err_msg("menu /");
		menu_items = gtk_menu_item_new_with_label("/");
		/* ...and add it to the menu. */
		gtk_menu_shell_append((GtkMenuShell *)(menu), menu_items);
		g_signal_connect_swapped(G_OBJECT (menu_items), "activate", G_CALLBACK (popup_chdir), xstrdup("/"));
		g_signal_connect_after(G_OBJECT (menu_items), "activate", G_CALLBACK(set_up_button), up_button);
		/* Show the widget */
		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, menu_set_position, widget, 0, gtk_get_current_event_time());
		xfree(&tmp);
	}
}

static void hidden_clicked(void)
{
	show_hidden_files = !show_hidden_files;
	fill_store(FALSE);
}

static void pref_set_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in,  gpointer user_data)
{
	GtkRequisition requisition;
	GtkRequisition b_requisition;
	int wx;
	int wy;

	GtkWidget *button = GTK_WIDGET(user_data);
	GtkAllocation *allocation = g_new0(GtkAllocation, 1);

	gtk_menu_set_screen(menu, gtk_widget_get_screen(button));
	gdk_window_get_origin(gtk_widget_get_parent_window(button), &wx, &wy);
#ifdef USE_NEW_PREFERRED_SIZE
	gtk_widget_get_preferrred_size(GTK_WIDGET(menu), NULL, &requisition);
	gtk_widget_get_preferrred_size(GTK_WIDGET(button), NULL, &b_requisition);
#else
	gtk_widget_size_request(GTK_WIDGET(menu), &requisition);
	gtk_widget_size_request(GTK_WIDGET(button), &b_requisition);
#endif
	gtk_widget_get_allocation(button, allocation);
	*x = wx + allocation->x - requisition.width + b_requisition.width;
	*y = wy + allocation->y + allocation->height;
	*push_in = FALSE;
	g_free(allocation);
}

static void pref_create(GtkWidget *widget)
{
	GtkWidget* menu;
	GtkWidget* menu_items;

	menu =  gtk_menu_new();
	menu_items = gtk_check_menu_item_new_with_mnemonic(_("Show _Hidden Files"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_items), show_hidden_files);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_items);
	g_signal_connect_swapped(G_OBJECT (menu_items), "toggled", G_CALLBACK (hidden_clicked), NULL);
	/* Show the widget */
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, pref_set_position, widget, 0, gtk_get_current_event_time());
}

static GtkWidget *my_combo_path_button(char *label_str, GCallback func, gpointer up_button)
{

	GtkWidget *button;
	GtkWidget *box;
	GtkWidget *image;
	GtkWidget *sep;

	button = gtk_button_new();
	g_signal_connect(button, "clicked", G_CALLBACK(func), up_button);
	box = gtk_hbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(button), box);
	
	image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

#ifdef USE_NEW_SEPARATOR
	sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
	sep = gtk_vseparator_new();
#endif
	gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 0);
	
	my_chooser_button_label = gtk_label_new(label_str);
	gtk_label_set_ellipsize(GTK_LABEL(my_chooser_button_label), PANGO_ELLIPSIZE_START);
	gtk_misc_set_alignment(GTK_MISC(my_chooser_button_label), 0.0, 0.5);
	gtk_container_add(GTK_CONTAINER(box), my_chooser_button_label);

#ifdef USE_NEW_SEPARATOR
	sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
	sep = gtk_vseparator_new();
#endif
	gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 0);
	
	image = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
	
	return button;
}

#ifdef USE_NEW_COMBO_BOX_TEXT
static void change_filter(GtkComboBoxText *widget)
#else
static void change_filter(GtkComboBox *widget)
#endif
{
	debug_err_msg("change_filter: current %d new %d", filter_flag, !filter_flag);
#ifdef USE_NEW_COMBO_BOX_TEXT
	if (strcmp(gtk_combo_box_text_get_active_text(widget),  _("All files (*)")) == 0)
#else
	if (strcmp(gtk_combo_box_get_active_text(widget),  _("All files (*)")) == 0)
#endif
		filter_flag = 0;
	else
		filter_flag = 1;

	fill_store(FALSE);
}

char * my_file_selector_create(char *label, int hidden_files, int action, char *path , char **filter_str)
{
	GtkWidget *icon_view;
	GtkWidget *sw;
	GtkWidget *vbox;
	GtkWidget *bbox;
	GtkWidget *hbox0;
	GtkWidget *hbox1 = NULL;
	GtkWidget *hbox2;
	GtkWidget *frame;
	GtkWidget *image = NULL;
	GtkWidget *filter_combo;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
	GtkWidget *tool_bar;
	GtkWidget *path_button;
	GtkWidget *pref_button;
	GtkToolItem *up_button;
	GtkToolItem *root_button;
	GtkToolItem *home_button;
	GtkToolItem *desktop_button;
	GtkToolItem *folder_new_button = NULL;
	GtkToolItem *separator;
	char *tmp;
	GtkIconTheme *theme;

	int ok = 0;

	DBG_FUNCTION_NAME;

	theme = gtk_icon_theme_get_default();
	show_hidden_files = hidden_files;
	load_pixbufs();
	window = window_create(540, 400, 4, (label) ? label : (action == ACTION_SAVE) ?  _("Save") : _("Open"),
							G_CALLBACK(gtk_main_quit), NULL, MODAL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	/* Use path passed by the caller or / if NULL */
	parent = xstrdup((path) ? path : "/");
	/* Main container */
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	/* Container for toolbar and menubar */
	hbox0 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox0, FALSE, FALSE, 0);
	tool_bar = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(hbox0), tool_bar, TRUE, TRUE, 0);

	/* Up button */
	/* Don't use gtk_tool_button_new_from_stock as we don't want a stock label */
	up_button = gtk_tool_button_new(NULL, NULL);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(up_button), GTK_STOCK_GO_UP);
	gtk_tool_item_set_is_important(up_button, TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(up_button), (strcmp(parent, "/") != 0));
	gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), up_button, -1);

	/* Root button */
	root_button = gtk_tool_button_new(NULL, NULL);
	if (gtk_icon_theme_has_icon(theme, "computer")) {
	/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(root_button), "computer");
	} else if (gtk_icon_theme_has_icon(theme, "gnome-fs-client")) {
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(root_button), "gnome-fs-client");
	} else {
		debug_err_msg("failed to load root button icon");
	}
	gtk_tool_item_set_is_important(root_button, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), root_button, -1);

	/* Home button */
	/* Don't use gtk_tool_button_new_from_stock as we don't want a stock label */
	home_button = gtk_tool_button_new(NULL, NULL);
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(home_button), GTK_STOCK_HOME);
	gtk_tool_item_set_is_important(home_button, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), home_button, -1);

	/* Desktop button */
	desktop_button = gtk_tool_button_new(NULL, NULL);
	if (gtk_icon_theme_has_icon(theme, "user-desktop")) {
		/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(desktop_button), "user-desktop");
	} else if (gtk_icon_theme_has_icon(theme, "gnome-fs-desktop")) {
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(desktop_button), "gnome-fs-desktop");
	} else {
		debug_err_msg("failed to load Desktop button icon");
	}
	gtk_tool_item_set_is_important(desktop_button, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), desktop_button, -1);

	if (action == ACTION_SAVE) {
		separator = gtk_separator_tool_item_new();
		gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), TRUE);
		gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), separator, -1);
		/* Folder-New button */
		/* Don't use gtk_tool_button_new_from_stock as we don't want a stock label */
		folder_new_button = gtk_tool_button_new(NULL, NULL);
		if (gtk_icon_theme_has_icon(theme, "folder-new")) {
			/* As per http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
			gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(folder_new_button), "folder-new");
		} else 	if (gtk_icon_theme_has_icon(theme, "folder")) {
			gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(folder_new_button), "folder");
		} else 	if (gtk_icon_theme_has_icon(theme, "gnome-fs-directory")) {
			gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(folder_new_button), "gnome-fs-directory");
		} else 	if (gtk_icon_theme_has_icon(theme, GTK_STOCK_DIRECTORY)) {
			gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(folder_new_button), GTK_STOCK_DIRECTORY);
		} else {
			debug_err_msg("failed to load New-folder button icon");
		}
		gtk_tool_item_set_is_important(folder_new_button, TRUE);
		gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), folder_new_button, -1);
	}

	separator = gtk_separator_tool_item_new();
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
	gtk_tool_item_set_expand(separator, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(tool_bar), separator, -1);

	/* Option menu */
	pref_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(pref_button), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
	gtk_button_set_relief(GTK_BUTTON(pref_button), GTK_RELIEF_NONE);
	gtk_box_pack_end(GTK_BOX(hbox0), pref_button, FALSE, FALSE, 0);

	/* Create the store  */
	store = gtk_list_store_new(NUM_COLS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					GDK_TYPE_PIXBUF,
					G_TYPE_BOOLEAN);
	/* Set sort column and function */ 
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store),
						sort_func,
						NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),
						GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
						GTK_SORT_ASCENDING);
	/* Path Combo Box */
	path_button = my_combo_path_button(parent, G_CALLBACK(popup_create), up_button);
	gtk_box_pack_start(GTK_BOX(vbox), path_button, FALSE, FALSE, 0);

	/* Scrolling window */
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
						GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

	/* File path box */
	if (action != ACTION_OPEN_FOLDER) {
		hbox1 = gtk_hbox_new(FALSE, 4);
		gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
		image = gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
		gtk_box_pack_start(GTK_BOX(hbox1), image, FALSE, FALSE, 0);
	}
	/* Editable only if action is ACTION_SAVE */
	if (action == ACTION_SAVE) { 
		file_label = text_box_create("", EDITABLE);
		gtk_box_pack_start(GTK_BOX(hbox1), file_label, TRUE, TRUE, 0);
	} else if (action == ACTION_OPEN || action == ACTION_OPEN_FILE) {
		/* ACTION_OPEN */
		/* To align icon to frame, at least on my box.... */
		gtk_misc_set_alignment(GTK_MISC(image), 0.0, 0.8);
		frame = frame_create("");
		/* With some themes this does nothing */
		gtk_frame_set_shadow_type(GTK_FRAME(frame),  GTK_SHADOW_ETCHED_IN);
		gtk_container_set_border_width(GTK_CONTAINER(frame), 0);
		file_label = label_create("");
		gtk_label_set_ellipsize(GTK_LABEL(file_label), PANGO_ELLIPSIZE_START);
		gtk_misc_set_alignment(GTK_MISC(file_label), 0.0, 0.5);
		gtk_container_add(GTK_CONTAINER(frame), file_label);
		gtk_box_pack_start(GTK_BOX(hbox1), frame, TRUE, TRUE, 0);
	}

	/* Create the Icon view */
	icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(icon_view), ICON_SIZE * 3);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_SINGLE);
	/* We now set which model columns that correspond to the text and pixbuf of each item */
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(icon_view), COL_DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), COL_PIXBUF);
	gtk_container_add(GTK_CONTAINER(sw), icon_view);
	gtk_widget_grab_focus(icon_view);

	if (action == ACTION_SAVE) {
		g_signal_connect_swapped(folder_new_button, "clicked", G_CALLBACK(new_folder_f), icon_view);
	}

	/* Add the file filter combo with action open */
	if (action == ACTION_OPEN || action == ACTION_OPEN_FILE) {
		hbox2 = gtk_hbox_new(FALSE, 4);
		gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
		image = gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU);
		gtk_box_pack_start(GTK_BOX(hbox2), image, FALSE, FALSE, 0);
#ifdef USE_NEW_COMBO_BOX_TEXT
		filter_combo = gtk_combo_box_text_new();
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(filter_combo), _("All files (*)"));
#else
		filter_combo = gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(filter_combo), _("All files (*)"));
#endif
		gtk_combo_box_set_active(GTK_COMBO_BOX(filter_combo), 0);
		gtk_box_pack_start(GTK_BOX(hbox2), filter_combo, TRUE, TRUE, 0);
		/* Filter is a NULL terminated array of strings */
		/* The first string is the label to be displayed by the combo button */
		if (filter_str) {
			/* Add the file filter */
			extra_filter = gtk_file_filter_new ();
			/* Add first string to the combo as label */
#ifdef USE_NEW_COMBO_BOX_TEXT
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(filter_combo), *filter_str);
#else
			gtk_combo_box_append_text(GTK_COMBO_BOX(filter_combo), *filter_str);
#endif
			/* Move on */
			filter_str++;
			for (; *filter_str; filter_str++) {
				debug_err_msg("%s", *filter_str);
				gtk_file_filter_add_pattern (extra_filter, *filter_str);
			}
		} else {
			/* No Filter, set insensitive */
			gtk_widget_set_sensitive(filter_combo, FALSE);
		}
		/* select and switch global file filter between default and new file filter*/
		g_signal_connect(filter_combo, "changed", G_CALLBACK(change_filter), NULL);
	}

	/* Connect the signal handlers */
	g_signal_connect(icon_view, "destroy", G_CALLBACK(clean_up), NULL);
	g_signal_connect(icon_view, "item-activated", G_CALLBACK(item_activated), up_button);
	g_signal_connect(icon_view, "selection-changed", G_CALLBACK(item_selected), up_button);
	g_signal_connect_swapped(up_button, "clicked", G_CALLBACK(up_clicked), up_button);
	g_signal_connect_swapped(root_button, "clicked", G_CALLBACK(root_clicked), up_button);
	g_signal_connect_swapped(home_button, "clicked", G_CALLBACK(home_clicked), up_button);
	g_signal_connect_swapped(desktop_button, "clicked", G_CALLBACK(desktop_clicked), up_button);
	g_signal_connect(pref_button, "clicked", G_CALLBACK(pref_create), NULL);
	
	g_object_unref(store);

	/* OK and Cancel buttons */
	bbox = default_button_box_create(HORIZONTAL);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
	cancel_button = button_create_in_container(GTK_STOCK_CANCEL, G_CALLBACK(gtk_main_quit), &window, bbox);
	ok_button = button_create_in_container(GTK_STOCK_OK, G_CALLBACK(file_sel_ok), &ok, bbox);

	/* Show all */
	gtk_widget_show_all(window);

	/* Fill the store, clearing file_label not needed */
	fill_store(FALSE);

	/* Main loop */
	gtk_main();

	/* Reuse parent var for UTF-8 */
	parent = g_filename_to_utf8(parent, -1, NULL, NULL, NULL);

	if (GTK_IS_ENTRY(file_label)) {
		tmp = xstrdup(gtk_entry_get_text(GTK_ENTRY(file_label)));
	} else if (GTK_IS_LABEL(file_label)) {
		tmp = xstrdup(gtk_label_get_text(GTK_LABEL(file_label)));
	} else {
		tmp = xstrdup("");
	}

	if ((GTK_IS_WIDGET(window))) {
		gtk_widget_destroy(window);
	}
	/* ok clicked: something was selected */
	if (ok) {
		if (action == ACTION_OPEN) {
			/* Files or folders: cannot fail */
			if (*tmp)
				return xconcat_path_file(parent, tmp);
			else 
				return parent;
		}
		if (action == ACTION_OPEN_FILE || action == ACTION_SAVE) {
			if (*tmp)
				return xconcat_path_file(parent, tmp);
			else 
				error_dialog(_("No file selected!"));
		}
		if (action == ACTION_OPEN_FOLDER) {
			/* Cannot fail */
			return parent;
		}
	}
	xfree(&tmp);
	xfree(&parent);
	return NULL;
}

