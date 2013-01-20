#!/bin/sh

# This is a build script that can help you create a package for 
# Ultimate Boot CD (UBCD) or Parted Magic.

# By Explorer <explorer09@gmail.com>

cd `dirname $0`
work_dir=`pwd`

package_dir=$work_dir/xfprot-ubcd-pkg
package_name=xfprot-2.4expl3.txz

# For Parted Magic the locale directory structure is different, so we are
# going to modify the locales here.
cd ../xfprot
cd po
mv it_IT.UTF-8.po it.po
mv de_DE.UTF-8.po de.po
mv fr_FR.UTF-8.po fr.po
mv pt_BR.UTF-8.po pt_BR.po
mv pl_PL.UTF-8.po pl.po
mv ru_RU.UTF-8.po ru.po
mv zh_TW.UTF-8.po zh_TW.po
mv LINGUAS LINGUAS_bak

cd $work_dir
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
rm -f -r usr/share/apps
rm -f -r usr/share/man

# Create the package.
# Note: the --no-recursion works only for GNU tar.
find . | sort | sed -e '2,$ s/^\.\///g' -e '1 s/^\.$/\.\//' > $work_dir/xfprot-ubcd-pkg.list
tar -c -v --xz --no-recursion --files-from ../xfprot-ubcd-pkg.list -f $package_name
mv $package_dir/$package_name $work_dir

# Clean up.
rm -f -r $package_dir
rm -f $work_dir/xfprot-ubcd-pkg.list
cd $work_dir

cd ../xfprot

# Restore the po directory.
cd po
rm -f *.gmo
mv it.po it_IT.UTF-8.po
mv de.po de_DE.UTF-8.po
mv fr.po fr_FR.UTF-8.po
mv pt_BR.po pt_BR.UTF-8.po
mv pl.po pl_PL.UTF-8.po
mv ru.po ru_RU.UTF-8.po
mv zh_TW.po zh_TW.UTF-8.po
rm -f LINGUAS
mv LINGUAS_bak LINGUAS

echo "Done."
