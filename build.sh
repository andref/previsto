#!/bin/bash

set -e

# Qt build tools must be available.

if [[ $(which qmake) == '' ]]; then
	echo "Sorry, qmake must be in the PATH."
	exit 1
fi

TIMESTAMP=$(date -u +%Y%m%d-%H%M%S)
BUILDDIR="$(pwd)/.build-$TIMESTAMP"
JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)

# Create our make believe fs root

mkdir -p $BUILDDIR/root/DEBIAN
mkdir -p $BUILDDIR/root/usr/bin
mkdir -p $BUILDDIR/root/usr/lib/previsto
mkdir -p $BUILDDIR/root/usr/share/doc/previsto
mkdir -p $BUILDDIR/root/usr/share/applications
mkdir -p $BUILDDIR/root/usr/share/icons/hicolor

# Build the multimarkdown app

pushd dist/peg-multimarkdown
make --jobs=$JOBS
popd

# Build the application

pushd $BUILDDIR
qmake CONFIG+=release ../src/previsto.pro
make --jobs=$JOBS
make install

# Copy the Debian control files

cp ../ubuntu/control root/DEBIAN
cp ../ubuntu/postinst root/DEBIAN
cp ../ubuntu/prerm root/DEBIAN

# Copy the multimarkdown binary

install -s ../dist/peg-multimarkdown/multimarkdown root/usr/lib/previsto

# Copy xdg files

cp ../ubuntu/copyright root/usr/share/doc/previsto
cp ../ubuntu/changelog root/usr/share/doc/previsto
cp ../ubuntu/previsto.desktop root/usr/share/applications
cp -r ../ubuntu/icons/* root/usr/share/icons/hicolor

gzip --best root/usr/share/doc/previsto/changelog

# Create the Debian package

fakeroot dpkg-deb --build root
mv root.deb ../previsto-$TIMESTAMP.deb

popd

# Get rid of the successful build garbage.

rm -rf $BUILDDIR
