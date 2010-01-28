MSGFMT=msgfmt
XGETTEXT=xgettext

locale: judoshiai-fi_FI.mo judoshiai-sv_SE.mo

judoshiai-fi_FI.mo: judoshiai-fi_FI.po
	$(MSGFMT) -o judoshiai-fi_FI.mo judoshiai-fi_FI.po 

judoshiai-fi_FI.po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-fi_FI -C --keyword=_ -j ../judotimer/*.c

judoshiai-sv_SE.mo: judoshiai-sv_SE.po
	$(MSGFMT) -o judoshiai-sv_SE.mo judoshiai-sv_SE.po 

judoshiai-sv_SE.po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-sv_SE -C --keyword=_ -j ../judotimer/*.c

po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-fi_FI -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-sv_SE -C --keyword=_ -j ../judotimer/*.c

clean:
	rm -f *.mo
