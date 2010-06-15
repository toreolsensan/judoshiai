include common/Makefile.inc

JUDOSHIAIFILE=judoshiai/judoshiai$(SUFF)
JUDOTIMERFILE=judotimer/judotimer$(SUFF)
JUDOINFOFILE=judoinfo/judoinfo$(SUFF)
LANGUAGEFILE=common/judoshiai-fi_FI.mo
FLASHFILE=flash/judotimer.swf
DOCFILE=doc/judoshiai.pdf
DOCFILE2=doc/judotimer.pdf
RELEASEDIR=release
RELDIR=$(RELEASEDIR)/judoshiai
RELFILE=$(RELDIR)/bin/judoshiai$(SUFF)
RUNDIR=$(DEVELDIR)

# To do: Add $(FLASHFILE) to targets

all: $(JUDOSHIAIFILE) $(JUDOTIMERFILE) $(JUDOINFOFILE) $(LANGUAGEFILE) $(DOCFILE) $(DOCFILE2)
	rm -rf $(RELDIR)
	mkdir -p $(RELDIR)/bin
	mkdir -p $(RELDIR)/share/locale/
	mkdir -p $(RELDIR)/share/locale/fi/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/sv/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/es/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/en_GB/LC_MESSAGES
	mkdir -p $(RELDIR)/lib
	mkdir -p $(RELDIR)/doc
	mkdir -p $(RELDIR)/licenses
	mkdir -p $(RELDIR)/etc
	cp $(JUDOSHIAIFILE) $(RELDIR)/bin/
	cp $(JUDOTIMERFILE) $(RELDIR)/bin/
	cp $(JUDOINFOFILE) $(RELDIR)/bin/
ifeq ($(OS),Windows_NT)
	cp $(RUNDIR)/bin/*.dll $(RELDIR)/bin/
	cp $(SOUNDDIR)/fmodex.dll $(RELDIR)/bin/
	cp -r $(RUNDIR)/lib/gtk-2.0 $(RELDIR)/lib/
	cp -r $(RUNDIR)/share/locale/fi $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/sv $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/es $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/en_GB $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/themes $(RELDIR)/share/
	cp -r $(RUNDIR)/etc $(RELDIR)/
else
	cp $(SOUNDDIR)/lib/libfmodex-*.so $(RELDIR)/lib/libfmodex.so
endif
	cp doc/judoshiai.pdf $(RELDIR)/doc/
	cp doc/judotimer.pdf $(RELDIR)/doc/
	cp common/judoshiai-fi_FI.mo $(RELDIR)/share/locale/fi/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-sv_SE.mo $(RELDIR)/share/locale/sv/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-es_ES.mo $(RELDIR)/share/locale/es/LC_MESSAGES/judoshiai.mo
	cp etc/*.png $(RELDIR)/etc/
	cp etc/*.txt $(RELDIR)/etc/
	cp etc/*.css $(RELDIR)/etc/
	cp etc/*.mp3 $(RELDIR)/etc/
	#cp flash/judo*.swf flash/judo*.html $(RELDIR)/etc/
	cp licenses/* $(RELDIR)/licenses
	@echo
	@echo "To make a setup executable run" 
	@echo "  make setup"
	@echo
	@echo "To make a Debian package run (Linux only)"
	@echo "  sudo make debian" 

#echo 'gtk-theme-name = "MS-Windows"' >$(RELDIR)/etc/gtk-2.0/gtkrc

$(JUDOSHIAIFILE): judoshiai/*.c common/*.c
	make -C judoshiai

$(JUDOTIMERFILE): judotimer/*.c common/*.c
	make -C judotimer

$(JUDOINFOFILE): judoinfo/*.c common/*.c
	make -C judoinfo

$(LANGUAGEFILE): common/*.po
	make -C common

$(DOCFILE) $(DOCFILE2): doc/judoshiai.odt doc/judotimer.odg
	make -C doc

$(FLASHFILE): flash/judoshiai/*.hx
	make -C flash

setup:
ifeq ($(OS),Windows_NT)
	sed "s/AppVerName=.*/AppVerName=Shiai $(SHIAI_VER_NUM)/" etc/judoshiai.iss >judoshiai1.iss
	sed "s/OutputBaseFilename=.*/OutputBaseFilename=judoshiai-setup-$(SHIAI_VER_NUM)/" judoshiai1.iss >judoshiai2.iss
	/c/Program\ Files/Inno\ Setup\ 5/ISCC.exe judoshiai2.iss
	rm -f judoshiai*.iss
else
	tar -C $(RELEASEDIR) -cf judoshiai.tar judoshiai
	tar -C etc -rf judoshiai.tar install.sh
	gzip judoshiai.tar
	cat etc/header.sh judoshiai.tar.gz >$(RELEASEDIR)/judoshiai-setup-$(SHIAI_VER_NUM).bin
	chmod a+x $(RELEASEDIR)/judoshiai-setup-$(SHIAI_VER_NUM).bin
	rm -f judoshiai.tar.gz
endif

install:
	cp -r $(RELDIR) /usr/lib/
	ln -s /usr/lib/judoshiai/bin/judoshiai /usr/bin/judoshiai
	ln -s /usr/lib/judoshiai/bin/judotimer /usr/bin/judotimer
	ln -s /usr/lib/judoshiai/bin/judoinfo /usr/bin/judoinfo
	cp gnome/judoshiai.desktop /usr/share/applications/
	cp gnome/judotimer.desktop /usr/share/applications/
	cp gnome/judoinfo.desktop /usr/share/applications/
	cp etc/judoshiai.png /usr/share/pixmaps/
	cp etc/judotimer.png /usr/share/pixmaps/
	cp etc/judoinfo.png /usr/share/pixmaps/
	cp gnome/judoshiai.mime /usr/share/mime-info/
	cp gnome/judoshiai.keys /usr/share/mime-info/
	cp gnome/judoshiai.applications /usr/share/application-registry/
	cp gnome/judoshiai.packages /usr/lib/mime/packages/judoshiai
	cp gnome/judoshiai.xml /usr/share/mime/packages/
	cp gnome/judoshiai.menu /usr/share/menu/judoshiai

debian:
	cp gnome/*-pak .
	checkinstall -D --install=no --pkgname=judoshiai --pkgversion=$(SHIAI_VER_NUM) \
	--maintainer=oh2ncp@kolumbus.fi --nodoc 
	mv *.deb release/
	rm description-pak postinstall-pak postremove-pak

clean:
	make -C common clean
	make -C judoshiai clean
	make -C judotimer clean
	make -C judoinfo clean
	make -C doc clean
	#make -C flash clean
	rm -rf $(RELEASEDIR)
