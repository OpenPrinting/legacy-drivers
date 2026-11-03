###--------------- IBM Omni Driver Framework --------------------------###

epclr_h1=$(CONTRIBSRC)defs.h

$(DD)omni.dev : $(DEVOBJ)gomni.$(OBJ) $(DD)page.dev \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)omni $(DEVOBJ)gomni.$(OBJ)

$(DEVOBJ)gomni.$(OBJ) : $(CONTRIBSRC)gomni.c $(epclr_h1) $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(DEVO_)gomni.$(OBJ) $(C_) $(CONTRIBSRC)gomni.c
