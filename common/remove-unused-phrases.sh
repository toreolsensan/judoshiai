#!/bin/sh

make po COMMENT=YES

for file in *.po
do
    echo
    echo "*** $file ***"
    ./remove-unused-phrases.py $file $file.tmp
    mv $file.tmp $file
done
