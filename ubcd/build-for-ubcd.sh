#!/bin/sh

# This is a build script that can help you create a package for 
# Ultimate Boot CD (UBCD) or Parted Magic.

# By Explorer <explorer09@gmail.com>

cd `dirname $0`
work_dir=`pwd`
xfprot_dir=$work_dir/../xfprot

package_dir=$work_dir/xfprot-ubcd-pkg
package_name=xfprot-2.4expl6.txz

# restore_xfprot restore the xfprot directory to its original state except
# it does not run 'make distclean'.
# Used in sighandler below, so be careful when editing this function.
restore_xfprot () {
    # Restore the po directory.
    cd $xfprot_dir/po
    rm -f *.gmo
    mv it.po    it_IT.UTF-8.po
    mv de.po    de_DE.UTF-8.po
    mv fr.po    fr_FR.UTF-8.po
    mv es.po    es_ES.UTF-8.po
    mv pt_BR.po pt_BR.UTF-8.po
    mv pl.po    pl_PL.UTF-8.po
    mv ru.po    ru_RU.UTF-8.po
    mv ja.po    ja_JP.UTF-8.po
    mv zh_CN.po zh_CN.UTF-8.po
    mv zh_TW.po zh_TW.UTF-8.po
    if [ -f LINGUAS_bak ]; then
        rm -f LINGUAS
        mv LINGUAS_bak LINGUAS
    fi
}

# sighandler is a signal handler for SIGHUP, SIGINT (^C), SIGQUIT (Ctrl-\),
# and SIGTERM.
sighandler () {
    trap - 1 2 3 15
    restore_xfprot
    echo "build-for-ubcd: Terminated by signal."
    exit 1
}

# make_package creates the .txz package. This section may be run in a fakeroot
# environment.
make_package () {
    cd $package_dir

    chmod 755 $package_dir
    chmod 755 root
    chmod 755 root/.xfprot
    chmod 644 root/.xfprot/xfprot.config
    chmod 644 root/.xfprot/xfprot.no_root_warn
    chmod 644 root/.xfprot/xfprot.no_splash

    # Override the user's umask setting and force all directories in the
    # package to have mode 755.
    find . -type d -exec chmod 755 '{}' \;

    chown -h 0:0 .
    chown -Rh 0:0 *

    # Don't include these directories because they have no use in Parted Magic.
    rm -f -r usr/share/apps
    rm -f -r usr/share/man

    # Create the package.
    # Note: the --no-recursion works only for GNU tar.
    find . | LC_ALL=C sort | sed -e '2,$ s/^\.\///g' -e '1 s/^\.$/\.\//' | \
    tar -c -v --xz --no-recursion --files-from - -f $package_name
}

# -----------------------------------------------------------------------------

if [ "x$1" = "x_make_package" ]; then
    make_package
    exit 0
fi

# For Parted Magic the locale directory structure is different, so we are
# going to modify the locales here.
cd $xfprot_dir/po
mv it_IT.UTF-8.po it.po
mv de_DE.UTF-8.po de.po
mv fr_FR.UTF-8.po fr.po
mv es_ES.UTF-8.po es.po
mv pt_BR.UTF-8.po pt_BR.po
mv pl_PL.UTF-8.po pl.po
mv ru_RU.UTF-8.po ru.po
mv ja_JP.UTF-8.po ja.po
mv zh_CN.UTF-8.po zh_CN.po
mv zh_TW.UTF-8.po zh_TW.po
mv LINGUAS LINGUAS_bak

cd $work_dir
cp po/LINGUAS $xfprot_dir/po

trap 'sighandler' 1 2 3 15

# Configure and make
cd $xfprot_dir
if [ -f Makefile ]; then
    make distclean
fi
./configure --prefix=/usr --enable-sudo && make

if [ "$?" -eq "0" ]; then
    mkdir -p $package_dir
    make DESTDIR=$package_dir install-strip

    cd $package_dir

    # Copy the config files for the root user into the package
    mkdir -p root/.xfprot
    cp $work_dir/root/xfprot/xfprot.config root/.xfprot/xfprot.config
    touch root/.xfprot/xfprot.no_root_warn
    touch root/.xfprot/xfprot.no_splash

    if [ `id -u` = "0" ]; then
        make_package
    elif [ `which fakeroot` ]; then
        echo "build-for-ubcd: Faking root privileges when possible..."
        fakeroot -- "./`basename $0`" _make_package
    else
        echo "build-for-ubcd: Root privilege or a fakeroot environment is needed to create" >&2
        echo "the package." >&2
        trap - 1 2 3 15
        restore_xfprot
        exit 1
    fi

    mv $package_dir/$package_name $work_dir

    # Clean up.
    rm -f -r $package_dir
    rm -f $work_dir/xfprot-ubcd-pkg.list

    trap - 1 2 3 15
    restore_xfprot
    echo "build-for-ubcd: Done."
    exit 0
else
    echo "build-for-ubcd: An error occurred during the compilation. $package_name is not created." >&2
    trap - 1 2 3 15
    restore_xfprot
    exit 1
fi
