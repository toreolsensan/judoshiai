#!/usr/bin/python

import os
import sys
import string
import polib
import csv

try:
    po = polib.pofile(sys.argv[1], encoding='utf-8')
except:
    print sys.argv[1]+": Unexpected error:", sys.exc_info()[0]
    sys.exit(1)

with open(sys.argv[2], 'wb') as f:
    writer = csv.writer(f, quoting=csv.QUOTE_ALL)
    for entry in po:
        a = string.replace(entry.msgid.encode('utf-8'), '\n', '\\n')
        b = string.replace(entry.msgstr.encode('utf-8'), '\n', '\\n')
        writer.writerow([a, b])

