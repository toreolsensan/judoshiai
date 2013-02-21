#!/usr/bin/python

import os
import sys
import string
import csv

with open(sys.argv[2], 'wb') as f:
    writer = csv.writer(f, quoting=csv.QUOTE_ALL)
    with open(sys.argv[1], "rb") as r:
        comment = ""
        for l in r:
            l2 = string.replace(l, '\n', '')
            line = string.replace(l2, '\r', '')
            if line[0] == line[1] == line[2] == '#':
                if len(comment) > 0 :
                    comment = comment +'\n' + line[3:]
                else :
                    comment = line[4:]
            else :
                writer.writerow([comment, line])
                comment = ""

