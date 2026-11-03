### -------------------- The Mitsubishi CP50 printer -------------------- ###
### Note: this driver was contributed by a user: please contact           ###
###       Michael Hu (michael@ximage.com) if you have questions.          ###

cp50_=$(DEVOBJ)gdevcp50.$(OBJ)
$(DD)cp50.dev : $(cp50_) $(DD)page.dev $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)cp50 $(cp50_)

$(DEVOBJ)gdevcp50.$(OBJ) : $(DEVSRC)gdevcp50.c $(PDEVH) $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(DEVO_)gdevcp50.$(OBJ) $(C_) $(DEVSRC)gdevcp50.c
