include common/Makefile.inc

JUDOSHIAIFILE=judoshiai/$(OBJDIR)/judoshiai$(SUFF)
JUDOTIMERFILE=judotimer/$(OBJDIR)/judotimer$(SUFF)
JUDOINFOFILE=judoinfo/$(OBJDIR)/judoinfo$(SUFF)
JUDOWEIGHTFILE=judoweight/$(OBJDIR)/judoweight$(SUFF)
JUDOJUDOGIFILE=judojudogi/$(OBJDIR)/judojudogi$(SUFF)

RELDIR=$(RELEASEDIR)/judoshiai
RELFILE=$(RELDIR)/bin/judoshiai$(SUFF)
RUNDIR=$(DEVELDIR)

all:
	make -C common
	make -C judoshiai
	make -C judotimer
	make -C judoinfo
	make -C judoweight
	make -C judojudogi
	make -C doc
	rm -rf $(RELDIR)
	mkdir -p $(RELDIR)/bin
	mkdir -p $(RELDIR)/share/locale/
	mkdir -p $(RELDIR)/share/locale/fi/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/sv/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/es/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/et/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/uk/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/is/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/nb/LC_MESSAGES
	mkdir -p $(RELDIR)/share/locale/en_GB/LC_MESSAGES
	mkdir -p $(RELDIR)/lib
	mkdir -p $(RELDIR)/doc
	mkdir -p $(RELDIR)/licenses
	mkdir -p $(RELDIR)/etc
	cp $(JUDOSHIAIFILE) $(RELDIR)/bin/
	cp $(JUDOTIMERFILE) $(RELDIR)/bin/
	cp $(JUDOINFOFILE) $(RELDIR)/bin/
	cp $(JUDOWEIGHTFILE) $(RELDIR)/bin/
	cp $(JUDOJUDOGIFILE) $(RELDIR)/bin/
### Windows executable ###
ifeq ($(TGT),WIN32)
	cp $(RUNDIR)/bin/*.dll $(RELDIR)/bin/
	cp $(SOUNDDIR)/fmodex.dll $(RELDIR)/bin/
	cp $(RSVGDIR)/bin/*.dll $(RELDIR)/bin/
	cp -r $(RUNDIR)/lib/gtk-2.0 $(RELDIR)/lib/
	cp -r $(RUNDIR)/share/locale/fi $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/sv $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/es $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/et $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/uk $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/is $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/nb $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/locale/en_GB $(RELDIR)/share/locale/
	cp -r $(RUNDIR)/share/themes $(RELDIR)/share/
	cp -r $(RUNDIR)/etc $(RELDIR)/
### Linux executable ###
else
	cp $(SOUNDDIR)/lib/libfmodex-*.so $(RELDIR)/lib/libfmodex.so
endif
	cp doc/*.pdf $(RELDIR)/doc/
	cp common/judoshiai-fi_FI.mo $(RELDIR)/share/locale/fi/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-sv_SE.mo $(RELDIR)/share/locale/sv/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-es_ES.mo $(RELDIR)/share/locale/es/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-et_EE.mo $(RELDIR)/share/locale/et/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-uk_UA.mo $(RELDIR)/share/locale/uk/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-is_IS.mo $(RELDIR)/share/locale/is/LC_MESSAGES/judoshiai.mo
	cp common/judoshiai-nb_NO.mo $(RELDIR)/share/locale/nb/LC_MESSAGES/judoshiai.mo
	cp etc/*.png $(RELDIR)/etc/
	cp etc/*.txt $(RELDIR)/etc/
	cp etc/*.css $(RELDIR)/etc/
	cp etc/*.mp3 $(RELDIR)/etc/
	cp etc/*.shi $(RELDIR)/etc/
	cp etc/*.svg $(RELDIR)/etc/
	cp etc/*.html $(RELDIR)/etc/
	cp etc/*.js $(RELDIR)/etc/
	cp -r etc/flags-ioc $(RELDIR)/etc/
	cp licenses/* $(RELDIR)/licenses
	cp -r svg $(RELDIR)/
	@echo
	@echo "To make a setup executable run" 
	@echo "  make setup"
	@echo
	@echo "To make a Debian package run (Linux only)"
	@echo "  sudo make debian" 

#echo 'gtk-theme-name = "MS-Windows"' >$(RELDIR)/etc/gtk-2.0/gtkrc

setup:
ifeq ($(TGT),WIN32)
	sed "s/AppVerName=.*/AppVerName=Shiai $(SHIAI_VER_NUM)/" etc/judoshiai.iss >judoshiai1.iss
	sed "s/OutputBaseFilename=.*/OutputBaseFilename=judoshiai-setup-$(SHIAI_VER_NUM)/" judoshiai1.iss >judoshiai2.iss
	$(INNOSETUP) judoshiai2.iss
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
	ln -sf /usr/lib/judoshiai/bin/judoshiai /usr/bin/judoshiai
	ln -sf /usr/lib/judoshiai/bin/judotimer /usr/bin/judotimer
	ln -sf /usr/lib/judoshiai/bin/judoinfo /usr/bin/judoinfo
	ln -sf /usr/lib/judoshiai/bin/judoweight /usr/bin/judoweight
	ln -sf /usr/lib/judoshiai/bin/judojudogi /usr/bin/judojudogi
	cp gnome/judoshiai.desktop /usr/share/applications/
	cp gnome/judotimer.desktop /usr/share/applications/
	cp gnome/judoinfo.desktop /usr/share/applications/
	cp gnome/judoweight.desktop /usr/share/applications/
	cp gnome/judojudogi.desktop /usr/share/applications/
	cp etc/judoshiai.png /usr/share/pixmaps/
	cp etc/judotimer.png /usr/share/pixmaps/
	cp etc/judoinfo.png /usr/share/pixmaps/
	cp etc/judoweight.png /usr/share/pixmaps/
	cp etc/judojudogi.png /usr/share/pixmaps/
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
	mv *.deb release-linux/
	rm description-pak postinstall-pak postremove-pak

clean:
	make -C common clean
	make -C judoshiai clean
	make -C judotimer clean
	make -C judoinfo clean
	make -C judoweight clean
	make -C judojudogi clean
	make -C doc clean
	rm -rf $(RELEASEDIR)
