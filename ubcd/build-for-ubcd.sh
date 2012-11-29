#!/bin/sh

# This is a build script that can help you create a package for 
# Ultimate Boot CD (UBCD) or Parted Magic.

# By Explorer <explorer09@gmail.com>

cd `dirname $0`
build_dir=`pwd`

package_dir=$build_dir/xfprot-ubcd-pkg
package_name=xfprot-2.4expl1-full.txz

cd ../xfprot
if [ -f Makefile ]; then
    make distclean
fi
./configure --prefix=/usr --enable-sudo
make
mkdir -p $package_dir
make DESTDIR=$package_dir install-strip
cd $build_dir

# Copy the config files for the root user
mkdir -p $package_dir/root/.xfprot
cp root/xfprot/xfprot.config $package_dir/root/.xfprot/xfprot.config
touch $package_dir/root/.xfprot/xfprot.no_root_warn
touch $package_dir/root/.xfprot/xfprot.no_splash

cd $package_dir
find . | sort | sed -e '2,$ s/^\.\///g' -e '1 s/^\.$/\.\//' > ../xfprot-ubcd-pkg.list

# Note: the --no-recursion works only for GNU tar.
tar -c -v --xz --no-recursion --files-from ../xfprot-ubcd-pkg.list -f $package_name

rm ../xfprot-ubcd-pkg.list
cd $build_dir

mv $package_dir/$package_name .

rm -r $package_dir

