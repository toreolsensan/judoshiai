#!/usr/bin/python

import os
import sys
import string
import csv

with open(sys.argv[2], 'wb') as f:
    with open(sys.argv[1], 'rb') as csvfile:
        rdr = csv.reader(csvfile, delimiter=',', quotechar='"')
        for row in rdr:
            a = string.replace(row[0], '\n', '\n### ')
            b = row[1]
            f.write('### '+a+'\n'+b+'\n')
