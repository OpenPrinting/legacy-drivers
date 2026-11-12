#	dmprt	 dot matrix printer driver for Ghostscript (it can use 
#		 dviprt printer config files, japanese)

### ---------------- Dot matrix printer device ---------------- ###
dmprt_=$(DEVOBJ)gdevdmpr.$(OBJ) $(DEVOBJ)dviprlib.$(OBJ) $(DEVOBJ)gdevprn.$(OBJ)

$(DD)dmprt.dev : $(dmprt_) $(DD)page.dev \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETDEV) $(DD)dmprt $(dmprt_)
	$(ADDMOD) $(DD)dmprt -ps dmp_init

$(DEVOBJ)gdevdmpr.$(OBJ) : $(JAPSRC)gdevdmpr.c $(JAPSRC)dviprlib.h $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(O_)$@ $(C_) $(JAPSRC)gdevdmpr.c

$(DEVOBJ)dviprlib.$(OBJ) : $(JAPSRC)dviprlib.c $(JAPSRC)dviprlib.h \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(O_)$@ $(C_) $(JAPSRC)dviprlib.c

extra-dmprt-install: install-libdata
	$(INSTALL_DATA) $(JAPSRC)dmp_init.ps $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
	$(INSTALL_DATA) $(JAPSRC)dmp_site.ps $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
	$(INSTALL_DATA) $(JAPSRC)escp_24.src $(DESTDIR)$(gsdatadir)$(D)lib || exit 1
