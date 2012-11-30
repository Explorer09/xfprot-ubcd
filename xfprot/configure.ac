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

# Process this file with autoconf to produce a configure script.
# require autoconf 2.54
AC_PREREQ(2.54)
AC_INIT(XFPROT, 2.4expl3, "tito-wolit@tiscali.it", xfprot)

# Save this value here, since automake will set cflags later
cflags_set=${CFLAGS+set}

AM_INIT_AUTOMAKE(no-define)
AM_CONFIG_HEADER(src/config.h)

IT_PROG_INTLTOOL([0.23],[no-xml])
AC_CHECK_HEADERS([libintl.h])
GETTEXT_PACKAGE=xfprot
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_CANONICAL_HOST

case {$host} in
	*linux*)
		AC_DEFINE([LINUX],[1],[needed for optional declarations to be visible])
		ostype=linux
		;;
	*-*-solaris2*)
		AC_DEFINE([SOLARIS],[1],[needed for optional declarations to be visible])
		ostype=solaris
		;;
	*)
		AC_MSG_RESULT(Unsupported operating system: ${host})
		ostype=unknown
		;;
esac

# Enable debugging argument option.
AC_ARG_ENABLE(debug,AC_HELP_STRING([--enable-debug], [enable debugging (default=no)]),	[ac_debug="$enableval"])

if test "x$ac_debug" = "xyes"; then
	AC_DEFINE(DEBUG, [], [Define if debugging is enabled.])
	CFLAGS+="-Wall -Wunused -Wshadow -Wmissing-noreturn -Wunused-function \
			 -Wunused-label  -Wunused-parameter -Wunused-value  -Wunused-variable \
			 -fstrict-aliasing -Wtrigraphs -Wmissing-format-attribute  -Wcomment \
			 -Wredundant-decls  -Wformat=2 -O3 -fno-builtin -Wformat-y2k \
			 -Wno-format-extra-args -Wno-format-zero-length -Wformat-security -funsigned-char"
	ac_debug="debug"
else
	CFLAGS+=" -DNDEBUG -O3 -pipe -fomit-frame-pointer -fno-builtin"
	ac_debug="release"
fi

# Build time sanity check...
AM_SANITY_CHECK

# Checks for programs.
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_HEADER_STDC
AC_LANG_C

changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac

  if test "x$enable_ansi" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-ansi[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -ansi" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-pedantic[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -pedantic" ;;
    esac
  fi
fi
changequote([,])dnl

# i18n stuff
ALL_LINGUAS="`grep -v '^#' "$srcdir/po/LINGUAS" | tr '\n' ' '`"
AM_GLIB_GNU_GETTEXT

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
	ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)

ac_datarootdir=`eval "echo $datarootdir"`
if test "x$ac_datarootdir" = "xNONE/share"; then
	ac_datarootdir=$ac_prefix/share
fi
AC_SUBST(ac_datarootdir)
AC_DEFINE_UNQUOTED(DATADIR, ["$ac_datarootdir"], [Data dir.])
AC_DEFINE_UNQUOTED(LICENSE_FILE, ["$ac_datarootdir/xfprot/COPYING"], [License file.])
AC_DEFINE_UNQUOTED(LOCALEDIR, ["$ac_datarootdir/locale"], [Locale dir.])

ac_elevator="none"

# Prefer use of sudo if available for updates
AC_ARG_ENABLE(sudo,
	AC_HELP_STRING([--enable-sudo], [prefer use of sudo if available for updates]), ac_elevator="sudo")

# Prefer use of su if availbale for updates
AC_ARG_ENABLE(su,
	AC_HELP_STRING([--enable-su], [prefer use of su if availbale for updates]), ac_elevator="su")

# Try to autodetect, preferring sudo if found
if test "x$ac_elevator" = "xnone"; then
	AC_PATH_PROG(ac_elevator, sudo, none)
	if test "x$ac_elevator" = "xnone"; then
		# We need one more var here as configure else  uses the cached value
		# of the previous test.
		AC_PATH_PROG(ac_elevator2, su, none)
		if test "x$ac_elevator2" = "xnone"; then
			ac_elevator="sudo"
			AC_MSG_WARN([
Found nor su neither sudo.
The su and sudo commands are not installed on
your system, or are not in your path variable.
Please install one of them or fix your path variable
The update button will not work correctly.
Using default value for 'sudo'.
You can change this values by editing manually config.h.
			])
		else
			ac_elevator=$ac_elevator2
		fi
	fi
fi

ac_elevator=`basename $ac_elevator`

if test "x$ac_elevator" = "xsudo"; then
	AC_DEFINE(PREFER_SUDO, [], [Prefer use of sudo if available for updates.])
fi

dnl it will define GTK_CFLAGS and GTK_LIBS for us,
dnl comes from gtk.m4 (aclocal will find it for us)
AM_PATH_GTK_2_0(2.18.0)

gtk_config_major_version=`$PKG_CONFIG --modversion gtk+-2.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
gtk_config_minor_version=`$PKG_CONFIG --modversion gtk+-2.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
gtk_config_micro_version=`$PKG_CONFIG --modversion gtk+-2.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

if test "x$ac_tooltip" = "x1"; then
	AC_DEFINE(USE_OLD_TOOLTIP, [], [Old/new gtk tooltips.])
fi

# since 2.12
AC_CHECK_LIB(gtk-x11-2.0, gtk_widget_set_tooltip_text, ac_tooltip=1, ac_tooltip=0)

if test "x$ac_tooltip" = "x0"; then
	AC_DEFINE(USE_OLD_TOOLTIP, [], [Old/new gtk tooltips.])
fi

AC_CHECK_FUNC(prctl, ac_prctl=yes, ac_prctl=no)
if test "x$ac_prctl" = "xyes"; then
	AC_DEFINE(HAVE_PRCTL, [], [Core dumps allowed.])
	ac_prctl="not allowed"
else
	ac_prctl="allowed"
fi

AC_ARG_WITH(kdeprefix,
	[AC_HELP_STRING(--with-kdeprefix=PATH,
		[prefix for KDE libraries for KDE3 desktop integration])],
		kdeprefix=$withval,
		kdeprefix="none")
if test "x$kdeprefix" = "xnone"; then
	AC_PATH_PROG(ac_kdeconfig, kde-config, "no")
	if test "x$ac_kdeconfig" = "xno"; then
		AC_MSG_WARN([kde-config............not found.
Setting kdeprefix to default /usr, use --with-kdeprefix=PATH to override."])
		kdeprefix=/usr
	else
		kdeprefix=`kde-config --prefix`
	fi
fi
AC_SUBST(kdeprefix)

#AC_ARG_WITH(kde4prefix,
#	[AC_HELP_STRING(--with-kde4prefix=PATH,
#		[prefix for KDE4 libraries for KDE4 desktop integration])],
#		kde4prefix=$withval,
#		kde4prefix="none")
#if test "x$kde4prefix" = "xnone"; then
#	AC_PATH_PROG(ac_kde4config, kde4-config, "no")
#	if test "x$ac_kde4config" = "xno"; then
#		AC_MSG_WARN([kde4-config............not found.
#Setting kde4prefix to default /usr, use --with-kde4prefix=PATH to override."])
#		kde4prefix=/usr
#	else
#		kde4prefix=`kde-config --prefix`
#	fi
#fi
#AC_SUBST(kde4prefix)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/xfprot.desktop
src/xfprot_scan.desktop
po/Makefile.in
icons/Makefile
])

AC_OUTPUT

echo
echo "  $PACKAGE_NAME $PACKAGE_VERSION"
echo
echo "  Host . . . . . . . . . . . . . . . . . . . . . . .: $host"
echo "  OS . . . . . . . . . . . . . . . . . . . . . . . .: $ostype"
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo "  Datarootdir  . . . . . . . . . . . . . . . . . . .: $ac_datarootdir"
echo "  Gtk+ library version . . . . . . . . . . . . . . .: $gtk_config_major_version.$gtk_config_minor_version.$gtk_config_micro_version"
echo "  Preferred command to become superuser. . . . . . .: $ac_elevator"
echo "  Core dumps . . . . . . . . . . . . . . . . . . . .: $ac_prctl"
echo "  KDE3 prefix for desktop integration  . . . . . . .: $kdeprefix"
#echo "  KDE4 prefix for desktop integration. . . . . . . .: $kde4prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
