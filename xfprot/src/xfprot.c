/*
*
* XFPROT Copyright (C) 2002-2009 by Tito Ragusa <tito-wolit@tiscali.it>
*
* XFPROT is a graphical frontend for the F-Prot Antivirus
* for Linux Small Business Edition.
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
#include <fcntl.h>
#include <pwd.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <libintl.h>
#ifdef HAVE_PRCTL
#include <sys/prctl.h>
#endif
#include "gunixsignal.h"
#include "mygtk.h"
#include "mylib.h"

/* For checkboxes */
static int flag_nosub = 0;
static int flag_disinf = 0;
static int flag_rename = 0;
static int flag_delete = 0;
static int flag_append = 0;	
static int flag_list = 0;
static int flag_archives = 0;
static int flag_collect = 0;
static int flag_ai = 0;
static int flag_follow = 0;
static int flag_old = 0;
static int flag_mount = 0;
static int flag_boot = 0;
/* For radiobutton groups */
static int flag_scanmode = 0;
static int flag_file_types = 0;
static int flag_heur = 0;
static int flag_macro = 0;
static int flag_macro_action = 0;
/* Various flags */
static int have_version = 0;
static int saved_settings = 0;
static int aterm_terminal_hack = 0;
static int gnome_terminal_hack = 0;
static int page_cmd = 0;
/* Widgets */
static GtkWidget *main_window;
static GtkWidget *path_entry_box;
static GtkWidget *report_path_entry_box;
static GtkWidget *action_table;
static GtkWidget *disinfect_button;
static GtkWidget *rename_button;
static GtkWidget *delete_button;
static GtkWidget *macro_remove_table;
static GtkWidget *scan_1_button;
static GtkWidget *scan_3_button;
static GtkWidget *macro_1_button;
static GtkWidget *macro_2_button;
static GtkWidget *macro_3_button;
static GtkWidget *macro_4_button;
static GtkWidget *heur_1_button;
static GtkWidget *heur_2_button;
static GtkWidget *heur_3_button;
static GtkWidget *archives_button;
static GtkWidget *check_button1;
static GtkWidget *check_button3;
static GtkWidget *check_button5;
static GtkWidget *check_button6;
static GtkWidget *check_button7;
static GtkWidget *check_button8;
static GtkWidget *my_chooser_button_label;
/* Various strings */
static char *terminal_emulator = NULL;
static char *fprot_bin = NULL;
static char *fprot_dir = NULL;
static char *update_cmd = NULL;
/* Options common to f-prot 3.x.x and 4.x.x: used as default */
/* no leading space here*/
static char *info_cmd;
/* no leading space here*/
static char *virlist_cmd;
static char *follow_cmd;
static char *collect_cmd;
static char *list_cmd;
static char *nosub_cmd;

/*#################################### File Selection Widget wrappers  ##############################################*/

static void do_file_selection(void)
{
	char *home = (char *)g_get_home_dir();
	if (home)
		home = xconcat_path_file(home, "Desktop");
	else
		home = xstrdup("/");

	char *path = my_file_selector_create(_("Select path to scan:"), TRUE, ACTION_OPEN, home, NULL);

	if (path) {
		debug_err_msg("selected path %s", path);
		gtk_entry_set_text(GTK_ENTRY(path_entry_box), path);
		xfree(&path);
	}
	xfree(&home);
}

static void do_report_file_selection(void)
{
	DBG_FUNCTION_NAME;

	const char *current_report_file = gtk_label_get_text(GTK_LABEL(my_chooser_button_label));

	if (!current_report_file || !*current_report_file) {
		current_report_file = NULL;
	}

	char *path = file_chooser(_("Report file"), GTK_FILE_CHOOSER_ACTION_SAVE, FALSE, TRUE, current_report_file, NULL, NULL);

	if (path) {
		debug_err_msg("report: selected path %s", path);
		gtk_label_set_text(GTK_LABEL(my_chooser_button_label), path);
		xfree(&path);
	}
	/*else Gtk_entry unchanged */
}

/*############################### End of File Selection Widget wrappers #############################################*/


/*############################### Fprot-info, Virus-list, Update, Test, Report  & About button functions ############*/

static FILE *fprot_spawn(char *cmd)
{
	FILE *file;
	int fd_int;
	char *argv_p[3];

	DBG_FUNCTION_NAME;	
	
	argv_p[0] = fprot_bin;
	argv_p[1] = cmd;
	argv_p[2] = NULL;
	
	debug_err_msg("%s %s", argv_p[0], argv_p[1]);

	g_spawn_async_with_pipes (	NULL		/*working_directory*/,
								argv_p		/*argvp */,
								NULL		/*envp*/,
								0		/* flags*/,
								NULL		/* child_setup */,
								NULL		/* user_data */,
								NULL		/* child_pid */,
								NULL		/* standard_input*/,
								&fd_int 	/* standard_output */,
								NULL 	/* standard_error */,
								NULL		/* error*/);	
	return file = fdopen(fd_int, "r");
}

static void fprot_info(void)
{
	FILE *file;
	
	file = fprot_spawn(info_cmd);
	textview_window_create(_("Info"), 530, 430, 8, NULL, file);
	xfclose_nostdin(file);
}

static void fprot_virlist(void)
{
	FILE *file;

	file = fprot_spawn(virlist_cmd);
	textview_window_create(_("Virus list"), 530, 430, 8, NULL, file);
	xfclose_nostdin(file);
}

static void fprot_test(void)
{
	FILE *file;
	FILE *test_file = fopen("eicar.com", "w+");

	/* EICAR virus test file signature. Note that the compiled binary will
	   contain escaped '%%' so existing antivirus won't complain it (until
	   printed out). */
	fprintf(test_file, "X5O!P%%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*");
	xfclose_nostdin(test_file);
	file = fprot_spawn("eicar.com");
	textview_window_create(_("Test"), 530, 430, 8, NULL, file);
	xfclose_nostdin(file);
	unlink("eicar.com");
}

/* Copyright (C) 2006 Gabriel Somlo <somlo at cmu.edu> */
int execable_file(const char *name)
{
	struct stat s;

	return (!access(name, X_OK) && !stat(name, &s) && S_ISREG(s.st_mode));
}

/* Copyright (C) 2006 Gabriel Somlo <somlo at cmu.edu> - Modified */
char *find_execable(char *filename)
{
	char *path, *p, *n;

	p = path = xstrdup((getenv("PATH")) ? : "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin");
	debug_err_msg("path %s", path);
	while (p) {
		n = strchr(p, ':');
		if (n)
			*n++ = '\0';
		if (*p != '\0') { /* it's not a PATH="foo::bar" situation */
			p = xconcat_path_file(p, filename);
			if (execable_file(p)) {
				free(path);
				return p;
			}
			free(p);
		}
		p = n;
	}
	free(path);
	return NULL;
}

static void fprot_update(void)
{
	char *cmd;
	char *su_sudo_path = NULL;

	if (getuid() == 0) {
		/* I AM ROOT */
		cmd = xmake_message("%s %s%s", terminal_emulator, update_cmd, gnome_terminal_hack ? "\'" : "");
#ifdef PREFER_SUDO
	} else if ((su_sudo_path = find_execable("sudo"))) {
		/* Prefer sudo for updates if available */
		cmd = xmake_message("%s %s %s%s", terminal_emulator, su_sudo_path, update_cmd, gnome_terminal_hack ? "\'" : "");
		xfree(&su_sudo_path);
	} else if ((su_sudo_path = find_execable("su"))) {
		/* else try su */
		cmd = xmake_message("%s %s -c \"%s\"%s", terminal_emulator, su_sudo_path, update_cmd, gnome_terminal_hack ? "\'" : "");
		xfree(&su_sudo_path);
#else
	} else if ((su_sudo_path = find_execable("su"))) {
		/* Prefer su for updates if available */
		cmd = xmake_message("%s %s -c \"%s\"%s", terminal_emulator, su_sudo_path, update_cmd, gnome_terminal_hack ? "\'" : "");
		xfree(&su_sudo_path);
	} else if ((su_sudo_path = find_execable("sudo"))) {
		/* else try sudo */
		cmd = xmake_message("%s %s %s%s", terminal_emulator, su_sudo_path, update_cmd, gnome_terminal_hack ? "\'" : "");
		xfree(&su_sudo_path);
#endif
	} else {
		/* NOT ROOT */
		error_dialog(_("Cannot become superuser: sudo or su not found!"));
		return;
	}
	debug_err_msg("%s: %s", __FUNCTION__,  cmd);
	g_spawn_command_line_async(cmd, NULL);
	xfree(&cmd);
}

static void view_report(void)
{
	char *report =  g_filename_from_utf8(gtk_label_get_text(GTK_LABEL(my_chooser_button_label)),
									-1, NULL, NULL, NULL);
	textedit_window_create(report, 530, 450, 8, report);
	xfree(&report);
}

int show_about(int flag)
{
	FILE *file;
	int ret;

	file = open_file(LICENSE_FILE);
	ret = default_about_window(file, flag);
	xfclose_nostdin(file);
	return ret;
}

static void about_f(void)
{
	show_about(NOT_FIRST_TIME);
}


/*######################### END of Fprot-info, Virus-list, Update, Test, Report & About button functions ################*/

/*############################# START of Scan button function  ##########################################################*/

static void add_to_command(char **command, char *option)
{
	int size = xstrlen(option) + xstrlen(*command) + 1;
	*command = (char*)xrealloc(*command, size);
	xstrncat(*command, option, size);
	debug_err_msg("%s: %s", __FUNCTION__, option);
}

static void do_scan_new_f(void)
{
	char *scan_path = NULL;
	char *report_path = NULL;
	char *command = NULL;

	DBG_FUNCTION_NAME;

	scan_path = g_locale_from_utf8(gtk_entry_get_text(GTK_ENTRY(path_entry_box)), -1, NULL, NULL, NULL);
	/* We always "" quote the path to scan to avoid troubles with filenames with spaces */
	/* which are split by the shell and mess up our command line args */

	command = xmake_message("%s %s", terminal_emulator, "Xfprot_Scan");

	report_path =  g_filename_from_utf8(gtk_label_get_text(GTK_LABEL(my_chooser_button_label)),
									-1, NULL, NULL, NULL);
	/* I prefer to create always a report */
	/* We always "" quote the report file path to avoid troubles with filenames with spaces */
	/* which are split by the shell and mess up our command line args */
	if (report_path) {
		add_to_command(&command, " --output=");
		add_to_command(&command, "\"");	
		add_to_command(&command, report_path);
		add_to_command(&command, "\"");
	} else {
		error_dialog(_("Report file is not a regular file!"));
	}
	/* Let's parse the options*/
	if (flag_scanmode == 0 ) {
		/* Report only */
		add_to_command(&command," --report");
	} else if (flag_scanmode == 2) {
		/* Automatically disinfect if possible */
		add_to_command(&command," --disinfect");
	}

	if (IS_SET(flag_archives)) {
		/* Depth to scan inside nested archives. We use the maximum value here. */
		add_to_command(&command," --archive=99");
	} else {
		/* Don't scan inside archives. */
		add_to_command(&command," --archive=0");
	}
	
	if (IS_SET(flag_nosub))
		add_to_command(&command, nosub_cmd);

	if (IS_SET(flag_boot))
		add_to_command(&command, " --boot");

	if (IS_SET(flag_collect)) {
		/* --adware --applications */
		add_to_command(&command, collect_cmd);
		add_to_command(&command, " --scanlevel=3");
	} else {
		add_to_command(&command, " --scanlevel=2");
	}

	if (IS_SET(flag_list))
		add_to_command(&command, list_cmd);
	if (IS_SET(flag_follow))
		add_to_command(&command, follow_cmd);
	if (IS_SET(flag_mount))
		add_to_command(&command, " --mount");

	/* Twisted values for compatibility with f-prot 4.x.x logic */
	switch (flag_heur) {
		case 1:
			add_to_command(&command," --heurlevel=0");
			break;
		case 0:
			add_to_command(&command," --heurlevel=2");
			break;
		case 2:
			add_to_command(&command," --heurlevel=4");
			break;
	}

	/* In version 6.x.x we always scan for macros */
	flag_macro = 0;

	if (flag_scanmode > 0 /*&& flag_macro != 1*/) {
		switch (flag_macro_action) {
			/*case 0: is default */
			case 1:
				add_to_command(&command," --macros_safe");
				break;
			case 2:
				add_to_command(&command," --macros_new");
				break;
			case 3:
				add_to_command(&command," --stripallmacros");
				break;
		}
	}
	/* Add the path to scan */
	add_to_command(&command, " \"");
	add_to_command(&command, scan_path);
	add_to_command(&command,"\"");
	if (gnome_terminal_hack) {
		add_to_command(&command, "\'");
	}
	debug_err_msg("%s", command);
	g_spawn_command_line_async (command, NULL);
	xfree(&scan_path);
	xfree(&command);
}

int check_report_path(void)
{
	int ret = 0;
	char *report_path = NULL;
	struct	stat stat_p;

	DBG_FUNCTION_NAME;

	report_path = g_filename_from_utf8(gtk_label_get_text(GTK_LABEL(my_chooser_button_label)),
									-1, NULL, NULL, NULL);
	if (report_path) {
		if (lstat(report_path,&stat_p) == 0) {/* File exists */
			if (!S_ISREG(stat_p.st_mode)) { /* is not a regular file */
				error_dialog(_("Report file is not a regular file!"));
				ret = 1;
			} else { /* is a regular file */
				if (IS_NOT_SET(flag_append)) { /* Don't append */
					if (dialog_window(GTK_STOCK_DIALOG_WARNING, TWO_BUTTONS, NULL, 2, _("Report file exists. Overwrite?")) == 1) {
						/* Don't overwrite */
						ret = 1;
					}
					/*else overwrite existing report */
				}
				/*else append to existing report */
			}
		} /*else file don't exist so write it now */
		xfree(&report_path);
	}
	return ret;
}

static void scan_f(void)
{
	DBG_FUNCTION_NAME;

	if (check_report_path())
		return;
	do_scan_new_f();
}

/*############################## End of Scan button function  ######################################################*/

void remove_lock(void)
{
	DBG_FUNCTION_NAME;
	
	/* No lock file */
	if (access(LOCK_FILE, W_OK))
		return;

	if (remove(LOCK_FILE) == -1) {
		debug_err_msg("failed to remove lock file");
		error_dialog(_("Failed to remove lock file.\nAborting..."));
		exit(EXIT_FAILURE);
	}
	debug_err_msg("removed lock file");
}

static void create_file(char * filename)
{
	int pfd = open(filename ,O_RDWR|O_CREAT, 0600);

	if (pfd != -1)
		close(pfd);
}

static void root_warn(gid_t uid)
{
	int ret;
	struct stat stat_p;

	DBG_FUNCTION_NAME;

	if (uid != 0)
		return;

	debug_err_msg("you are root!");

	if (lstat(NO_ROOT_WARN_FILE,&stat_p) != 0) {
		ret = dialog_window(GTK_STOCK_DIALOG_WARNING, TWO_BUTTONS_ONE_CHECK, _("Don't show it again"),
								2, _("You are running xfprot as root!\nContinue?"));
		switch (ret) {
			case 4: /* No + checkbutton */
				create_file(NO_ROOT_WARN_FILE);
				/* FALLTRHOUGH */
			case 1: /* No */
				remove_lock();
				debug_err_msg("...exiting");
				exit(EXIT_SUCCESS);
			case 0: /* Ok */
				break;
			case 3: /* Ok + checkbutton */
				create_file(NO_ROOT_WARN_FILE);
		}
	}
}

static void do_lock_file(void)
{
	int pfd;
	int dialog_retval = -1;
	struct	stat stat_p;

	DBG_FUNCTION_NAME;
	
	/* Check if LOCK FILE exists and remove if stale */
	
	/* File exists */
	if (lstat(LOCK_FILE, &stat_p) == 0) {
		dialog_retval = dialog_window(GTK_STOCK_DIALOG_WARNING, TWO_BUTTONS, NULL, 2,
										_("xfprot has detected a lock file.\n"
										"Another instance of xfprot seems to be running.\n"
										"If you have determined that there is no other xfprot"
										" running\nplease click \"Yes\" to begin."));
		if (dialog_retval == 1)
				exit(EXIT_SUCCESS);
		remove_lock();
	}
	/* cannot open */
	if ((pfd = open(LOCK_FILE, O_RDWR|O_CREAT|O_EXCL, 0640)) < 0) {
		error_dialog(_("Cannot open lock file.\nAborting..."));
		exit(EXIT_FAILURE);
	}
	/* cannot lock */
	if (lockf(pfd, F_TLOCK, 0) < 0) {
		error_dialog(_("Cannot lock lock-file.\nAborting..."));
		exit(EXIT_FAILURE);
	}
	/* only first instance continues */
	close(pfd);
}

static void working_dir_setup_and_chdir(uid_t uid, gid_t gid)
{
	DBG_FUNCTION_NAME;

	debug_err_msg("setting permissions of private dir to 0700");
	if (xchmod(XFPROT_PRIV_DIR,0700, RETURN_ON_FAILURE) == -1) {
		error_dialog(_("Cannot set permissions\nof private dir to 0700.\nAborting..."));
		exit(EXIT_FAILURE);
	}

	debug_err_msg("setting owner of private dir");
	if (xchown(XFPROT_PRIV_DIR, uid, gid, RETURN_ON_FAILURE) == -1) {
		error_dialog(_("Cannot set owner\nof private dir.\nAborting..."));
		exit(EXIT_FAILURE);
	}

	debug_err_msg("chdir to private dir");
	if (xchdir(XFPROT_PRIV_DIR, RETURN_ON_FAILURE) == -1) {
		error_dialog(_("Cannot change dir\nto private dir.\nAborting..."));
		exit(EXIT_FAILURE);
	}
}

void toggle1_f(void)
{
	DBG_FUNCTION_NAME;

	debug_err_msg("flag_scanmode %d flag_macro %d\n", flag_scanmode, flag_macro);
	if (flag_scanmode > 0 && flag_macro < 2 ) {
		/* It's better to reset the flags to default values
			here to avoid leftovers from previous toggles */
		gtk_widget_show(action_table);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (disinfect_button),	TRUE);
		SET(flag_disinf);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rename_button),	FALSE);
		RESET(flag_rename);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (delete_button),	FALSE);
		RESET(flag_delete);
	} else {
		gtk_widget_hide(action_table);
		debug_err_msg("hiding widget action_table");
	}
}

static void toggle2_f(void)
{
	DBG_FUNCTION_NAME;

	debug_err_msg("flag_scanmode %d flag_macro %d", flag_scanmode, flag_macro);
	if ( flag_scanmode > 0 && flag_macro != 1 ) {
		/* It's better to reset the flags to default values
			here to avoid leftovers from previous toggles */
		gtk_widget_show(macro_remove_table);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (macro_1_button), TRUE);
		SET2(flag_macro_action, 0);
	} else {
		gtk_widget_hide(macro_remove_table);
		debug_err_msg("hiding widget macro_remove_table");
	}
}

static void new_flag_scanmode_callback(GtkWidget *widget, gpointer callback_data)
{
	assert(widget != NULL);

	SET2(flag_scanmode, atoi(callback_data));
	toggle2_f();
}

static void flag_macro_action_callback(GtkWidget *widget, gpointer callback_data)
{
	assert(widget != NULL);

	SET2(flag_macro_action, atoi(callback_data));
}

static void new_flag_heur_callback(GtkWidget *widget, gpointer callback_data)
{
	assert(widget != NULL);

	SET2(flag_heur, atoi(callback_data));
}

static void flag_callback(GtkWidget *widget, int *callback_data)
{
	assert(widget != NULL);

	if (IS_SET(*callback_data))
		RESET(*callback_data);
	else
		SET(*callback_data);	
}

/* This callback quits the program */
static void main_window_close(void)
{
	DBG_FUNCTION_NAME;

	gtk_main_quit ();
	if (GTK_IS_WIDGET(main_window))
		gtk_widget_destroy(main_window);
	remove_lock();
	debug_err_msg("...exiting");
}

void terminate(gint sig, gpointer data) __attribute__(( __noreturn__ ));
void terminate(gint sig, gpointer data)
{
	switch (sig) {
		case SIGINT:
			debug_err_msg("SIGINT signal catched");	
			break;
		case SIGTERM:
			debug_err_msg("SIGTERM signal catched");	
			break;
		case SIGHUP:
			debug_err_msg("SIGHUP signal catched");	
			break;
	}
	main_window_close();
	exit(EXIT_FAILURE);
	/* To shut a compiler warning */
	data = NULL;
}

static void save_settings_f(void)
{
	FILE *fd;
	mode_t  saved_mask = umask(077);

	if ((fd = fopen(XFPROT_CONFIG_FILE, "w")) != NULL) {
			fprintf(fd, "# %s - %s\n# Configuration file\n#\n\n", PACKAGE_STRING, COPYRIGHT); 
			fprintf(fd, "scanmode=%d\n",     flag_scanmode);
			fprintf(fd, "macro=%d\n",        flag_macro);
			fprintf(fd, "macro_action=%d\n", flag_macro_action);
			fprintf(fd, "file_types=%d\n",   flag_file_types);
			fprintf(fd, "heur=%d\n",         flag_heur);
			fprintf(fd, "nosub=%d\n",        flag_nosub);
			fprintf(fd, "disinf=%d\n",       flag_disinf);
			fprintf(fd, "rename=%d\n",       flag_rename); 
			fprintf(fd, "delete=%d\n",       flag_delete);
			fprintf(fd, "append=%d\n",       flag_append);
			fprintf(fd, "list=%d\n",         flag_list);
			fprintf(fd, "archives=%d\n",     flag_archives);
			fprintf(fd, "collect=%d\n",      flag_collect);
			fprintf(fd, "ai=%d\n",           flag_ai);
			fprintf(fd, "follow=%d\n",       flag_follow);
			fprintf(fd, "old=%d\n",          flag_old);
			fprintf(fd, "mount=%d\n", flag_mount);
			fprintf(fd, "boot=%d\n",  flag_boot);
			if (fflush(fd) || fsync(fileno(fd)) || fclose(fd)) {
				dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1,
								_("Error writing file '%s': %s"),
								XFPROT_CONFIG_FILE, strerror(errno));
			} else {
				dialog_window(GTK_STOCK_DIALOG_INFO, ONE_BUTTON, NULL, 1, _("Settings saved!!!"));
			}
	} else {
		dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1,
						_("Cannot open file '%s':\n%s"), XFPROT_CONFIG_FILE, strerror(errno));
	}
	umask(saved_mask);
}

static void clear_settings_f(void)
{
	DBG_FUNCTION_NAME;
	
	/* No lock file */
	if (access(XFPROT_CONFIG_FILE, W_OK) == 0) {
		if (remove(XFPROT_CONFIG_FILE) == -1) {
			debug_err_msg("failed to remove file: %s", XFPROT_CONFIG_FILE);
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, 
							_("Failed to remove '%s':\n%s"), XFPROT_CONFIG_FILE, strerror(errno));
			return;
		}
	}
	dialog_window(GTK_STOCK_DIALOG_INFO, ONE_BUTTON, NULL, 1, _("Done"));
	debug_err_msg("removed config file: %s/%s", XFPROT_PRIV_DIR, XFPROT_CONFIG_FILE);
}

int read_saved_settings(void)
{
	FILE *fd;
	int ret = 0;
	char *str;

	DBG_FUNCTION_NAME;

	/* Set default just for paranoia */
	flag_nosub = 0;
	flag_disinf= 0;
	flag_rename = 0;
	flag_delete = 0;
	flag_append = 0;
	flag_list = 0;
	flag_archives = 0;
	flag_ai = 0;
	flag_collect = 0;
	flag_follow = 0;
	flag_old = 0;
	flag_scanmode = 0;
	flag_macro = 0;
	flag_macro_action = 0;
	flag_heur = 0;
	flag_file_types = 0;
	flag_mount = 0;
	flag_boot = 0;

	if ((fd = fopen(XFPROT_CONFIG_FILE, "r")) != NULL) {
		debug_err_msg("reading config from: %s/%s", XFPROT_PRIV_DIR, XFPROT_CONFIG_FILE);
		/* Read the config file */
		while ((str = get_chomped_line_from_file(fd)) != NULL) {
			/* Trim leading and triling whitespace */
			trim(str);

			debug_err_msg("%s", str);

			/* Skip commented or empty lines */
			if (str[0] == '#' || str[0] == '\0')
				continue;

			/* Set the config values */
			if (strcmp("scanmode=0", str) == 0)
				flag_scanmode = 0;
			else if (strcmp("scanmode=1", str) == 0)
				flag_scanmode = 1;
			else if (strcmp("scanmode=2", str) == 0)
				flag_scanmode = 2;
			
			if (strcmp("macro=0", str) == 0)
				flag_macro =  0;
			else if (strcmp("macro=1", str) == 0)
				flag_macro = 1;
			else if (strcmp("macro=2", str) == 0)
				flag_macro = 2;

			if (strcmp("macro_action=0", str) == 0)
				flag_macro_action = 0;
			else if (strcmp("macro_action=1", str) == 0)
				flag_macro_action = 1;
			else if (strcmp("macro_action=2", str) == 0)
				flag_macro_action = 2;
			else if (strcmp("macro_action=3", str) == 0)
				flag_macro_action = 3;

			if (strcmp("heur=0", str) == 0)
				flag_heur = 0;
			else if (strcmp("heur=1", str) == 0)
				flag_heur = 1;
			else if (strcmp("heur=2", str) == 0)
				flag_heur = 2;

			if (strcmp("file_types=0", str) == 0)
				flag_file_types = 0;
			else if (strcmp("file_types=1", str) == 0)
				flag_file_types = 1;
			else if (strcmp("file_types=2", str) == 0)
				flag_file_types = 2;

			if (strcmp("nosub=1", str) == 0)
				flag_nosub = 1;

			if (strcmp("disinf=1", str) == 0)
				flag_disinf = 1;

			if (strcmp("rename=1", str) == 0)
				flag_rename = 1;

			if (strcmp("delete=1", str) == 0)
				flag_delete = 1;

			if (strcmp("append=1", str) == 0)
				flag_append = 1;

			if (strcmp("list=1", str) == 0)
				flag_list = 1;

			if  (strcmp("archives=1", str) == 0)
				flag_archives  = 1;

			if (strcmp("ai=1", str) == 0)
				flag_ai = 1;

			if (strcmp("collect=1", str) == 0)
				flag_collect = 1;

			if (strcmp("mount=1", str) == 0)
				flag_mount = 1;

			if (strcmp("boot=1", str) == 0)
				flag_boot = 1;

			if  (strcmp("follow=1", str) == 0)
				flag_follow = 1;

			if (strcmp("old=1", str) == 0)
				flag_old  = 1;
		}
		ret++;
	}
	debug_err_msg("restore saved_settings %d", ret);
	return ret;
}

/* TODO: ROXTERM can be forced to use a separate process for a new spawned instance,
 * by using the --separate option. Gert Hulselmans <hulselmansgert@gmail.com>
 */

/* List of terminal emulators known to work */
char *terms[] = {
#ifdef SOLARIS
	"gnome-terminal", "-e \'",          "2", /* Tested - needs quoting hack to work - works also on Opensolaris */
#else
	"konsole",        "--noclose -e",   "0", /* Tested */
	"gnome-terminal", "-e \'",          "2", /* Tested - needs quoting hack to work */
	"Terminal",       "-H -x",          "0", /* Tested */
	"lxterminal",     "-e \'",          "2", /* Tested - needs quoting hack to work */
	"roxterm",        "-e",             "0", /* Tested */
	"Eterm",          "--pause --exec", "0", /* Tested */
	"xterm",          "-hold -e",       "0", /* Tested */
	"uxterm",         "-hold -e",       "0", /*Not tested */
	"mrxvt",          "-hold -e",       "0", /*Not tested */
	"aterm",          "-e",             "1", /* Tested */
	"multi-aterm",    "-e",             "1", /*Not tested */
	"rxvt",           "-e",             "1", /* Tested */
#endif
	NULL,       NULL,             NULL
};

void autodetect_terminal_emulator(void)
{
	int i;
	char *path = NULL;

	terminal_emulator = NULL;

	for (i = 0; terms[i] != NULL; i += 3) {
		if ((path = find_execable(terms[i]))) {
			/* Now we support only f-prot version >= 6.2.x */
			if (terms[i + 2][0] == '1' /*&& have_version >= 620*/ && !page_cmd) {
				xfree(&path);
				continue;
			}
			terminal_emulator = xmake_message("%s %s", path, terms[i + 1]);
			xfree(&path);
			if (terms[i + 2][0] == '1') {
				aterm_terminal_hack = 1;
			}
			if (terms[i + 2][0] == '2') {
				gnome_terminal_hack = 1;
			}
			break;
		}
	}

	if (!terminal_emulator) {
		error_dialog(_("No suitable terminal emulator found"));
		remove_lock();
		exit(EXIT_FAILURE);
	}
	debug_err_msg("%s: aterm_terminal hack %sabled", terminal_emulator, aterm_terminal_hack ? "en" : "dis");
}

char *xmalloc_readlink(const char *path)
{
	enum { GROWBY = 80 }; /* how large we will grow strings by */

	char *buf = NULL;
	int bufsize = 0, readsize = 0;

	do {
		bufsize += GROWBY;
		buf = xrealloc(buf, bufsize);
		readsize = readlink(path, buf, bufsize);
		if (readsize == -1) {
			free(buf);
			return NULL;
		}
	} while (bufsize < readsize + 1);

	buf[readsize] = '\0';

	return buf;
}

void autodetect_fprot(void)
{
	char *tmp = NULL;

	fprot_bin = find_execable("fpscan");

	if (fprot_bin) {
		/* Find the real path in case it si a symlink */
		if ((tmp = xmalloc_readlink(fprot_bin))) {
			debug_err_msg("fprot_bin is a link %s real path %s", fprot_bin, tmp);
			xfree(&fprot_bin);
			fprot_bin = tmp;
		}
		fprot_dir = dirname(xstrdup(fprot_bin));
		debug_err_msg("fprot_bin %s fprot_dir %s", fprot_bin, fprot_dir);
	} 
	if (!fprot_bin) {
		error_dialog(_("F-prot binary not found"));
		remove_lock();
		exit(EXIT_FAILURE);
	}
}

static void autodetect_pager(void)
{
	/* the pager path is not inherited by the scripts
	 * they'll autodetect it by themselves
	 */
	if (find_execable("less")) {
		page_cmd = 1;
	} else if (find_execable("more")) {
		page_cmd = 1;
	} else {
		page_cmd = 0;
	}
}

static void detect_fprot_version(void)
{
	char *cmd = NULL;
	char tmp[64];
	FILE *fd;

	/* Detect f-prot version */
	/* Please note that the version of fpscan binary might be different 
	   from the f-prot release version. */
	if (strcmp(basename(fprot_bin), "f-prot") == 0) {
		cmd = xmake_message("%s -verno", fprot_bin);
		fd = xpopen(cmd, "r");
		while (fgets(tmp, sizeof(tmp), fd) != NULL) {
			/* Detect -verno switch f-prot >=3.13 */
			if (strstr(tmp,"F-PROT ANTIVIRUS") != NULL)
				SET2(have_version, 313); /* f-prot version 3.13 */
			/* Detect version */
			if (strstr(tmp,"Program version: 4.1") != NULL)
				SET2(have_version, 410); /* f-prot version 4.1.x */
			if (strstr(tmp,"Program version: 4.4") != NULL)
				SET2(have_version, 440); /* f-prot version 4.4.x */
			if (strstr(tmp,"Program version: 4.5") != NULL)
				SET2(have_version, 450); /* f-prot version 4.5.x */
			if (strstr(tmp,"Program version: 4.6") != NULL)
				SET2(have_version, 460); /* f-prot version 4.6.x */
		}
	} else {
		cmd = xmake_message("%s --version", fprot_bin);
		fd = xpopen(cmd, "r");
		while (fgets(tmp, sizeof(tmp), fd) != NULL) {
			/* fpscan version 6.2.x (f-prot version 6.0.x) */
			if (strstr(tmp,"F-PROT Antivirus version 6.2") != NULL)
				SET2(have_version, 620);
			/* fpscan version 6.3.x (f-prot version 6.0.3) */
			if (strstr(tmp,"F-PROT Antivirus version 6.3") != NULL)
				SET2(have_version, 630);
			/* Generic signature for fpscan 6.5+ (f-prot 6.1+) */
			if (strstr(tmp,"F-PROT Antivirus CLS version 6") != NULL)
				SET2(have_version, 650);
			/* fpscan version 6.5.x (f-prot version 6.1.x) */
			if (strstr(tmp,"F-PROT Antivirus CLS version 6.5") != NULL)
				SET2(have_version, 650);
			/* fpscan version 6.7.x (f-prot version 6.2.x) */
			if (strstr(tmp,"F-PROT Antivirus CLS version 6.7") != NULL)
				SET2(have_version, 670);
		}
	}
	xpclose_nostdin(fd);
	xfree(&cmd);
	/* Could not detect the version */
	if (have_version == 0) {
		error_dialog(_("Cannot detect f-prot's version.\nAborting..."));
		remove_lock();
		unixsignal_cleanup();
		exit(EXIT_FAILURE);
	}
}

static void setup_commands(void)
{
	switch (have_version) {
		case 313:
			/* Unsupported version: fallthrough */
		case 410:
			/* Unsupported version: fallthrough */
		case 440:
			/* Unsupported version: fallthrough */
		case 450:
			/* Unsupported version: fallthrough */
		case 460:
			/* Unsupported version */
			break;
		case 620:
			/* Fallthrough */
		case 630:
			/* Fallthrough */
		case 650:
			/* Fallthrough */
		case 670:
			update_cmd = xmake_message("Xfprot_Update %s/fpupdate", fprot_dir);
			/* no leading space here*/
			info_cmd     = "--version";
			virlist_cmd  = "--virlist";
			/* leading space here */
			follow_cmd   = " --follow";
			collect_cmd  = " --adware --applications";
			list_cmd     = " --verbose=2";
			nosub_cmd    = " --maxdepth=0";
			/* No page_cmd here, we will use the one set by autodetect_pager function. */
			break;
	}
}

static GtkWidget *my_gtk_file_chooser_button(char *label_str)
{
	GtkWidget *button;
	GtkWidget *box;
	GtkWidget *image;
	GtkWidget *sep;

	button = gtk_button_new();
	g_signal_connect (button, "clicked", G_CALLBACK(do_report_file_selection), button);

	box = gtk_hbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(button), box);
	
	image = gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);

#ifdef USE_NEW_SEPARATOR
	sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
	sep = gtk_vseparator_new();
#endif
	gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 0);
	
	my_chooser_button_label = gtk_label_new(label_str);
	gtk_label_set_ellipsize (GTK_LABEL(my_chooser_button_label), PANGO_ELLIPSIZE_START);
	gtk_misc_set_alignment (GTK_MISC(my_chooser_button_label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(box), my_chooser_button_label, TRUE, TRUE, 0);
	
#ifdef USE_NEW_SEPARATOR
	sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
	sep = gtk_vseparator_new();
#endif
	gtk_box_pack_start(GTK_BOX (box), sep, FALSE, FALSE, 0);
	
	image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX (box), image, FALSE, FALSE, 0);
	return button;
}

char * get_desktop(void)
{
	char *home = (char *)g_get_home_dir();
	if (!home)
		return xgetcwd(NULL);
	return xstrdup(g_build_filename(home, "Desktop", NULL));
}

GList *get_icon_list(void)
{
	GList *list = NULL;
	GdkPixbuf *pixbuf;
	char *path;
	int i = 0;
	char *icon_names[5] = {"xfprot-16x16.png", "xfprot-32x32.png",
			"xfprot-48x48.png", "xfprot-64x64.png", "xfprot-128x128.png"};

	for (i = 0; i < 5; i++) {
		path = g_build_filename(DATADIR, "pixmaps", icon_names[i], NULL);
		pixbuf = gdk_pixbuf_new_from_file(path, NULL);
		if (pixbuf) {
			list = g_list_append(list, pixbuf);
		}
		g_free(path);
	}
	return list;
}

static void new_gui(GList *icon_list)
{
	GtkWidget *top_table;
	GtkWidget *mode_table;
	GtkWidget *heur_table;
	GtkWidget *report_table;
	GtkWidget *option_table;
	GtkWidget *frame1;
	GtkWidget *frame2;
	GtkWidget *frame5;
	GtkWidget *frame6;
	GtkWidget *frame7;
	GtkWidget *table1;
	GtkWidget *button1;
	GtkWidget *button3;
	GtkWidget *button4;
	GtkWidget *button5;
	GtkWidget *button6;
	GtkWidget *button7;
	GtkWidget *button8;
	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *scan;
	GtkWidget *update;
	GtkWidget *report;
	GtkWidget *sep;
	GtkWidget *quit;
	GtkWidget *toolsmenu;
	GtkWidget *tools;
	GtkWidget *test;
	GtkWidget *info;
	GtkWidget *viruslist;
	GtkWidget *save_settings;
	GtkWidget *clear_settings;
	GtkWidget *help_menu;
	GtkWidget *help;
	GtkWidget *about;
	GtkAccelGroup *accel_group = NULL;
#if !defined(USE_NEW_TOOLTIP)
	GtkTooltips *action_tooltip;
	GtkTooltips *onlyheur_tooltip;
#endif
	GSList *scan_mode_group = NULL;
	GSList *remove_macro_group = NULL;
	GSList *heuristics_group = NULL;
	char *path = NULL;
	char *report_file_path = NULL;

	main_window = window_create(640, 450, 0, PACKAGE_STRING, G_CALLBACK(main_window_close), NULL, NOT_MODAL);
	gtk_window_set_icon_list(GTK_WINDOW(main_window), icon_list);
	gtk_window_set_default_icon_list(icon_list);

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(main_window), accel_group);

	table1 = table_create(10, 21, main_window, 2 , NOT_HOMOGENEUS);

	menubar = gtk_menu_bar_new();
	attach_to_table(table1, menubar, 0, 10, 0, 1);
	/* File Menu */
	filemenu = gtk_menu_new();
	/* File */
	file = gtk_image_menu_item_new_with_mnemonic(_("_File"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file), gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	/* Scan */
	scan = gtk_image_menu_item_new_with_mnemonic(_("_Scan"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(scan), gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(scan, "activate", accel_group, GDK_KEY_F1, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(scan), "activate",G_CALLBACK(scan_f), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), scan);
	/* Update */
	update = gtk_image_menu_item_new_with_mnemonic(_("_Update"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(update), gtk_image_new_from_stock(GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(update, "activate", accel_group, GDK_KEY_F2, 0, GTK_ACCEL_VISIBLE); 
	g_signal_connect(G_OBJECT(update), "activate",G_CALLBACK(fprot_update), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), update);
	/* Report */
	report = gtk_image_menu_item_new_with_mnemonic(_("_Report"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(report), gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(report, "activate", accel_group, GDK_KEY_F3, 0, GTK_ACCEL_VISIBLE); 
	g_signal_connect(G_OBJECT(report), "activate",G_CALLBACK(view_report), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), report);
	/* Separator */
	sep = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep);
	/* Quit */
	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	g_signal_connect(G_OBJECT(quit), "activate",G_CALLBACK(main_window_close), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

	/* Tools Menu */
	toolsmenu = gtk_menu_new();
	/* Tools */
	tools = gtk_image_menu_item_new_with_mnemonic(_("_Tools"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tools), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools), toolsmenu);
	/* Test */
	test = gtk_image_menu_item_new_with_mnemonic(_("T_est"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(test), gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(test, "activate", accel_group, GDK_KEY_F4, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(test), "activate",G_CALLBACK(fprot_test), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), test);
	/* Info */
	info = gtk_image_menu_item_new_with_mnemonic(_("_Info"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(info), gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(info, "activate", accel_group, GDK_KEY_F5, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(info), "activate",G_CALLBACK(fprot_info), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), info);
	/* Virus list */
	viruslist = gtk_image_menu_item_new_with_mnemonic(_("_Virus List"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(viruslist), gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(viruslist, "activate", accel_group, GDK_KEY_F6, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(viruslist), "activate",G_CALLBACK(fprot_virlist), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), viruslist);
	/* Save settings */
	save_settings = gtk_image_menu_item_new_with_mnemonic(_("_Save Settings"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(save_settings), gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(save_settings, "activate", accel_group, GDK_KEY_F7, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(save_settings), "activate",G_CALLBACK(save_settings_f), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), save_settings);
	/* clear settings */
	clear_settings = gtk_image_menu_item_new_with_mnemonic(_("_Clear Settings"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(clear_settings), gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU));	
	gtk_widget_add_accelerator(clear_settings, "activate", accel_group, GDK_KEY_F8, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(clear_settings), "activate",G_CALLBACK(clear_settings_f), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolsmenu), clear_settings);
	
	/* Help Menu */
	help_menu = gtk_menu_new();
	/* Help */
	help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_group);
	/* About */
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel_group);
	g_signal_connect(G_OBJECT(about), "activate",G_CALLBACK(about_f), NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about);

	/* Finalize Menus */
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), tools);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);

	frame1 = frame_create_on_table(_("Path to scan"), table1, 0, 10, 1, 4);
	top_table = table_create(20, 2, frame1, 2, HOMOGENEUS);
	path = get_desktop();
	debug_err_msg("%s", path);

	path_entry_box = default_text_box_create_on_table(path, EDITABLE, top_table, 0, 19, 0, 1);
	button1 = button_create_on_table("", G_CALLBACK(do_file_selection), NULL, top_table, 19, 20, 0, 1);
	gtk_button_set_image(GTK_BUTTON(button1), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON));
	check_button1 = check_create_on_table(_("Do not scan subdirectories"), G_CALLBACK(flag_callback), &flag_nosub, 
													INACTIVE, top_table, 0, 10, 1, 2);

	frame2 = frame_create_on_table(_("Scan mode"), table1, 0, 5, 4, 8);
	mode_table = table_create(1, 3, frame2, 2 , HOMOGENEUS);

	/* Warning we can't use the ACTIVE flag here because the callback function acts on widgets */
	/* we haven't created yet, so we do things manually later by calling toggle1_f() */
	scan_1_button = radio_create_on_table(NULL, _("Report only"), G_CALLBACK(new_flag_scanmode_callback), "0",
													INACTIVE, mode_table, 0, 1, 0, 1);
	scan_mode_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (scan_1_button));
	scan_3_button = radio_create_on_table(scan_1_button, _("Automatic"),   G_CALLBACK(new_flag_scanmode_callback), "2",
													INACTIVE, mode_table, 0, 1, 1, 2);

	/* Version 6.x.x always scans for macro virus: By default fpscan removes only known malware macros */
	flag_macro = 0;
	
	frame5 = frame_create_on_table(_("Macro virus removal options"), table1, 5, 10, 4, 8);
	macro_remove_table = table_create(1, 4, frame5, 2 ,  HOMOGENEUS);

	macro_1_button = radio_create_on_table(	NULL, _("Remove infected macros from docs"),
											G_CALLBACK(flag_macro_action_callback), "0",
											INACTIVE, macro_remove_table, 0, 1, 0, 1);
	remove_macro_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (macro_1_button));
	macro_2_button = radio_create_on_table(	macro_1_button, _("Remove all macros from infected docs"),
													G_CALLBACK(flag_macro_action_callback), "1",
													INACTIVE, macro_remove_table, 0, 1, 1, 2);
	macro_3_button = radio_create_on_table(	macro_2_button, _("Remove all macros from doc if new variant is found"),
													G_CALLBACK(flag_macro_action_callback), "2",
													INACTIVE, macro_remove_table, 0, 1, 2, 3);
	/* --stripallmacros switch undocumented in the help text. Needs --disinfect */
	macro_4_button = radio_create_on_table(	macro_3_button, _("Remove all macros from all docs"),
													G_CALLBACK(flag_macro_action_callback), "3",
													INACTIVE, macro_remove_table, 0, 1, 3, 4);

	frame6 = frame_create_on_table(_("Use heuristics"), table1, 0, 5, 12, 16);

	heur_table = table_create(1, 3, frame6, 2 ,  HOMOGENEUS);
	heur_1_button = radio_create_on_table(NULL, _("No"), G_CALLBACK(new_flag_heur_callback), "1",
										INACTIVE, heur_table, 0, 1, 0, 1);
	heuristics_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (heur_1_button));
	heur_2_button = radio_create_on_table(heur_1_button, _("Default"), G_CALLBACK(new_flag_heur_callback), "0",
										INACTIVE, heur_table, 0, 1, 1, 2);
	heur_3_button = radio_create_on_table(heur_2_button, _("Maximum"),G_CALLBACK(new_flag_heur_callback), "2", 
										INACTIVE, heur_table, 0, 1, 2, 3);

										
	frame6 = frame_create_on_table(_("Report file"), table1, 0, 5, 16, 20);
	report_table = table_create(8, 3, frame6, 2 , HOMOGENEUS);

	report_file_path = xconcat_path_file(xgetcwd(NULL), DEF_REPORT_FILE);
	report_path_entry_box = my_gtk_file_chooser_button(report_file_path);
	attach_to_table(report_table, report_path_entry_box, 0, 7, 0, 1);
	xfree(&report_file_path);
	xfree(&path);

	check_button3 = check_create_on_table(_("List all scanned files"),
										G_CALLBACK(flag_callback), &flag_list,
										INACTIVE, report_table, 0, 7, 1, 2);

	frame7 = frame_create_on_table(_("Options"), table1, 5, 10, 12, 20);
	option_table = table_create(5, 7, frame7, 2 , HOMOGENEUS);

	archives_button = check_create_on_table(_("Scan inside archives"),
										G_CALLBACK(flag_callback), &flag_archives,
										INACTIVE, option_table, 0, 5, 0, 1);
	

	check_button5 = check_create_on_table(_("Scan for other various malware"),
										G_CALLBACK(flag_callback), &flag_collect,
										INACTIVE, option_table, 0, 5, 1, 2);
#ifdef USE_NEW_TOOLTIP
	gtk_widget_set_tooltip_text(check_button5, _("Scan for adware and remote access tools"));
#else
	malware_tooltip = gtk_tooltips_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS (malware_tooltip), check_button5,
						_("Scan for adware and remote access tools"), NULL);
#endif
	check_button6 = check_create_on_table(_("Follow symbolic links"),
										G_CALLBACK(flag_callback), &flag_follow,
										INACTIVE, option_table, 0, 5, 2, 3);

	check_button7 = check_create_on_table(_("Don't descend directories\non other filesystems"),
										G_CALLBACK(flag_callback), &flag_mount,
										INACTIVE, option_table, 0, 5, 3, 4);

	check_button8 = check_create_on_table(_("Scan boot sectors"),
										G_CALLBACK(flag_callback), &flag_boot,
										INACTIVE, option_table, 0, 5, 4, 5);

#ifdef USE_NEW_TOOLTIP	
	gtk_widget_set_tooltip_text(check_button8, _("You need administrator rights"));
#else
	boot_tooltip = gtk_tooltips_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS (boot_tooltip), check_button8,
						_("You need administrator rights"), NULL);
#endif

	button3 = button_create_on_table(_("F6 Virus list"),G_CALLBACK(fprot_virlist),	 NULL, table1, 7, 8,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button3), gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_BUTTON));
	button4 = button_create_on_table(_("F5 Info"),		G_CALLBACK(fprot_info),		 NULL, table1, 6, 7,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button4), gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_BUTTON));
	button5 = button_create_on_table(_("F4 Test"),		G_CALLBACK(fprot_test),		 NULL, table1, 5, 6,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button5), gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON));
	button6 = button_create_on_table(_("F3 Report"),	G_CALLBACK(view_report),	 NULL, table1, 4, 5,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button6), gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_BUTTON));
	button7 = button_create_on_table(_("F2 Update"),	G_CALLBACK(fprot_update),	 NULL, table1, 3, 4,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button7), gtk_image_new_from_stock(GTK_STOCK_NETWORK, GTK_ICON_SIZE_BUTTON));
	button8 = button_create_on_table(_("F1 Scan"),		G_CALLBACK(scan_f),			 NULL, table1, 2, 3,  20, 21);
	gtk_button_set_image(GTK_BUTTON(button8), gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_BUTTON));
	/* DON'T REMOVE THIS */
	gtk_widget_show_all(main_window);
}

static void new_restore_saved_state(void)
{
	int flag_macro_action_saved = flag_macro_action;
	/* Version 6.x.x always scans for macro virus */
	flag_macro = 0;

	/* Restore saved settings or setup the GUI with defaults  */
	if (saved_settings) {
		/* Simple check boxes */
		if (flag_nosub) {
			flag_nosub = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button1), TRUE);
		}
		if (flag_list) {
			flag_list = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button3), TRUE);
		}
		if (flag_collect) {
			flag_collect = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button5), TRUE);
		}
		if (flag_follow) {
			flag_follow = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button6), TRUE);
		}
		if (flag_mount) {
			flag_mount = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button7), TRUE);
		}
		if (flag_boot) {
			flag_boot = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button8), TRUE);
		}
		if (flag_archives) {
			flag_archives = 0;
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(archives_button), TRUE);
		}
		switch (flag_scanmode) {
			/* new_flag_scanmode_callback sets the value of flag_scanmode */	
			case 2: /* Automatic */
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (scan_3_button), TRUE);
				switch (flag_macro_action_saved) {
					/* flag_macro_callback sets the value of flag_macro */	
					case 0:
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (macro_1_button), TRUE);
						break;
					case 1:
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (macro_2_button), TRUE);
						break;
					case 2:
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (macro_3_button), TRUE);
						break;
					case 3:
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (macro_4_button), TRUE);
						break;
				}
				break;
			case 0:  /* Report-only */
			/* Fallthrough */
			default: /* Illegal Value */
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (scan_1_button), TRUE);
				flag_macro_action = 0;
				gtk_widget_hide(macro_remove_table);
		}
		switch (flag_heur) {
			/* new_flag_heur_callback sets the value of flag_heur */
			case 1:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (heur_1_button), TRUE);
				break;
			case 0:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (heur_2_button), TRUE);
				break;
			case 2:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (heur_3_button), TRUE);
				break;
		}
	} else {
		/* Setup with defaults */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (heur_2_button), TRUE);
		toggle2_f();
	}
}


int main(int argc, char **argv)
{
	char *command;
	struct passwd *pw;
	struct stat stat_p;
	GList * icon_list;
	/* This is to remove a false warning of gcc 4.3.1
	 * about: warning: array subscript is above array bounds */
	char v[] = "-v";
	char h[] = "-h";
	char k[] = "-k";

	setlocale(LC_ALL, "" );
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	textdomain(GETTEXT_PACKAGE);
	
	/* Print GPL disclaimer to terminal */
	command = xmake_message("\n%s\n\n%s\n%s\n%s\n%s.\n", PACKAGE_STRING, COPYRIGHT,
						_("Licensed under the GNU General Public License, version 2"),
						HOMEPAGE,
						_("This program comes with ABSOLUTELY NO WARRANTY"));
	xputs(command);
	xfree(&command);

	unixsignal_init();
	unixsignal_install(SIGHUP, terminate, NULL);
	unixsignal_install(SIGTERM, terminate, NULL);
	unixsignal_install(SIGINT, terminate, NULL);
	
	/* Initialize i18n support */
	gtk_init (&argc, &argv);

	/* Force the output to be unbuffered when debugging.*/
#ifdef DEBUG
	setbuf(stdout, (char *) 0);
#endif

	/* Command line args. No dialog windows on errors as this is console stuff */
	if (argc == 2) {
		if (strcmp(argv[1], v) == 0) {
			/* Version is displayed above so just exit */
			exit(EXIT_SUCCESS);
		}
		if (strcmp(argv[1], h) == 0) {
			/* Version is displayed above so just exit */
			printf(_("Usage: %s [option] [path]\n"), argv[0]);
			printf(_("\t-k        run as KDE menu action\n"));
			printf(_("\t-h        display this help and exit\n"));
			printf(_("\t-v        output version information and exit\n\n"));
			exit(EXIT_SUCCESS);
		}
		err_msg(_("%s: unknown command line switch: %s"), argv[0], argv[1]);
		exit(EXIT_FAILURE);
	}

	debug_err_msg("starting...");

	autodetect_fprot();
	/* Needed by aterm_terminal_hack */
	autodetect_pager();	
	autodetect_terminal_emulator();

	if (argc > 2) {
		if (strcmp(argv[1], k) == 0) {
			command = xmake_message("%s %s \"%s\"", terminal_emulator, fprot_bin, argv[2]);
			debug_err_msg("executing scan on: %s", argv[2]);
			g_spawn_command_line_async (command, NULL);
			xfree(&command);
			exit(EXIT_SUCCESS);
		} else {
			err_msg(_("%s: too much args on command line"), argv[0]);
			exit(EXIT_FAILURE);
		}
	}

#ifdef HAVE_PRCTL
	/* Make the process not dumpable under Linux 2.4, seems to work also for 2.6.0-test3 */
	prctl(PR_SET_DUMPABLE,0);
	debug_err_msg("process is%sdumpable", (prctl(PR_GET_DUMPABLE) == 0) ? " not " : " ");
#endif

	/* Try to determine the home dir of the user in */
	/* passwd struct to avoid use of getenv("HOME").*/

	/* Who am I ? */
	if ((pw = getpwuid(getuid())) == NULL) {
		error_dialog(_("Cannot get password file entry.\nAborting..."));
		exit(EXIT_FAILURE);
	}
	/* Chdir to user's home dir */
	if ((xchdir(pw->pw_dir, RETURN_ON_FAILURE)) == EXIT_ERROR) {
		dialog_window(GTK_STOCK_DIALOG_WARNING, ONE_BUTTON,
						NULL, 1, _("Failed to change dir to user's home dir,\ntrying\"/\"instead."));
		if (xchdir("/",RETURN_ON_FAILURE) == -1) {
			error_dialog(_("Failed to change dir to \"/\",\nAborting..."));
			exit(EXIT_FAILURE);
		}
	}
	/* Check for xfprot's private dir */
	if (lstat(XFPROT_PRIV_DIR,&stat_p) == 0) {
		if (S_ISLNK(stat_p.st_mode)) {
			/* We don't want to be redirected by a link */
			error_dialog(_("xfprot's private dir is a link.\nAborting..."));
			exit(EXIT_FAILURE);
		}
	} else {
		if (xmkdir(XFPROT_PRIV_DIR, 0700, RETURN_ON_FAILURE) == -1) {
			error_dialog(_("Cannot create xfprot's private dir.\nAborting..."));
			exit(EXIT_FAILURE);
		}
	}

	/* Explicitly set private dir's permissions and owner and chdir into it */
	working_dir_setup_and_chdir(pw->pw_uid, pw->pw_gid);
	/* Lock file stuff */
	do_lock_file();
	/* Display a warning if we are root */
	root_warn(pw->pw_uid);
	/* Detect the version */
	detect_fprot_version();
	/* Setup commands  for our versions of fprot */
	setup_commands();
	/* Restore saved settings */
	saved_settings = read_saved_settings();

	icon_list = get_icon_list();
	if (have_version < 620) {
		error_dialog(_("Unsupported old F-prot version found,\nplease update to latest.\nAborting..."));
		exit(EXIT_SUCCESS);
	} else {
		new_gui(icon_list);
		new_restore_saved_state(); 
	}

	debug_err_msg("Settings at startup");
	debug_err_msg("scanmode=%d",     flag_scanmode);
	debug_err_msg("macro=%d",        flag_macro);
	debug_err_msg("macro_action=%d", flag_macro_action);
	debug_err_msg("heur=%d",         flag_heur);
	debug_err_msg("nosub=%d",        flag_nosub);
	debug_err_msg("disinf=%d",       flag_disinf);
	debug_err_msg("rename=%d",       flag_rename); 
	debug_err_msg("delete=%d",       flag_delete);
	debug_err_msg("append=%d",       flag_append);
	debug_err_msg("list=%d",         flag_list);
	debug_err_msg("archives=%d",     flag_archives);
	debug_err_msg("collect=%d",      flag_collect);
	debug_err_msg("ai=%d",           flag_ai);
	debug_err_msg("follow=%d",       flag_follow);
	debug_err_msg("mount=%d", flag_mount);
	debug_err_msg("boot=%d",  flag_boot);
	debug_err_msg("old=%d",          flag_old);

	/* Display license window, this can be disabled */
	if (lstat(NO_SPLASH_FILE, &stat_p) != 0) {
		if (show_about(FIRST_TIME) == 1) {
			create_file(NO_SPLASH_FILE);
		}
	}

	gtk_main();
	unixsignal_cleanup();
	exit(EXIT_SUCCESS);
}
