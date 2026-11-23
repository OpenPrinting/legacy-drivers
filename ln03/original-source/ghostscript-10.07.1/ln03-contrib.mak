### -------- The DEC LN03/DL2100/LA50/LA70/LA75 printer devices -------- ###

ln03_=$(DEVOBJ)gdevln03.$(OBJ)

# Author: Ulrich Mueller (ulm@vsnhd1.cern.ch)
# Printer: DEC LN03
$(DD)ln03.dev : $(ln03_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)ln03 $(ln03_)

# Author: Nick Brown (nick.brown@coe.int)
# Printer: DEClaser 2100
$(DD)dl2100.dev : $(ln03_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)dl2100 $(ln03_)

# Author: Ian MacPhedran (macphed@dvinci.USask.CA)
# Printer: DEC LA50
$(DD)la50.dev : $(ln03_) $(DD)page.dev \
                          $(CONTDEV)  $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)la50 $(ln03_)

# Author: Bruce Lowekamp (lowekamp@csugrad.cs.vt.edu)
# Printer: DEC LA70
$(DD)la70.dev : $(ln03_) $(DD)page.dev \
                         $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)la70 $(ln03_)

# Author: Ian MacPhedran (macphed@dvinci.USask.CA)
# Printer: DEC LA75
$(DD)la75.dev : $(ln03_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)la75 $(ln03_)

# Author: Andre' Beck (Andre_Beck@IRS.Inf.TU-Dresden.de)
# Printer: DEC LA75plus
$(DD)la75plus.dev : $(ln03_) $(DD)page.dev \
                           $(CONTDEV) $(CONTRIB_MAK) $(MAKEDIRS)
	$(SETPDEV) $(DD)la75plus $(ln03_)

$(DEVOBJ)gdevln03.$(OBJ) : $(CONTRIBSRC)gdevln03.c $(PDEVH) \
                           $(CONTRIB_MAK) $(MAKEDIRS)
	$(DEVCC) $(DEVO_)gdevln03.$(OBJ) $(C_) $(CONTRIBSRC)gdevln03.c
