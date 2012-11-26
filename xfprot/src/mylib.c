/*
 * Various utility routines put together by Tito <tito-wolit@tiscali.it> .
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

#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "mylib.h"

void err_msg(const char *fmt, ...)
{
	va_list p;
	fflush(stdout);
	#ifdef PACKAGE_STRING
	fprintf(stderr, "%s: ", PACKAGE_STRING);
	#endif
	va_start(p, fmt);
	vfprintf(stderr,fmt, p);
	va_end(p);
	putc('\n', stderr);
}

void err_msg_and_die(const char *fmt, ...)
{
	va_list p;
	fflush(stdout);
	#ifdef PACKAGE_STRING
	fprintf(stderr, "%s: ", PACKAGE_STRING);
	#endif
	va_start(p, fmt);
	vfprintf(stderr,fmt, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}
#if 0  /* UNUSED */
void msg(const char *fmt, ...)
{
	va_list p;

	fflush(stdout);
	#ifdef PACKAGE_STRING
	fprintf(stdout, "%s: ", PACKAGE_STRING);
	#endif
	va_start(p, fmt);
	vfprintf(stdout,fmt, p);
	va_end(p);
	putc('\n', stdout);
}
#endif
/* Memory allocation */
void out_of_memory_error_and_die(void)
{
	err_msg_and_die("memory exhausted");
}

void *xmalloc(size_t size)
{
	void *ptr;

	if ((ptr = malloc((size) ? size : 1)))
		return ptr;

	out_of_memory_error_and_die();
}

void *xrealloc(void *ptr, size_t size)
{
	/* It avoids to free ptr if size = 0 */
	/* if ptr == NULL it does malloc(size) */
	if ((ptr = realloc(ptr, (size) ? size : 1)))
		return ptr;

	out_of_memory_error_and_die();
}
#if 0  /* UNUSED */
void *xcalloc(size_t nmemb, size_t size)
{
	void *ptr;

	/* It avoids nmemb = 0 and size = 0 */
	if ((ptr = calloc(nmemb ? (nmemb) : 1, (size) ? size : 1)))
		return ptr;

	out_of_memory_error_and_die();
}

/* Die if we can't allocate and zero size bytes of memory.*/
void *xzalloc(size_t size)
{
	void *ptr = xmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}
#endif
void xfree(void *ptr)
{
	/*	 sets the pointer to NULL
		after it has been freed.*/
	void **pp = (void **)ptr;

	if (*pp == NULL)
		return;

	free(*pp);
	*pp = NULL;
}

char *xstrdup(const char *s)
{
	char *n;
	
	if (!s)
		return NULL;

	if ((n = strdup(s)))
		return n;

	out_of_memory_error_and_die();
}
#if 0  /* UNUSED */
char *xstrndup(const char *s, int n)
{
	int m;
	char *t;

	if (s == NULL)
		s = "";

	/* We can just xmalloc(n+1) and strncpy into it, */
	/* but think about xstrndup("abc", 10000) wastage! */
	m = n;
	t = (char*) s;
	while (m) {
		if (!*t)
			break;
		m--; t++;
	}
	n = n - m;
	t = xmalloc(n + 1);
	t[n] = '\0';

	return memcpy(t,s,n);
}
#endif
void path_error(const char * func, const char * path)
{
	err_msg("%s: %s: %s", func, (path) ? path : "NULL", strerror(errno));
}

void path_error_and_die(const char * func, const char * path)
{
	path_error(func, path);
	exit(EXIT_FAILURE);
}
#if 0  /* UNUSED */
FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp;

	if ((fp = fopen(path, mode)) == NULL)
		path_error_and_die(__FUNCTION__, path);

	return fp;
}
#endif
FILE *xpopen(const char *path, const char *mode)
{
	FILE *fp;

	if ((fp = popen(path, mode)) == NULL)
		path_error_and_die(__FUNCTION__, path);

	return fp;
}
#if 0  /* UNUSED */
/* Die with an error message if opendir() fails. */
DIR *xopendir(const char *path)
{
	DIR *dp;

	if ((dp = opendir(path)) == NULL)
		path_error_and_die(__FUNCTION__, path);

	return dp;
}

/* Die if we can't open an existing file and return an fd. */
int xopen(const char *path, int flags)
{
	return xopen3(path, flags, 0666);
}

/* Die if we can't open a new file and return an fd. */
int xopen3(const char *path, int flags, int mode)
{
	int ret;

	if ((ret = open(path, flags, mode)) < 0)
		path_error_and_die(__FUNCTION__, path);

	return ret;
}

// Warn if we can't open a file and return a fd.
int open3_or_warn(const char *pathname, int flags, int mode)
{
	int ret;

	ret = open(pathname, flags, mode);
	if (ret < 0) {
		path_error(__FUNCTION__, pathname);
	}
	return ret;
}

// Warn if we can't open a file and return a fd.
int open_or_warn(const char *pathname, int flags)
{
	return open3_or_warn(pathname, flags, 0666);
}


/* Die with an error message if we can't lseek to the right spot. */
off_t xlseek(int fd, off_t offset, int whence)
{
	off_t off = lseek(fd, offset, whence);
	if (off == (off_t) -1)
		err_msg_and_die("xlseek:%s", strerror(errno));
	return off;
}
#endif
/* Return the current directory, newly allocated, arbitrarily long.
   Return NULL and set errno on error.
   If argument is not NULL (previous usage allocate memory), call free()
*/

char * xgetcwd (char *cwd)
{
	char *ret;
	unsigned path_max;
	
	path_max = (unsigned) PATH_MAX;
	path_max += 2;                /* The getcwd docs say to do this. */
	
	if (cwd == 0)
		cwd = xmalloc (path_max);
	
	while ((ret = getcwd (cwd, path_max)) == NULL && errno == ERANGE) {
		path_max += 32;
		cwd = xrealloc (cwd, path_max);
	}
	
	if (ret == NULL)
		xfree(&cwd);
	
	return cwd;
}
#if 0  /* UNUSED */
char *illegal_chars_in_string(char *string, int substitute)
{
	int illegal_chars = 0;
	int i = 0;


	/* skip white space */
	while (string && isspace(*string)) {
		++string;
	}

	if (string == NULL) {
		return NULL;
	}
	/* Don't allow leading dash */
	if (string[0] == '-') {
		if (substitute == 1)
			string[0] = '_';
		illegal_chars++;
	}

	for (i = 0; i <= (int)xstrlen(string); i++) {
		switch(string[i]) {
			case ' '	:
			/*case '/'	: legal in paths */ 
			/*case 92	: legal in urls  */ /*case '\':*/
			case '*'	:
			case '?'	:
			case ':'	:
			case ';'	:
			case ','	:
			case 39		: /*case ''': */
			case '`'	:
			case '<'	:
			case '>'	:
			case '"'	:
			case '!'	:
			case '$'	:
			case '%'	:
			case '&'	:
			case '='	:
			case '~'	:
			case '^'	:
			case '|'	:
			case '@'	:
			case '#'	:
			case '('	:
			case ')'	:
			case '['	:
			case ']'	:
			case '{'	:
			case '}'	:
				illegal_chars++;
				if (substitute == 1) {
					string[i] = '_';
				}
				break;
			case '-'	:
				/* Don't allow double dash */
			case '.'	:
				/* Don't allow double dot */
				if (string[i] == string[i+1]) {
					illegal_chars++;
					if (substitute == 1) {
						string[i] = '_';
						string[i + 1] = '_';
					}
				}
				break;
			/*esac*/
		}/*switch*/
	}/*for*/
	if (illegal_chars > 0 && substitute == 0) {
		return NULL;
	}
	return string;
}

/*
 *  This function reads chars from a fd until EOF storing them into
 *  a properly sized buffer and returns a pointer to this buffer .
 *  The buffer should be freed .
 */

char * fgetc_buf(FILE *fd)
{
	int c, i = 0;
	int size = 1024;
	char *buffer = NULL;

	buffer = xmalloc(size);
	while ((c = fgetc(fd)) != EOF) {
		buffer[i] = c;
		if (i == (size - 2)) {
			buffer = xrealloc(buffer, size += 1024);
		}
		i++;
	}
	buffer[i] = '\0';
	return buffer;
}
#endif
/*
 *  This function reads chars from a fd until EOF storing them into
 *  a buffer of given size and returns a pointer to this buffer or NULL on EOF .
 *  The buffer should be freed .
 */

char *xfgetc_nbuf(FILE *fd, int size)
{
	int c, i = 0;
	char *buffer;

	buffer = xmalloc(size);

	while ((c = fgetc(fd)) != EOF) {
		buffer[i] = c;
		i++;
		if (i == (size - 2)) {
			break;
		}
	}
	
	/* NULL terminate the buffer */
	buffer[i] = '\0';
	/*	Read to the end, and on the next iteration return NULL
		so we know the job is done */
	if (i == 0 && c == EOF) {
		free(buffer);
		return NULL;
	}
	return buffer;
}
#if 0  /* UNUSED */
/* Like strncpy but make sure the resulting string is always 0 terminated. */
char * safe_strncpy(char *dst, const char *src, size_t size)
{
	if (!dst)
		 dst = "";
	if (!src)
		return dst; 
	if (!size)
		return dst;
	dst[--size] = '\0';
	return strncpy(dst, src, size);
}
#endif
/*
 *	Function for formatted output conversion,  that disallows
 *	string truncation.
 */

int xsnprintf(char *dest, size_t size, const char *fmt, ...)
{
	int ret;
	va_list p;

	/*The functions snprintf and vsnprintf do not write more  than
	size  bytes (including the trailing '\0').  If the output was truncated
	due to this limit then the return value is  the  number  of  characters
	(not  including the trailing '\0') which would have been written to the
	final string if enough space had been available. Thus, a  return  value
	of  size  or  more means that the output was truncated.
	If an output error is encountered, a negative  value  is returned.
	The  glibc  implementation of the functions snprintf and vsnprintf con-
	forms to the C99 standard, i.e.,  behaves  as  described  above,  since
	glibc version 2.1. Until glibc 2.0.6 they would return -1 when the out-
	put was truncated.
	Linux  libc5 adds a new  conversion character m, which outputs strerror(errno).
	*/

	fflush(stdout);
	va_start(p, fmt);
	ret = vsnprintf(dest, size, fmt, p);
	va_end(p);

	if (ret < 0)
		err_msg_and_die("xsnprintf: failed");

	#if (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1)
	if ((unsigned int)ret >= size)
		err_msg_and_die("xsnprintf: output truncated");
	#endif
	return ret;
}

char *xstrncat(char *dest, const char *src, size_t size)
{
	unsigned int ldest = xstrlen(dest);
	unsigned int lsrc = xstrlen(src);

	if ((size - ldest - 1) < lsrc)
		err_msg_and_die("xstrncat: destination overflow");

	memcpy(dest + ldest, src, lsrc + 1);
	return dest;
}

/*
 * This function changes the current directory to that specified in path
 * and exits on failure or returns -1 accordingly to int flag.
 */

int xchdir(char *path, int flag)
{
	if ((chdir(path)) == 0)
		return EXIT_SUCCESS;

	if (flag == EXIT_ON_FAILURE)
		exit(EXIT_FAILURE);

	return EXIT_ERROR;
}


/*
 * This function  attempts to create a directory named pathname.
 * mode  specifies the permissions to use. It is modified by the process's
 * umask in the usual way and exits on failure or returns -1 accordingly to int flag.
 */

int xmkdir(const char *path , mode_t mode, int flag)
{
	if ((mkdir(path, mode)) == 0)
		return EXIT_SUCCESS;

	if (flag == EXIT_ON_FAILURE)
		exit(EXIT_FAILURE);

	return EXIT_ERROR;
}

/*
 * The  owner of the file specified by path is changed.This function
 * exits on failure or returns -1 accordingly to int flag.
 */

int xchown(const char *path, uid_t owner, gid_t group, int flag)
{
	if ((chown(path, owner, group)) == 0)
		return EXIT_SUCCESS;

	if (flag == EXIT_ON_FAILURE)
		exit(EXIT_FAILURE);

	return EXIT_ERROR;
}

/*
 *  The  mode of the file given by path is changed.This function
 *  exits on failure or returns -1 accordingly to int flag.
 */

int xchmod(const char *path, mode_t mode, int flag)
{
	if ((chmod(path, mode )) == 0)
		return EXIT_SUCCESS;

	if (flag == EXIT_ON_FAILURE)
		exit(EXIT_FAILURE);

	return EXIT_ERROR;
}

/*
 *
 *   The  xfclose  function  dissociates the named stream from its underlying
 *   file or set of functions. If stream is NULL or stdin this function returns -1.
 */

int xfclose_nostdin(FILE *fd)
{
	if (fd && fd != stdin) {
		return fclose(fd);
	}
	return EXIT_ERROR;
}

/*
 *    The  xpclose_nostdin  function waits for the associated process to terminate and
 *    returns the exit status of the command as returned by wait4.
 *    If stream is NULL or stdin this function returns -1.
 */

int xpclose_nostdin(FILE *fd)
{
	if (fd && fd != stdin) {
		return pclose(fd);
	}
	return EXIT_ERROR;
}

/*
 * Find out if the last character of a string matches the one given. Don't
 * underrun the buffer if the string length is 0.  Also avoids a possible
 * space-hogging inline of strlen() per usage.
 */

char * last_char_is(const char *s, int c)
{
	char *sret = (char *)s;
	if (sret) {
		sret = strrchr(sret, c);
		if (sret != NULL && *(sret + 1) != 0) {
			sret = NULL;
		}
	}
	return sret;
}
#if 0  /* UNUSED */
/*
 * Find out if the first character of a string matches the one given. Don't
 * underrun the buffer if the string length is 0.
 */

char * first_char_is(const char *s, int c)
{
	char *sret = (char *)s;
	if (sret) {
		if (sret !=NULL && sret[0] != c) {
			sret = NULL;
		}
	}
	return sret;
}
#endif
/*
 *	Concatenate path and file name to new allocated buffer,
 *	not adding '/' if path name already have it.
 */


char *xconcat_path_file(const char *path, char *filename)
{
	char *buffer;
	char *lc;
	int i = 0;
	int size;

	if (!path) {
	    path = "";
	}

	lc = last_char_is(path, '/');

	if (lc == NULL) {
		i = 1;
	}
	while (*filename == '/') {
		filename++;
	}
	size = xstrlen(path) + xstrlen(filename) + i + 1;
	buffer = xmalloc(size);
	xsnprintf(buffer,size, "%s%s%s", path, (lc == NULL) ? "/" : "", filename);

	return buffer;
}
#if 0  /* UNUSED */
/*
 *
 *   This function covers a special  case for recursive actions of
 *   concat_path_file(path, filename) skiping "." and ".." directory
 *   entries.
 */

char *xconcat_subpath_file(const char *path, char *f)
{
	if (f && *f == '.' && (!f[1] || (f[1] == '.' && !f[2]))) {
		return NULL;
	}
	return xconcat_path_file(path, f);
}
#endif
/*
 * xputs() writes the string s and a trailing newline to stdout. 
 * It returns a non-negative number on success, or EOF on error.
 * If string is NULL it exits with an error message.
 */

int xputs(const char *string)
{
	if (!string)
		return -1;

	return puts(string);
}

/*
 * From Linux Manpage  2000-10-16  PRINTF(3) EXAMPLES
 *
 * This function allocates a sufficiently large buffer for the given
 * string and prints into it (code correct for both glibc 2.0 and glibc 2.1).
 * It returns a pointer to the malloced buffer that you will have to free
 * yourself or exits if some error occurs.
 *
 */

char * xmake_message(const char *fmt, ...)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p;
	va_list ap;

	p = xmalloc(size);
	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf (p, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size) {
			return p;
		}
		/* Else try again with more space. */
		if (n > -1) {			/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		} else {				/* glibc 2.0 */
			size *= 2;		/* twice the old size */
		}
		p =  xrealloc (p, size);
	}
}

/*
	NOTES
	The  glibc  implementation of the functions snprintf and vsnprintf con-
	forms to the C99 standard, i.e.,  behaves  as  described  above,  since
	glibc version 2.1. Until glibc 2.0.6 they would return -1 when the out-
	put was truncated.

	RETURN VALUE

	Upon successful return, these functions return the number of characters
	printed  (not  including  the  trailing  '\0'  used  to  end  output to
	strings).  The functions snprintf and vsnprintf do not write more  than
	size  bytes (including the trailing '\0').  If the output was truncated
	due to this limit then the return value is  the  number  of  characters
	(not  including the trailing '\0') which would have been written to the
	final string if enough space had been available. Thus, a  return  value
	of  size  or  more means that the output was truncated. (See also below
	under NOTES.)  If an output error is encountered, a negative  value  is
	returned.

*/
#if 0  /* UNUSED */
char * get_uid_or_gid (int flag, long idnum)
{

	struct passwd *pw_ent;
	struct group *grp_ent;
	char *message = NULL;

	switch (flag) {
		case UID:
			if ((pw_ent = getpwuid(idnum)) != NULL) {
				message = xmake_message("%s", pw_ent->pw_name);
			} else {
				message = xmake_message("%ld", idnum);
			}
			break;
		case GID:
			if ((grp_ent = getgrgid(idnum)) != NULL) {
				message = xmake_message("%s", grp_ent->gr_name);
			} else {
				message = xmake_message("%ld", idnum);
			}
			break;
	}
	return message;
}

int file_exists(char *filename, struct stat *buf)
{
	struct stat stat_it;

	/* if we stat a broken softlink here it is not displayed as stat returns -1,
	so we use lstat as workaround for this */
	if ((filename == NULL) 
	|| ((stat(filename, (buf != NULL) ? buf : &stat_it)) == -1
	&& (lstat(filename, (buf != NULL) ? buf : &stat_it)) == -1)) {
		return -1;
	}
	return 0;
}
#endif
/*
 * xstrlen() returns the length of a string (even if it is NULL) 
 */

int xstrlen(const char* s)
{
	if (s && *s) {
		return strlen(s);
	}
	return 0;
}

void trim(char *s)
{
	size_t len;
	size_t lws;

	if (s && *s) {
		
		len = strlen(s);
		
		/* trim trailing whitespace */
		while (len && isspace(s[len - 1])) {
			--len;
		}
		/* trim leading whitespace */
		if (len) {
			lws = strspn(s, " \n\r\t\v");
			memmove(s, s + lws, len -= lws);
		}
		s[len] = '\0';
	}
}

/* get_line_from_file() - This function reads an entire line from a text file,
 * up to a newline or NUL byte.  It returns a malloc'ed char * which must be
 * stored and free'ed  by the caller.  If end is null '\n' isn't considered
 * end of line.  If end isn't null, length of the chunk read is stored in it.
 */

char *get_chunk_from_file(FILE * file, int *end)
{
	int ch;
	int idx = 0;
	char *linebuf = NULL;
	int linebufsz = 0;

	while ((ch = getc(file)) != EOF) {
		/* grow the line buffer as necessary */
		if (idx > linebufsz - 2) {
			linebuf = xrealloc(linebuf, linebufsz += 80);
		}
		linebuf[idx++] = (char) ch;
		if (!ch || (end && ch == '\n'))
			break;
	}
	if (end)
		*end = idx;
	if (linebuf) {
		if (ferror(file)) {
			free(linebuf);
			return NULL;
		}
		linebuf[idx] = '\0';
	}
	return linebuf;
}
#if 0  /* UNUSED */
/* Get line, including trailing /n if any */
char *get_line_from_file(FILE * file)
{
	int i;

	return get_chunk_from_file(file, &i);
}
#endif
/* Get line.  Remove trailing /n */
char *get_chomped_line_from_file(FILE * file)
{
	int i;
	char *c = get_chunk_from_file(file, &i);

	if (i && c[--i] == '\n')
		c[i] = '\0';

	return c;
}
#if 0  /* UNUSED */
unsigned long count_lines(char *path)
{
	FILE * file = NULL;	
	unsigned long newlines = 0;
	int c = 0;
	
	if ((file = fopen(path, "r")) == NULL)
		return 0;

	while ((c = fgetc(file)) != EOF) {
		if ( c == '\n')
			newlines++;
	}
	xfclose_nostdin(file);
	return newlines;
} 

/*
 * Ask for user input 
 * I use a static buffer in this function.  Plan accordingly.
 */

char *ask_user(const char * prompt)
{
	char *ret;
	int i, size;
	static char user_input[256];

	size = sizeof(user_input);
	ret = user_input;
	memset(user_input, 0, size);

	fputs(prompt, stdout);
	fflush(stdout);

	if (read(STDIN_FILENO, user_input, size-1) <= 0) {
		ret = NULL;
	} else {
		for(i = 0; i < size && user_input[i]; i++) {
			if (user_input[i]== '\r' || user_input[i] == '\n') {
				user_input[i]= '\0';
				break;
			}
		}
	}
	fflush(stdout);
	return ret;
}

/*
 * Converts an integer into the ascii representation of the number.
 */

char *itoa(int i)
{
	size_t digits = 2; /* automatically has room for the trailing null */
	char *a = NULL;
	int cp_i = i; /* copy of i, used for counting the digits */

	/* if the number is negative, we'll need to store space for the '-' */
	if (i < 0) {
		digits++;
	}

	/* while the copy of i has more than one digit */
	/* incrememnt the digit count, and divide copy of i by ten */
	while (cp_i > 10) {
		digits++;
		cp_i /= 10;
	}

	a = (char *) xmalloc(digits);

	(void) sprintf(a, "%d", i);
	return(a);
}

/* returns the array number of the string */
int compare_string_array(char *string_array[], const char *key)
{
	int i;

	if (!key)
		return -1;

	for (i = 0; string_array[i] != 0; i++) {
		if (strcmp(string_array[i], key) == 0) {
			return i;
		}
	}
	return -i;
}

/* returns the array number of the needle string */
int find_array_in_haystack(char *needle_array[], const char *haystack)
{
	int i;

	if (!haystack)
		return -1;

	for (i = 0; needle_array[i] != 0; i++) {
		if (strstr(haystack, needle_array[i])) {
			return i;
		}
	}
	return -i;
}

int xstrncmp(const char *s1, const char *s2, size_t n)
{
	if (!s1 || !s2 || n < 1)
		return -1;

	return strncmp(s1, s2, n);
}

int xstrcmp(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;

	return strcmp(s1, s2);
}

int check_date(char *date)
{
	int i;
	int len;
	int use_separator = 0;

	/* Check length */
	len = xstrlen(date);
	if (len != 8) {
		if (len == 10) {
			use_separator = 1;
		} else {
			return 1;
		}
	}
	/*  Check that it is composed of digits */
	for (i = 0; i < len; i++) {
		if (use_separator) {
			if (date[i] == '/' || date[i] == '-' || date[i] == '.' || date[i] == ' ') {
				continue;
			}
		}
		if (isdigit(date[i]) == 0) {
			return 2;
		}
	}
	/* Check Year */
	/*VOID */
	/* Check month */ 
	if (date[(use_separator) ? 5 : 4] != '0' && date[(use_separator) ? 5 : 4] != '1')
		return 4;
	/* Check day */
	if (date[(use_separator) ? 8 : 6] != '0' 
	&&  date[(use_separator) ? 8 : 6] != '1'
	&&  date[(use_separator) ? 8 : 6] != '2'
	&&  date[(use_separator) ? 8 : 6] != '3')
		return 5;

	return 0;
}

char *str_reverse(char *str)
{
	int size = xstrlen(str);
	char *p = xstrdup(str);
	
	/* reverse string */
	while (size--) {
		*p = str[size];
		p++;
	}
	/* restore pointer */
	return p -= xstrlen(str);
}

char *format_time(time_t cal_time)
{
	struct tm *time_struct;
	static char string[35];

	time_struct = localtime(&cal_time);
	strftime(string, 35, "%Y-%m-%d %H:%M", time_struct);
	return string;
}

int strtodouble(const char *numstr, double* value)
{
	int errno_save = 0;
	char *endptr;
	double result;

	errno_save = errno;
	errno = 0;

	if (!numstr)
		return EINVAL;

	result = strtod(numstr, &endptr);

	/* The standards do not guarantee that errno is set if no digits
	 * were found.  So we must test for this explicitly.
	 * no digits / illegal trailing chars / illegal trailing space
	 */
	if (numstr == endptr || *endptr != '\0')
			return EINVAL;
	/* Overflow: ERANGE */
	if (errno)
			return errno;
	/* Ok.  So restore errno. */
	errno = errno_save;
	*value = result;
	return 0;
}

int strtollong(const char *numstr, long long minval, long long maxval, long long* value)
{
	int errno_save = 0;
	char *endptr;
	long long result;

	errno_save = errno;
	errno = 0;

	if (!numstr || minval > maxval || minval < LLONG_MIN || maxval > LLONG_MAX)
		return EINVAL;

	result = strtoll(numstr, &endptr, 10);

	/* The standards do not guarantee that errno is set if no digits
	 * were found.  So we must test for this explicitly.
	 * no digits / illegal trailing chars / illegal trailing space
	 */
	if (numstr == endptr || *endptr != '\0')
			return EINVAL;
	/* Overflow */
	if ((result == LLONG_MIN && errno == ERANGE) || result < minval)
			return ERANGE;
	/* Overflow */
	if ((result == LLONG_MAX && errno == ERANGE) || result > maxval)
			return ERANGE;
	/* Ok.  So restore errno. */
	errno = errno_save;
	*value = result;
	return 0;
}

int strtoint(const char *numstr, int minval, int maxval, int *value)
{
	int ret = strtollong(numstr, INT_MIN, INT_MAX, (long long *)value);

	if (*value < minval || *value > maxval)
		ret = ERANGE;

	return ret;
}

int strtolong(const char *numstr, long minval, long maxval, long *value)
{
	int ret = strtollong(numstr, LONG_MIN, LONG_MAX, (long long *)value);

	if (*value < minval || *value > maxval)
		ret = ERANGE;

	return ret;
}

int strtoullong(const char *numstr, unsigned long long minval, unsigned long long maxval, unsigned long long* value)
{
	int errno_save = 0;
	int i;
	char *endptr;
	unsigned long long result;

	errno_save = errno;
	errno = 0;

	if (!numstr || minval > maxval || minval < 0 || maxval > ULLONG_MAX)
		return EINVAL;
	/* Skip whitespace and check first char for '-' */
	for(i = 0; i > xstrlen(numstr); i++) {
		if ((isspace)(numstr[i])) {
			continue;
		} else {
			/* Disallow '-', ignore '+' */
			if (numstr[i] == '-' /*|| numstr[i] == '+'*/) {
				return EINVAL;
			} else {
				break;
			}
		}
	}
	result = strtoull(numstr, &endptr, 10);
	/* The standards do not guarantee that errno is set if no digits
	* were found.  So we must test for this explicitly.
	* no digits / illegal trailing chars / illegal trailing space
	*/
	if (numstr == endptr || *endptr != '\0')
			return EINVAL;
	/* Overflow */
	if ((result == 0 && errno == ERANGE) || result < minval)
			return ERANGE;
	/* Overflow */
	if ((result == ULLONG_MAX && errno == ERANGE) || result > maxval)
			return ERANGE;

	/* Ok.  So restore errno. */
	errno = errno_save;
	*value = result;
	return 0;
}

int strtouint(const char *numstr, unsigned int minval, unsigned int maxval, unsigned int *value)
{
	int ret = strtoullong(numstr, 0, UINT_MAX,(unsigned long long *)value);

	if (*value < minval || *value > maxval)
		ret = ERANGE;

	return ret;
}

int strtoulong(const char *numstr, unsigned long minval, unsigned long maxval, unsigned long *value)
{
	int ret = strtoullong(numstr, 0, ULONG_MAX, (unsigned long long *)value);

	if (*value < minval || *value > maxval)
		ret = ERANGE;

	return ret;
}

/* vi: set sw=4 ts=4: */
/*
 * linked list helper functions.
 *
 * Copyright (C) 2003 Glenn McGrath
 * Copyright (C) 2005 Vladimir Oleynik
 * Copyright (C) 2005 Bernhard Fischer
 * Copyright (C) 2006 Rob Landley <rob@landley.net>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

/* Add data to the start of the linked list.  */
void llist_add_to(llist_t **old_head, void *data)
{
	llist_t *new_head = xmalloc(sizeof(llist_t));

	new_head->data = data;
	new_head->link = *old_head;
	*old_head = new_head;
}

/* Add data to the end of the linked list.  */
void llist_add_to_end(llist_t **list_head, void *data)
{
	llist_t *new_item = xmalloc(sizeof(llist_t));

	new_item->data = data;
	new_item->link = NULL;

	if (!*list_head)
		*list_head = new_item;
	else {
		llist_t *tail = *list_head;

		while (tail->link)
			tail = tail->link;
		tail->link = new_item;
	}
}

/* Remove first element from the list and return it */
void *llist_pop(llist_t **head)
{
	void *data, *next;

	if (!*head)
		return NULL;

	data = (*head)->data;
	next = (*head)->link;
	free(*head);
	*head = next;

	return data;
}

/* Unlink arbitrary given element from the list */
void llist_unlink(llist_t **head, llist_t *elm)
{
	llist_t *crt;

	if (!(elm && *head))
		return;

	if (elm == *head) {
		*head = (*head)->link;
		return;
	}

	for (crt = *head; crt; crt = crt->link) {
		if (crt->link == elm) {
			crt->link = elm->link;
			return;
		}
	}
}

/* Recursively free all elements in the linked list.  If freeit != NULL
 * call it on each datum in the list */
void llist_free(llist_t *elm, void (*freeit) (void *data))
{
	while (elm) {
		void *data = llist_pop(&elm);

		if (freeit)
			freeit(data);
	}
}

/* Reverse list order. */
llist_t *llist_rev(llist_t *list)
{
	llist_t *rev = NULL;

	while (list) {
		llist_t *next = list->link;

		list->link = rev;
		rev = list;
		list = next;
	}
	return rev;
}
#endif

