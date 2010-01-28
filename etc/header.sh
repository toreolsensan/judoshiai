#!/bin/sh

export WRKDIR=`mktemp -d /tmp/shiai.XXXXXX`

SKIP=`awk '/^__ARCHIVE_FOLLOWS__/ { print NR + 1; exit 0; }' $0`

tail -n +$SKIP $0 | tar xz -C $WRKDIR

PREV=`pwd`
cd $WRKDIR
./install.sh

cd $PREV
rm -rf $WRKDIR

exit 0

__ARCHIVE_FOLLOWS__
