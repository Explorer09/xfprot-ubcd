#
# XFPROT Copyright (C) 2002-2009 by Tito Ragusa <tito-wolit@tiscali.it>
#
# XFPROT is a graphical frontend for the F-Prot Antivirus
# for Linux Small Business Edition. It was based upon the
# XSTEP-3.5.1 library until version 0.19beta.
# Starting from version 0.20beta it is based upon Gtk-1.2.10.
# Starting from version 1.0-rc2  it can use  also  Gtk-2.2.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Please read the COPYING and README file!!!
#
# Report bugs to <tito-wolit@tiscali.it>

bin_PROGRAMS = xfprot
dist_bin_SCRIPTS = Xfprot_Scan Xfprot_Update

menudir = @ac_datarootdir@/applications
menu_DATA = xfprot.desktop

konquerorServiceMenudir = @kdeprefix@/share/apps/konqueror/servicemenus
dolphinServiceMenudir =  @kdeprefix@/share/apps/dolphin/servicemenus
konquerorServiceMenu_DATA = xfprot_scan.desktop
dolphinServiceMenu_DATA = xfprot_scan.desktop

#kde4ServiceMenudir = @kde4prefix@/share/kde4/services/ServiceMenus/
#kde4ServiceMenu_DATA = xfprot_scan4.desktop


noinst_HEADERS = xfprot.h\
				 gunixsignal.h\
				 mygtk.h\
				 mylib.h

licensedir = @ac_datarootdir@/xfprot
license_DATA = ../COPYING
man_MANS = xfprot.1

DISTCLEANFILES = xfprot.desktop xfprot_scan.desktop
EXTRA_DIST = $(license_DATA) $(konquerorServiceMenu_DATA) $(dolphinServiceMenu_DATA) $(menu_DATA) xfprot.1

xfprot_SOURCES = about_window.c\
				 dialog_window.c\
				 file_selector.c\
				 my_file_selector.c\
				 gunixsignal.c\
				 mygtk.c\
				 mylib.c\
				 text_common.c\
				 text_edit_window.c\
				 text_view_window.c\
				 xfprot.c

xfprot_LDADD = $(LIBOBJS) @GTK_LIBS@
GTK_ADD_FLAGS = -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
xfprot_CFLAGS = $(GTK_ADD_FLAGS) @GTK_CFLAGS@ -include config.h -include xfprot.h -D_EXTENSIONS__
