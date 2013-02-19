MSGFMT=msgfmt
XGETTEXT=xgettext
pos := $(wildcard *.po)
mos := $(patsubst %.po,%.mo,$(pos))
tmps := $(patsubst %.po,%.tmp,$(pos))

locale: $(mos)

%.mo: %.po
	$(MSGFMT) -o $@ $< 

%.po: FORCE
	$(XGETTEXT) --from-code=UTF-8 -d $* -C --keyword=_ --keyword=N_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c ../judoweight/*.c ../judojudogi/*.c

FORCE:

%.tmp: %.po
	grep -v '^#' $< > $@
	mv $@ $< 

po: $(tmps)

clean:
	rm -f *.mo

# for future use
#merge:
#	echo '' > messages.po # xgettext needs that file, and we need it empty
#	find .. -type f -iname "*.c" | xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ -j -f -
#	msgmerge -N existing.po messages.po > new.po
#	mv new.po existing.po
#	rm messages.po
