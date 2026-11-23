### ----------------- The Okidata OkiPage 4w+ device ------------------- ###

oki4w_=$(DEVOBJ)gdevop4w.$(OBJ)
$(DD)oki4w.dev : $(oki4w_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)oki4w $(oki4w_)

# Author: Ivan Schreter (ivan@shadow.sk)
# Printer: Okidata OkiPage 4w+
$(DEVOBJ)gdevop4w.$(OBJ) : $(CONTRIBSRC)gdevop4w.c $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(DEVO_)gdevop4w.$(OBJ) $(C_) $(CONTRIBSRC)gdevop4w.c
