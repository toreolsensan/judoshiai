#!/usr/bin/python

import os
import sys
import string
import polib
import csv

po = polib.POFile()
po.metadata = {
    'Project-Id-Version': 'PACKAGE VERSION',
    'Report-Msgid-Bugs-To': 'oh2ncp@kolumbus.fi',
    'POT-Creation-Date': '2007-10-18 14:00+0100',
    'PO-Revision-Date': '2007-10-18 14:00+0100',
    'Last-Translator': 'FULL NAME <EMAIL@ADDRESS>',
    'Language-Team': 'LANGUAGE <LL@li.org>',
    'MIME-Version': '1.0',
    'Content-Type': 'text/plain; charset=utf-8',
    'Content-Transfer-Encoding': '8bit',
}

with open(sys.argv[1], 'rb') as csvfile:
    rdr = csv.reader(csvfile, delimiter=',', quotechar='"')
    for row in rdr:
        a = string.replace(string.replace(row[0], '\n', ''), '\\n', '\n').decode('utf-8')
        b = string.replace(string.replace(row[1], '\n', ''), '\\n', '\n').decode('utf-8')
        entry = polib.POEntry(msgid=a, msgstr=b)
        po.append(entry)

po.save(sys.argv[2])

