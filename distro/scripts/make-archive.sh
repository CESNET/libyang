#!/bin/bash
# create archive from current source using git

VERSION=$(grep \(LIBYANG_M.*_VERSION CMakeLists.txt | tr -d '[:alpha:] _()' | sed 'N; N; s/\n/./g')

NAMEVER=libyang-$VERSION
ARCHIVE=$NAMEVER.tar.gz

git archive --format tgz --output $ARCHIVE --prefix $NAMEVER/ HEAD
mkdir -p pkg/archives/dev/
mv $ARCHIVE pkg/archives/dev/

# apkg expects stdout to list archive files
echo pkg/archives/dev/$ARCHIVE
