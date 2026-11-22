#
# module.mk
# (C) 2009 James Hammons
#
# Compilation file for DPLiX
#
# Options:
#	DISABLE_THREADS
# Compilation option:
#	V=1          Verbose mode
#	DESTDIR=xxx  Change the destination directory prefix

MODE			:= optimized
SUBDIRS 		+= src
#TARGETS			:= rastertodpl pstodpl
TARGETS			:= rastertodpl
PRE_GENERIC_TARGETS	:= optionList


# Default options

THREADS				?= 2
CACHESIZE			?= 30
DISABLE_THREADS		?= 1


# Flags

CXXFLAGS			+= `cups-config --cflags` -Isrc -Wall -I/opt/local/include
DEBUG_CXXFLAGS		+= -DDEBUG  -DDUMP_CACHE
OPTIM_CXXFLAGS 		+= -g
rastertodpl_LDFLAGS	:= `cups-config --ldflags` -L/opt/local/lib
rastertodpl_LIBS	:= `cups-config --libs` -lcupsimage
pstodpl_LDFLAGS		:= `cups-config --ldflags`
pstodpl_LIBS		:= `cups-config --libs` -lcupsimage


# Update compilation flags with defined options

ifneq ($(DISABLE_THREADS),0)
CXXFLAGS		+= -DDISABLE_THREADS
else
CXXFLAGS		+= -DTHREADS=$(THREADS) -DCACHESIZE=$(CACHESIZE)
endif


# Get some information

CUPSFILTER		:= `cups-config --serverbin`/filter
CUPSPPD			:= `cups-config --datadir`/model
ifeq ($(ARCHI),Darwin)
PSTORASTER		:= pstocupsraster
else
PSTORASTER		:= pstoraster
endif
export CUPSFILTER CUPSPPD


# Specific information needed by pstoqpdl

src_pstodpl_cpp_FLAGS	:= -DRASTERDIR=\"$(CUPSFILTER)\"
src_pstodpl_cpp_FLAGS	+= -DRASTERTODPL=\"rastertodpl\"
src_pstodpl_cpp_FLAGS	+= -DPSTORASTER=\"$(PSTORASTER)\"
src_pstodpl_cpp_FLAGS	+= -DCUPSPPD=\"$(CUPSPPD)\"
