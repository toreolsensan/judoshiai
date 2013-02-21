#!/usr/bin/python

import os
import sys
import string

comment = ok = 0

with open(sys.argv[1], 'rb') as f:
    with open(sys.argv[2], 'wb') as t:
        for line in f:
            if line[0] == '#':
                comment = 1
            elif line[0:5] == 'msgid':
                ok = comment
                comment = 0

            if ok == 1 or comment == 1:
                t.write(line)
            else:
                print 'UNUSED:',line,

