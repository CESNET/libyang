#!/bin/sh

cat >&2 <<EOF
The local-deb.sh script has been replaced with a new local-deb.py script,
which is not called from make (i.e. cmake -> make -> local-deb.sh -> dpkg),
but rather started with (local-deb.py -> dpkg -> cmake -> make).  Please
update your build setup as needed.
EOF
exit 1
