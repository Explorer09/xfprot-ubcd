#!/bin/sh

# This is a build script that can help you create a package for 
# Ultimate Boot CD (UBCD) or Parted Magic.

# By Explorer <explorer09@gmail.com>

cd `dirname $0`
work_dir=`pwd`

package_dir=$work_dir/xfprot-ubcd-pkg
package_name=xfprot-2.4expl1-full.txz

# For Parted Magic the locale directory structure is different, so we are
# going to modify the locales here.
cd ../xfprot
mkdir -p po_bak
mv po/*.po po_bak
mv po/LINGUAS po_bak
cd $work_dir
cp po/*.po ../xfprot/po
cp po/LINGUAS ../xfprot/po

# Configure and make
cd ../xfprot
if [ -f Makefile ]; then
    make distclean
fi
./configure --prefix=/usr --enable-sudo
make
mkdir -p $package_dir
make DESTDIR=$package_dir install-strip

cd $work_dir

# Copy the config files for the root user into the package
mkdir -p $package_dir/root/.xfprot
cp root/xfprot/xfprot.config $package_dir/root/.xfprot/xfprot.config
touch $package_dir/root/.xfprot/xfprot.no_root_warn
touch $package_dir/root/.xfprot/xfprot.no_splash

cd $package_dir

# Don't include these directories because they have no use in Parted Magic.
rm -r usr/share/apps
rm -r usr/share/man

# Create the package.
# Note: the --no-recursion works only for GNU tar.
find . | sort | sed -e '2,$ s/^\.\///g' -e '1 s/^\.$/\.\//' > $work_dir/xfprot-ubcd-pkg.list
tar -c -v --xz --no-recursion --files-from ../xfprot-ubcd-pkg.list -f $package_name
mv $package_dir/$package_name $work_dir

# Clean up.
rm -r $package_dir
rm $work_dir/xfprot-ubcd-pkg.list
cd $work_dir

cd ../xfprot

# Restore the po directory.
rm po/*.po
mv po/LINGUAS
mv po_bak/*.po po
mv po_bak/LINGUAS po
rmdir po_bak


