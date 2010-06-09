MSGFMT=msgfmt
XGETTEXT=xgettext

locale: judoshiai-fi_FI.mo judoshiai-sv_SE.mo judoshiai-es_ES.mo

judoshiai-fi_FI.mo: judoshiai-fi_FI.po
	$(MSGFMT) -o judoshiai-fi_FI.mo judoshiai-fi_FI.po 

judoshiai-fi_FI.po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-fi_FI -C --keyword=_ -j ../judotimer/*.c

judoshiai-sv_SE.mo: judoshiai-sv_SE.po
	$(MSGFMT) -o judoshiai-sv_SE.mo judoshiai-sv_SE.po 

judoshiai-sv_SE.po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-sv_SE -C --keyword=_ -j ../judotimer/*.c

judoshiai-es_ES.mo: judoshiai-es_ES.po
	$(MSGFMT) -o judoshiai-es_ES.mo judoshiai-es_ES.po 

judoshiai-es_ES.po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-es_ES -C --keyword=_ -j ../judotimer/*.c

po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-fi_FI -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-sv_SE -C --keyword=_ -j ../judotimer/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-es_ES -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c

clean:
	rm -f *.mo
