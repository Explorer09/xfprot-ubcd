/* vi: set sw=4 s=4: */
/*
 * Various utility routines.
 *
 * Copyright (C) 2002-2009 by Tito Ragusa <tito-wolit@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#ifndef __HAVE_MYLIB_
#define __HAVE_MYLIB_ 1

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <errno.h>


#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif

/* Macros for min/max.  */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* Some useful definitions */
#undef FALSE
#define FALSE   ((int) 0)
#undef TRUE
#define TRUE    ((int) 1)
#undef SKIP
#define SKIP	((int) 2)

/* For debugging */
#ifdef DEBUG
#define DBG_FUNCTION_NAME err_msg("%s()",__FUNCTION__)
#else
#define DBG_FUNCTION_NAME
#endif
#ifdef DEBUG
#define debug(x)		x
#define debug_err_msg			err_msg
#else
#define debug(x)
#define debug_err_msg(...)
#endif

void err_msg(const char *fmt, ...)__attribute__ ((format (printf, 1, 2)));
void err_msg_and_die(const char *fmt, ...)__attribute__ ((format (printf, 1, 2))) __attribute__(( __noreturn__ ));
void msg(const char *fmt, ...)__attribute__ ((format (printf, 1, 2)));
void out_of_memory_error_and_die(void) __attribute__(( __noreturn__ ));
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xmalloc(size_t size);
void *xzalloc(size_t size);
void xfree(void *ptr);
char *xstrdup(const char *s);
char *xstrndup(const char *s, int n);
void path_error_and_die(const char * func, const char * path) __attribute__(( __noreturn__ ));
FILE *xfopen(const char *path, const char *mode);
FILE *xpopen(const char *path, const char *mode);
DIR *xopendir(const char *path);
int xopen(const char *path, int flags);
int xopen3(const char *path, int flags, int mode);
int open3_or_warn(const char *pathname, int flags, int mode);
int open_or_warn(const char *pathname, int flags);
off_t xlseek(int fd, off_t offset, int whence);
char *xgetcwd (char *cwd);

#define SUBSTITUTE 			((int) 1)
#define NOT_SUBSTITUTE		((int) 0)

char *illegal_chars_in_string(char *string, int substitute);
char *fgetc_buf(FILE *fd);
char *xfgetc_nbuf(FILE *fd, int size);
char *safe_strncpy(char *dst, const char *src, size_t size);
int  xsnprintf(char *dest, size_t size, const char *fmt, ...)  __attribute__ ((format (printf, 3, 4)));
char *xstrncat(char *dest, const char *src, size_t size);

#define RETURN_ON_FAILURE				((int) 0)
#define EXIT_ON_FAILURE					((int) 1)
#define RETURN_ON_ERROR					((int) 0)
#define EXIT_ON_ERROR					((int) 1)
#define CONTINUE_ON_ERROR				((int) 0)

#define EXIT_ERROR						((int) -1)

int xchdir(char *path, int flag);
int xmkdir(const char *path, mode_t mode, int flag);
int xchown(const char *path, uid_t owner, gid_t group, int flag);
int xchmod(const char *path, mode_t mode, int flag);
int xfclose_nostdin(FILE *fd);
int xpclose_nostdin(FILE *fd);

char *last_char_is(const char *s, int c);
char *first_char_is(const char *s, int c);
char *xconcat_path_file(const char *path,char *filename);
char *xconcat_subpath_file(const char *path, char *filename);
int  xputs(const char * string);
char *xmake_message(const char *fmt, ...)  __attribute__ ((format (printf, 1, 2)));

#define UID		((int) 0)
#define GID		((int) 1)

char *get_uid_or_gid (int flag,long idnum);
int file_exists(char *filename, struct stat *buf);
int  xstrlen(const char* s);
void trim(char *s);
char *get_chunk_from_file(FILE *file, int *end);
char *get_line_from_file(FILE *file);
char *get_chomped_line_from_file(FILE *file);
unsigned long count_lines( char * path );
char *ask_user(const char * prompt);
char *itoa(int i);
int compare_string_array(char *string_array[], const char *key);
int find_array_in_haystack(char *needle_array[], const char *haystack);
int xstrncmp(const char *s1, const char *s2, size_t n);
int xstrcmp(const char *s1, const char *s2);
int check_date(char *date);
char *str_reverse(char *str);
char *format_time(time_t cal_time);
int strtodouble(const char *numstr, double* value);
int strtoint(const char *numstr, int minval, int maxval, int *value);
int strtolong(const char *numstr, long minval, long maxval, long *value);
int strtollong(const char *numstr, long long minval, long long maxval, long long *value);
#define xstrtoint(str,min,max,val)			if(strtoint(str,min,max,val))err_msg_and_die("%s",strerror(errno))
#define xstrtolong(str,min,max,val)			if(strtolong(str,min,max,val))err_msg_and_die("%s",strerror(errno))
#define xstrtollong(str,min,max,val)		if(strtollong(str,min,max,val))err_msg_and_die("%s",strerror(errno))
int strtouint(const char *numstr, unsigned int minval, unsigned int maxval, unsigned int *value);
int strtoulong(const char *numstr, unsigned long minval, unsigned long maxval, unsigned long *value);
int strtoullong(const char *numstr, unsigned long long minval, unsigned long long  maxval, unsigned long long *value);
#define xstrtouint(str,min,max,val)			if(strtouint(str,min,max,val))err_msg_and_die("%s",strerror(errno))
#define xstrtoulong(str,min,max,val)		if(strtoulong(str,min,max,val))err_msg_and_die("%s",strerror(errno))
#define xstrtoullong(str,min,max,val)		if(strtoullong(str,min,max,val))err_msg_and_die("%s",strerror(errno))
/*long unsigned int xgetlarg(const char *arg, int base, long lower, long upper);*/
typedef struct llist_t {
	char *data;
	struct llist_t *link;
} llist_t;
void llist_add_to(llist_t **old_head, void *data);
void llist_add_to_end(llist_t **list_head, void *data);
void *llist_pop(llist_t **elm);
void llist_unlink(llist_t **head, llist_t *elm);
void llist_free(llist_t *elm, void (*freeit)(void *data));
llist_t *llist_rev(llist_t *list);
#endif /* __HAVE_MYLIB_ */

