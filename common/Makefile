MSGFMT=msgfmt
XGETTEXT=xgettext

locale: judoshiai-fi_FI.mo judoshiai-sv_SE.mo judoshiai-es_ES.mo judoshiai-et_EE.mo \
	judoshiai-uk_UA.mo judoshiai-is_IS.mo

judoshiai-fi_FI.mo: judoshiai-fi_FI.po
	$(MSGFMT) -o judoshiai-fi_FI.mo judoshiai-fi_FI.po 

judoshiai-sv_SE.mo: judoshiai-sv_SE.po
	$(MSGFMT) -o judoshiai-sv_SE.mo judoshiai-sv_SE.po 

judoshiai-es_ES.mo: judoshiai-es_ES.po
	$(MSGFMT) -o judoshiai-es_ES.mo judoshiai-es_ES.po 

judoshiai-et_EE.mo: judoshiai-et_EE.po
	$(MSGFMT) -o judoshiai-et_EE.mo judoshiai-et_EE.po 

judoshiai-uk_UA.mo: judoshiai-uk_UA.po
	$(MSGFMT) -o judoshiai-uk_UA.mo judoshiai-uk_UA.po 

judoshiai-is_IS.mo: judoshiai-is_IS.po
	$(MSGFMT) -o judoshiai-is_IS.mo judoshiai-is_IS.po 

po:
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-fi_FI -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-sv_SE -C --keyword=_ -j ../judotimer/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-es_ES -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-et_EE -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-uk_UA -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c
	$(XGETTEXT) --from-code=UTF-8 -d judoshiai-is_IS -C --keyword=_ -j ../judotimer/*.c -j ../judoshiai/*.c -j ../judoinfo/*.c ../common/*.c

uncomment:
	grep -v '^#' judoshiai-fi_FI.po >judoshiai-fi_FI.tmp
	cp judoshiai-fi_FI.tmp judoshiai-fi_FI.po 
	grep -v '^#' judoshiai-sv_SE.po >judoshiai-sv_SE.tmp
	cp judoshiai-sv_SE.tmp judoshiai-sv_SE.po 
	grep -v '^#' judoshiai-es_ES.po >judoshiai-es_ES.tmp
	cp judoshiai-es_ES.tmp judoshiai-es_ES.po 
	grep -v '^#' judoshiai-et_EE.po >judoshiai-et_EE.tmp
	cp judoshiai-et_EE.tmp judoshiai-et_EE.po 
	grep -v '^#' judoshiai-uk_UA.po >judoshiai-uk_UA.tmp
	cp judoshiai-uk_UA.tmp judoshiai-uk_UA.po 
	grep -v '^#' judoshiai-is_IS.po >judoshiai-is_IS.tmp
	cp judoshiai-is_IS.tmp judoshiai-is_IS.po 

clean:
	rm -f *.mo
