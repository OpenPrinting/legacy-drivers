### ----------------- The Xerox XES printer device --------------------- ###

xes_=$(DEVOBJ)gdevxes.$(OBJ)
$(DD)xes.dev : $(xes_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)xes $(xes_)

# Author: Peter Flass (flass@lbdrscs.bitnet)
# Printer: Xerox XES (2700, 3700, 4045, etc.)
$(DEVOBJ)gdevxes.$(OBJ) : $(CONTRIBSRC)gdevxes.c $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(DEVO_)gdevxes.$(OBJ) $(C_) $(CONTRIBSRC)gdevxes.c
