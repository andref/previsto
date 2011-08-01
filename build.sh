#!/bin/bash

set -e

# Qt build tools must be available.

if [[ $(which qmake) == '' ]]; then
	echo "Sorry, qmake must be in the PATH."
	exit 1
fi

TIMESTAMP=$(date -u +%Y%m%d-%H%M%S)
BUILDDIR="$(pwd)/.build-$TIMESTAMP"
JOBS=$(cat /proc/cpuinfo | grep "^processor" | wc -l)
VERSION=$(git describe --tags)

if [[ VERSION == '' ]]; then
    VERSION=$TIMESTAMP
fi

# Copy the Debian package environment

mkdir -p $BUILDDIR/root
cp -r ubuntu/* $BUILDDIR/root

# Build the multimarkdown app

pushd dist/peg-multimarkdown
make -i clean
make --jobs=$JOBS
popd

# Build the application

pushd $BUILDDIR
qmake CONFIG+=release ../src/previsto.pro
make --jobs=$JOBS
make install

# Copy the multimarkdown binary

install -s ../dist/peg-multimarkdown/multimarkdown root/usr/lib/previsto

# Compress the changelog

gzip --best root/usr/share/doc/previsto/changelog

# Create the Debian package

fakeroot dpkg-deb --build root
mv root.deb ../previsto-$VERSION.deb

popd

# Get rid of the successful build garbage.

rm -rf $BUILDDIR

