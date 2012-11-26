/*
* @file
* GLib main loop reentrant signal queue.
*
* Based on suggestions from Havoc Pennington <hp@redhat.com> 
* and Tim Janik <timj@gtk.org> on gtk-app-devel-list
* @Reference: http://mail.gnome.org/archives/gtk-app-devel-list/1999-September/msg00067.html
*
* Copyright (C) 2005  Andreas Stricker
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*/

/* 
* Example code

#include "gunixsignal.h"

void terminate(gint sig, gpointer data);

unixsignal_init();
unixsignal_install(SIGTERM, terminate, NULL);

gtk_main();

unixsignal_cleanup();

*/


#include "gunixsignal.h"

#define __GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE_READING 0
#define PIPE_WRITING 1
#define MAX_SIGNAL 32

typedef struct {
	gpointer        user_data;
	UnixSignalFunc callback;
} signal_handler_t;


/* Forward declarations */
static void sig_watcher(int signum);
static gboolean sig_catcher(GIOChannel *source, GIOCondition condition, gpointer data);
static void cleanup_signal_handler_list(gpointer data, gpointer user_data);


/*
* Signal pipe file destructor array as described in pipe(2)
*/
static int signal_fds[2];

/*
* A list for each signal of registred signal handlers.
*/
static GList *signal_handlers[MAX_SIGNAL];



/*" 
* Catch unix signals, write them to the pipe.
* @note this function must be reentrant as described in signal(2). It
*       must use safe functions only.
* @param signum: Signal number received
*/
static void 
sig_watcher(int signum) 
{ /* entering reentrant part */
	write(signal_fds[PIPE_WRITING], &signum, sizeof(signum));
} /* leaving reentrant part */

/*
* Signal catcher called by glib from IO-watcher. It's call every time a
* signal is received and written to signal_fds pipe.
* This function don't need to be reentrant but can do whatever we want.
* @param source: Source file descriptor.
* @param condition: Condition this event was emitted on.
* @param data: user data, NULL in this case.
* @return TRUE to not remove this event source.
*/
static gboolean 
sig_catcher(GIOChannel *source, GIOCondition condition, gpointer data) {
	int signum;

	while ( read(signal_fds[PIPE_READING], &signum, sizeof(signum)) > 0 ) {
		signal_handler_t* sigh;
		GList *list;

		/* error if signal number not in range [0..32[ */
		if (0 > signum || MAX_SIGNAL <= signum) return FALSE;   /* remove from event source */

		/* process signals */
		for (list = signal_handlers[signum]; list != NULL; list = g_list_next(list)) {
			sigh = (signal_handler_t*) list->data;
			if ( sigh != NULL ) 
				sigh->callback(signum, sigh->user_data);
		} /* while item in list */
	
	} /* while read signal on pipe */

	return TRUE;
	/* to shut a compiler warning */
	source = NULL;
	data = NULL;
	condition = 0;
}

/*
* Initialize glib signal handler enviroment.
* TODO ast: exit on error instead return ???
* @return TRUE if no error occured.
*/
gboolean 
unixsignal_init() {
	if ( pipe(signal_fds) == 0 ) {
		long arg;
		guint srcid;
		GIOChannel *channel;
		gint i;

		for (i = 0; i < MAX_SIGNAL; i++) signal_handlers[i] = NULL;

		/* create new handler for pipe read fd */
		channel = g_io_channel_unix_new(signal_fds[0]);
		/* add new watch for this fd */
		srcid = g_io_add_watch(channel, G_IO_IN | G_IO_ERR | G_IO_HUP, sig_catcher, NULL);
		/* modify pipe to not block on write (don't hang in signal handler) */
		arg = fcntl(signal_fds[0], F_GETFL);                /* read current flags */
		fcntl(signal_fds[0], F_SETFL, arg | O_NONBLOCK);    /* add nonblocking mode */
	}
	else return FALSE;

	return TRUE;    /* no errors occured */
}

/*
* Frees allocated memory of a list element
* @param data: data element from list.
* @param user_data: data fiven by g_list_foreach().
*/
static void 
cleanup_signal_handler_list(gpointer data, gpointer user_data) {
	g_free(data);
	/*  to shut a compiler warning */
	user_data = NULL;
}

/*
* Clean up signal handler by restoring default handlers and frees
* allocated memory.
*/
void 
unixsignal_cleanup() {
	gint i;
	
	for (i = 0; i < MAX_SIGNAL; i++) {
		if (signal_handlers[i] != NULL) {
			signal(i, SIG_DFL);             /* restore default behaviour */
			/* free list */
			g_list_foreach(signal_handlers[i], cleanup_signal_handler_list, NULL);
		}
	}
	close(signal_fds[PIPE_WRITING]);
	close(signal_fds[PIPE_READING]);
}

/*
* Install a new signal handler callback for a unix signal.
* TODO ast: prevent from installing on function more than once.
* @param unixsignal: unix signal (see signal(7) to install a handler for.
* @param callback: Function to call if signal is raised.
* @param data: user data to call callback function with (must be freed by user).
* @return TRUE if no error occurs.
*/
gboolean 
unixsignal_install(int unixsignal, UnixSignalFunc callback, gpointer data) {
	signal_handler_t *sigh = NULL;
	struct sigaction action;
	int result;

	/* error if no valid callback function */
	if (callback == NULL) return FALSE;
	/* error if signal number not in range [0..32[ */
	if (0 > unixsignal || MAX_SIGNAL <= unixsignal) return FALSE;
	/* error if signal number cannot be caught */
	if (unixsignal == SIGKILL || unixsignal == SIGSTOP) return FALSE;

	/* install callback for signal handler */
	sigh = g_new(signal_handler_t, 1);
	sigh->user_data = data;
	sigh->callback = callback;
	signal_handlers[unixsignal] = g_list_append(signal_handlers[unixsignal], sigh);

	/* install signal handler */
	action.sa_handler = sig_watcher;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	result = sigaction(unixsignal, &action, NULL);

	return (result == 0);
}

/*
* Deinstall a signal handler for a specific callback function.
* @param unixsignal: deinstall handler for this signal.
* @param callback: deinstall this function handler only.
* @return TRUE if callback was removed.
*/
gboolean 
unixsignal_deinstall(int unixsignal, UnixSignalFunc callback) {
	signal_handler_t* sigh;
	GList *list;

	/* error if signal number not in range [0..32[ */
	if (0 > unixsignal || MAX_SIGNAL <= unixsignal) return FALSE;

	for (list = signal_handlers[unixsignal]; list != NULL; list = g_list_next(list)) {
		sigh = (signal_handler_t*) list->data;
		if ( sigh != NULL ) {
			if ( sigh->callback == callback ) {
				g_free(sigh);
				signal_handlers[unixsignal] = 
					g_list_delete_link(signal_handlers[unixsignal], list);
				return TRUE;
			}
		}
	}   /* while item in list */
	return FALSE;
}


