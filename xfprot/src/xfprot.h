/*
 * XFPROT Copyright (C) 2002-2009 by Tito Ragusa <tito-wolit@tiscali.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef	__HAVE_XFPROT_
#define	__HAVE_XFPROT_ 1
/* NLS Hack for  GtkActionEntry */
//#define N_(STRING)   STRING
#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)
#else
#define _(STRING)    STRING
#endif
#define XFPROT_PRIV_DIR		".xfprot"
#define DEF_REPORT_FILE		"xfprot.log"
#define LOCK_FILE 			"xfprot.lock" 
#define NO_ROOT_WARN_FILE 	"xfprot.no_root_warn" 
#define NO_SPLASH_FILE 		"xfprot.no_splash" 
#define XFPROT_CONFIG_FILE 	"xfprot.config" 
/* These are for the about dialog box */
#define COPYRIGHT			"(C) 2002-2008 GPL Tito Ragusa <tito-wolit@tiscali.it> "
#define HOMEPAGE			"Homepage http://web.tiscali.it/sharp/xfprot/"
#define LICENSE				"GNU General Public License, version 2"
#endif /* __HAVE_XFPROT_ */
