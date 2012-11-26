/*
*
* Gtk+2.x  Utility routines: Text widgets common functions
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
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "mygtk.h"
#include "mylib.h"

/* This code is taken from:
 *
 *  Leafpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
 *  Licensed under the GPL v2 or later.
 */

enum {
	IANA = 0,
	OPENI18N,
	CODEPAGE,
	ENCODING_MAX_ITEM_NUM
};

typedef struct {
	const gchar *item[ENCODING_MAX_ITEM_NUM];
} EncArray;

enum {
	LF = 0x0A,
	CR = 0x0D,
};

#define MAX_COUNTRY_NUM 10

enum {
	LATIN1 = 0,
	LATIN2,
	LATIN3,
	LATIN4,
	LATINC,
	LATINC_UA,
	LATINC_TJ,
	LATINA,
	LATING,
	LATINH,
	LATIN5,
	CHINESE_CN,
	CHINESE_TW,
	CHINESE_HK,
	JAPANESE,
	KOREAN,
	VIETNAMESE,
	THAI,
	GEORGIAN,
	END_CODE
};

static const gchar *country_table[][MAX_COUNTRY_NUM] =
{
	/* LATIN1 */        {NULL},
	/* LATIN2 */        {"cs", "hr", "hu", "pl", "ro", "sk", "sl", "sq", "sr", "uz"},
	/* LATIN3 */        {"eo", "mt", NULL},
	/* LATIN4 */        {"et", "lt", "lv", "mi", NULL},
	/* LATINC */        {"be", "bg", "ky", "mk", "mn", "ru", "tt", NULL},
	/* LATINC_UA */     {"uk", NULL},
	/* LATINC_TJ */     {"tg", NULL},
	/* LATINA */        {"ar", "fa", "ur", NULL},
	/* LATING */        {"el", NULL},
	/* LATINH */        {"he", "yi", NULL},
	/* LATIN5 */        {"az", "tr", NULL},
	/* CHINESE_CN */    {"zh_CN", NULL},
	/* CHINESE_TW */    {"zh_TW", NULL},
	/* CHINESE_HK */    {"zh_HK", NULL},
	/* JAPANESE */      {"ja", NULL},
	/* KOREAN */        {"ko", NULL},
	/* VIETNAMESE */    {"vi", NULL},
	/* THAI */          {"th", NULL},
	/* GEORGIAN */      {"ka", NULL},
};

static const gchar *encoding_table[][ENCODING_MAX_ITEM_NUM] =
{
	                  /*  IANA           OpenI18N       Codepage */
	/* LATIN1 */        { "ISO-8859-1",  "ISO-8859-15", "CP1252" },
	/* LATIN2 */        { "ISO-8859-2",  "ISO-8859-16", "CP1250" },
	/* LATIN3 */        { "ISO-8859-3",  NULL,          NULL },
	/* LATIN4 */        { "ISO-8859-4",  "ISO-8859-13", "CP1257" },
	/* LATINC */        { "ISO-8859-5",  "KOI8-R",      "CP1251" },
	/* LATINC_UA */     { "ISO-8859-5",  "KOI8-U",      "CP1251" },
	/* LATINC_TJ */     { "ISO-8859-5",  "KOI8-T",      "CP1251" },
	/* LATINA */        { "ISO-8859-6",  NULL,          "CP1256" },
	/* LATING */        { "ISO-8859-7",  NULL,          "CP1253" },
	/* LATINH */        { "ISO-8859-8",  NULL,          "CP1255" },
	/* LATIN5 */        { "ISO-8859-9",  NULL,          "CP1254" },
	/* CHINESE_CN */    { "GB2312",      "GB18030",     "CP936" },
	/* CHINESE_TW */    { "BIG5",        "EUC-TW",      "CP950" },
	/* CHINESE_HK */    { "BIG5",        "BIG5-HKSCS",  "CP950" },
	/* JAPANESE */      { "ISO-2022-JP", "EUC-JP",      "CP932" },
	/* KOREAN */        { "ISO-2022-KR", "EUC-KR",      "CP949" },
	/* VIETNAMESE */    { NULL,          "VISCII",      "CP1258" },
	/* THAI */          { NULL,          "TIS-620",     "CP874" },
	/* GEORGIAN */      { NULL,          "GEORGIAN-PS", NULL },
};

guint get_encoding_code(void)
{
	static guint code = END_CODE;
	const gchar *env;
	guint i, j = 1;

	
	DBG_FUNCTION_NAME;
	
	if (code == END_CODE) {
		env = g_getenv("LC_ALL");
		if (!env)
			env = g_getenv("LANG");
		if (env && strlen(env) >= 2)
			while (code == END_CODE && j < END_CODE) {
				for (i = 0; i < MAX_COUNTRY_NUM; i++) {
					if (!country_table[j][i])
						break;
					if (strncmp(env, country_table[j][i], strlen(country_table[j][i])) == 0) {
						code = j;
						break;
					}
				}
				j++;
			}
		if (code == END_CODE)
			code = 0;
	}
	
	return code;
}

EncArray *get_encoding_items(guint code)
{
	gint i;
	static EncArray *array = NULL;

	DBG_FUNCTION_NAME;	
	if (!array) {
		array = g_malloc(sizeof(EncArray));
		for (i = 0; i < ENCODING_MAX_ITEM_NUM; i++)
			array->item[i] = encoding_table[code][i] ?
				encoding_table[code][i] : NULL;
	}
	
	return array;
}

const gchar *get_default_charset(void)
{
	const gchar *charset;
	
	DBG_FUNCTION_NAME;
	g_get_charset(&charset);
	
	return charset;
}

/*
 * Imported from KEdit (for BeOS, NOT KDE).
 * based on http://examples.oreilly.com/cjkvinfo/Ch7/DetectCodeType.c
*/
void convert_line_ending_to_lf(gchar *text)
{
	gint i, j;
	
	DBG_FUNCTION_NAME;
	for (i = 0, j = 0; TRUE; i++, j++) {
		if (*(text + i) == CR) {
			*(text + j) = LF;
			if (*(text + i + 1) == LF)
				i++;
		} else {
			*(text + j) = *(text + i);
			if (*(text + j) == '\0')
				break;
		}
	}
}

void convert_line_ending(gchar **text, gint retcode)
{
	gchar *buf, *str = *text;
	const gint len = strlen(str);
	gint i, j, LFNum = 0;
	
	DBG_FUNCTION_NAME;
	switch (retcode) {
	case CR:
		while (*str != '\0') {
			if (*str == LF)
				*str = CR;
			str++;
		}
		break;
	case CR+LF:
		for (i = 0; *(str + i) != '\0'; i++) {
			if (*(str + i) == LF)
				LFNum++;
		}
		buf = g_new(gchar, len + LFNum + 1);
		for (i= 0, j = 0;; i++, j++) {
			if (*(str + j) == LF) {
				*(buf + i) = CR;
				*(buf + (++i)) = LF;
			} else
				*(buf + i) = *(str + j);
			if (*(str + j) == '\0')
				break;
		}
		g_free(*text);
		*text = buf;
	}
}

gint detect_line_ending(const gchar *text)
{
	DBG_FUNCTION_NAME;

	while (*(text++) != '\0') {
		if (*text == LF)
			break;
		if (*text == CR) {
			if (*(++text) == LF)
				return CR+LF;
			else
				return CR;
		}
	}
	return LF;
}

static const gchar *detect_charset_cylillic(const gchar *text)
{
	guint8 c = *text;
	gboolean noniso = FALSE;
	guint32 xc = 0, xd = 0, xef = 0;
	
	const gchar *charset = get_encoding_items(get_encoding_code())->item[OPENI18N];

	DBG_FUNCTION_NAME;
	
	while ((c = *text++) != '\0') {
		if (c >= 0x80 && c <= 0x9F)
			noniso = TRUE;
		else if (c >= 0xC0 && c <= 0xCF)
			xc++;
		else if (c >= 0xD0 && c <= 0xDF)
			xd++;
		else if (c >= 0xE0)
			xef++;
	}
	
	if (!noniso && ((xc + xef) < xd))
		charset = "ISO-8859-5";
	else if ((xc + xd) < xef)
		charset = "CP1251";
	
	return charset;
}

static const gchar *detect_charset_chinese(const gchar *text)
{
	guint8 c = *text;
	
	const gchar *charset = get_encoding_items(get_encoding_code())->item[IANA];

	DBG_FUNCTION_NAME;
	
	while ((c = *text++) != '\0') {
		if (c >= 0x81 && c <= 0x87) {
			charset = "GB18030";
			break;
		}
		else if (c >= 0x88 && c <= 0xA0) {
			c = *text++;
			if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0)) {
				charset = "GB18030";
				break;
			} //else GBK/Big5-HKSCS cannot determine
		}
		else if ((c >= 0xA1 && c <= 0xC6) || (c >= 0xC9 && c <= 0xF9)) {
			c = *text++;
			if (c >= 0x40 && c <= 0x7E)
				charset = "BIG5";
			else if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0)) {
				charset = "GB18030";
				break;
			}
		}
		else if (c >= 0xC7) {
			c = *text++;
			if ((c >= 0x30 && c <= 0x39) || (c >= 0x80 && c <= 0xA0)) {
				charset = "GB18030";
				break;
			}
		}
	}
	
	return charset;
}

static const gchar *detect_charset_japanese(const gchar *text)
{
	guint8 c = *text;
	gchar *charset = NULL;
	
	DBG_FUNCTION_NAME;

	while (charset == NULL && (c = *text++) != '\0') {
		if (c >= 0x81 && c <= 0x9F) {
			if (c == 0x8E) /* SS2 */ {
				c = *text++;
				if ((c >= 0x40 && c <= 0xA0) || (c >= 0xE0 && c <= 0xFC))
					charset = "CP932";
			}
			else if (c == 0x8F) /* SS3 */ {
				c = *text++;
				if (c >= 0x40 && c <= 0xA0)
					charset = "CP932";
				else if (c >= 0xFD)
					break;
			}
			else
				charset = "CP932";
		}
		else if (c >= 0xA1 && c <= 0xDF) {
			c = *text++;
			if (c <= 0x9F)
				charset = "CP932";
			else if (c >= 0xFD)
				break;
		}
		else if (c >= 0xE0 && c <= 0xEF) {
			c = *text++;
			if (c >= 0x40 && c <= 0xA0)
				charset = "CP932";
			else if (c >= 0xFD)
				break;
		}
		else if (c >= 0xF0)
			break;
	}
	
	if (charset == NULL)
		charset = "EUC-JP";
	
	return charset;
}

static const gchar *detect_charset_korean(const gchar *text)
{
	guint8 c = *text;
	gboolean noneuc = FALSE;
	gboolean nonjohab = FALSE;
	gchar *charset = NULL;
	
	DBG_FUNCTION_NAME;

	while (charset == NULL && (c = *text++) != '\0') {
		if (c >= 0x81 && c < 0x84) {
			charset = "CP949";
		}
		else if (c >= 0x84 && c < 0xA1) {
			noneuc = TRUE;
			c = *text++;
			if ((c > 0x5A && c < 0x61) || (c > 0x7A && c < 0x81))
				charset = "CP1361";
			else if (c == 0x52 || c == 0x72 || c == 0x92 || (c > 0x9D && c < 0xA1)
				|| c == 0xB2 || (c > 0xBD && c < 0xC1) || c == 0xD2
				|| (c > 0xDD && c < 0xE1) || c == 0xF2 || c == 0xFE)
				charset = "CP949";
		}
		else if (c >= 0xA1 && c <= 0xC6) {
			c = *text++;
			if (c < 0xA1) {
				noneuc = TRUE;
				if ((c > 0x5A && c < 0x61) || (c > 0x7A && c < 0x81))
					charset = "CP1361";
				else if (c == 0x52 || c == 0x72 || c == 0x92 || (c > 0x9D && c < 0xA1))
					charset = "CP949";
				else if (c == 0xB2 || (c > 0xBD && c < 0xC1) || c == 0xD2
					|| (c > 0xDD && c < 0xE1) || c == 0xF2 || c == 0xFE)
					nonjohab = TRUE;
			}
		}
		else if (c > 0xC6 && c <= 0xD3) {
			c = *text++;
			if (c < 0xA1)
				charset = "CP1361";
		}
		else if (c > 0xD3 && c < 0xD8) {
			nonjohab = TRUE;
			c = *text++;
		}
		else if (c >= 0xD8) {
			c = *text++;
			if (c < 0xA1)
				charset = "CP1361";
		}
		if (noneuc && nonjohab)
			charset = "CP949";
	}
	
	if (charset == NULL) {
		if (noneuc)
			charset = "CP949";
		else
			charset = "EUC-KR";
	}
	
	return charset;
}

static gboolean detect_noniso(const gchar *text)
{
	guint8 c = *text;

	DBG_FUNCTION_NAME;
	
	while ((c = *text++) != '\0') {
		if (c >= 0x80 && c <= 0x9F)
			return TRUE;
	}
	return FALSE;
}

const gchar *detect_charset(const gchar *text)
{
	guint8 c = *text;
	const gchar *charset = NULL;
	
	DBG_FUNCTION_NAME;

	if (g_utf8_validate(text, -1, NULL)) {
		while ((c = *text++) != '\0') {
			if (c > 0x7F) {
				charset = "UTF-8";
				break;
			}
			if (c == 0x1B) /* ESC */ {
				c = *text++;
				if (c == '$') {
					c = *text++;
					switch (c) {
					case 'B': // JIS X 0208-1983
					case '@': // JIS X 0208-1978
						charset = "ISO-2022-JP";
						continue;
					case 'A': // GB2312-1980
						charset = "ISO-2022-JP-2";
						break;
					case '(':
						c = *text++;
						switch (c) {
						case 'C': // KSC5601-1987
						case 'D': // JIS X 0212-1990
							charset = "ISO-2022-JP-2";
						}
						break;
					case ')':
						c = *text++;
						if (c == 'C')
							charset = "ISO-2022-KR"; // KSC5601-1987
					}
					break;
				}
			}
		}
		if (!charset)
			charset = get_default_charset();
	}
	
	if (!charset) {
		switch (get_encoding_code()) {
		case LATINC:
		case LATINC_UA:
		case LATINC_TJ:
			charset = detect_charset_cylillic(text); // fuzzy...
			break;
		case CHINESE_CN:
		case CHINESE_TW:
		case CHINESE_HK:
			charset = detect_charset_chinese(text);
			break;
		case JAPANESE:
			charset = detect_charset_japanese(text);
			break;
		case KOREAN:
			charset = detect_charset_korean(text);
			break;
		case VIETNAMESE:
		case THAI:
		case GEORGIAN:
			charset = get_encoding_items(get_encoding_code())->item[OPENI18N];
			break;
		default:
			if (detect_noniso(text))
				charset = get_encoding_items(get_encoding_code())->item[CODEPAGE];
			else
				charset = get_encoding_items(get_encoding_code())->item[OPENI18N];
			if (!charset)
				charset = get_encoding_items(get_encoding_code())->item[IANA];					
		}
	}
	return charset;
}

/* End of Leafpad code */

/*
 *  Inspired by files.c of nano editor 1.2.2
 *
 *  Copyright (C) 1999-2003 Chris Allegretta GPL
 *  Licensed under the GPL v2 or later.
 */

FILE *open_file (char *filename )
{
	int fd;
	FILE *file = NULL;
	struct stat fileinfo;

	DBG_FUNCTION_NAME;

	/* We ignore NULL silently as this should have been checked before calling open_file() */
	if(filename == NULL) {
		return NULL;
	}
	
	if (filename[0] == '\0' || stat (filename, &fileinfo) == -1) {
		dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("Unable to stat:\n%s:\n%s"), filename, strerror(errno));
	} else if (S_ISDIR (fileinfo.st_mode) || S_ISCHR (fileinfo.st_mode) || S_ISBLK (fileinfo.st_mode)) {
		/* Don't open character or block files.  Sorry, /dev/sndstat! */
		if(S_ISDIR (fileinfo.st_mode)) {
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("\"%s\" is a directory"), filename);
		} else {
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("\"%s\" is a device file"), filename);
		}
	} else if ((fd = open (filename, O_RDONLY)) == -1) {
		dialog_window (GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, "%s: %s", filename, strerror(errno));
	} else {
		/* File is A-OK */
		/* Binary for our own line-end munging */
		/*The mode string can also include the letter  ``b''  either  as  a  last
		character  or  as a character between the characters in any of the two-
		character strings described above.  This is strictly for  compatibility
		with  ANSI  X3.159-1989  (``ANSI  C'')  and has no effect; the ``b'' is
		ignored on all POSIX conforming systems, including Linux.*/
		file = fdopen (fd, "rb");
		if (file == NULL) {
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, 
						_("Cannot read \"%s\": %s"), filename, strerror(errno));
			close(fd);
		}
	}
	return file;
}

/* End of nano code */

int read_and_convert(char *temp_ptr, GtkTextBuffer *buffer)
{
	char *utf8_temp;
	GtkTextIter iter;
	const char *charset = NULL;

	DBG_FUNCTION_NAME;

	if (!GTK_IS_TEXT_BUFFER(buffer) || !temp_ptr) {
		debug_err_msg("!buffer || !temp_ptr");
		return 1;
	}

	gtk_text_buffer_get_end_iter (buffer, &iter);

	if (g_utf8_validate(temp_ptr , -1, NULL) == TRUE) {
		gtk_text_buffer_insert (buffer, &iter, temp_ptr, -1);
	} else if ((charset = detect_charset(temp_ptr))) {
		/* Try to autodetect the charset of the text */
		debug_err_msg("charset %s", charset);
		if ((utf8_temp = g_convert(temp_ptr, xstrlen(temp_ptr),"UTF-8", charset, NULL, NULL, NULL))) {
			gtk_text_buffer_insert (buffer, &iter, utf8_temp, -1);
			xfree(&utf8_temp);
		} else {
			dialog_window(GTK_STOCK_DIALOG_ERROR, ONE_BUTTON, NULL, 1, _("Conversion to UTF-8 failed!"));
			return 1;
		}
	} else {
		/* Last chance, fallback */
		gtk_text_buffer_insert (buffer, &iter, temp_ptr, -1);
	}
	return 0;
}

void read_from_stream_and_display_text(GtkWidget *text, FILE *fd, unsigned long filesize, gdouble *progress)
{

	char *temp_ptr;
	GtkTextBuffer *buffer;
	float step = 0.0;
	int ret;

	#define _TEXT_BUF_SIZE 	8192	

	DBG_FUNCTION_NAME;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	step = (filesize > 0 && fd != stdin) ? (float) (1 / ((float) filesize / _TEXT_BUF_SIZE)) : 0.0;

	while ((temp_ptr =  xfgetc_nbuf(fd, _TEXT_BUF_SIZE))) {
		refresh_gui();
		ret = read_and_convert(temp_ptr, buffer);
		xfree(&temp_ptr);
		if (ret)
			break;
		if (progress)
			*progress += step;
	}
}

void clear_the_buffer(GtkWidget *text, int *text_modified)
{
	GtkTextBuffer	*buffer;
	GtkTextIter		start;
	GtkTextIter		end;

	DBG_FUNCTION_NAME;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_delete(buffer, &start, &end);
	RESET(*text_modified);
}
