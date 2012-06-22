##============================================================================
# makefile for the A.L.E: Atari Learing Environment (Powered by Stella)
##============================================================================

srcdir      ?= .
DEFINES     := -DRLGENV_NOMAINLOOP
LDFLAGS     := -L/u/mhauskn/local/lib 
INCLUDES    := -Isrc/control -Isrc/os_dependent -I/usr/include 
LIBS	    := -lz -lSDL -lSDL_gfx -lSDL_image -lboost_thread-mt -lboost_serialization -lboost_system -lboost_filesystem -lncurses
OBJS	    :=
HEADERS     :=
PROF        := 
PROFILE     := 
MODULES     :=
MODULE_DIRS :=
DISTNAME    := atari_learning_environment
DEBUG       := 0

# Optional libraries
HUNGARIAN   := 0 

CXX := g++
CXXFLAGS := -O3 -Wall -Wno-sign-compare -Wno-multichar -Wunused -std=c++0x
LD := g++
INSTALL := install
MKDIR := mkdir -p
ECHO := printf
CAT := cat
RM := rm -f
RM_REC := rm -f -r

NASM := /usr/bin/nasm
NASMFLAGS := -f elf

INSTALLDIR = build
INSTALL_INCLUDEDIR = $(INSTALLDIR)/include/
INSTALL_LIBDIR = $(INSTALLDIR)/lib/
INSTALL_BINDIR = $(INSTALLDIR)/bin/

INCLUDES += -Isrc/emucore -Isrc/emucore/m6502/src -Isrc/emucore/m6502/src/bspf/src -Isrc/common -Isrc/control -Isrc/player_agents -I/usr/include/ -I/usr/include/c++/4.4 -I/usr/include/c++/4.4/i486-linux-gnu/ -I/usr/include/c++/4.4/backward -I/usr/include 
DEFINES += -DUNIX -DHAS_ALTIVEC -DUSE_NASM -DBSPF_UNIX -DHAVE_INTTYPES -DWINDOWED_SUPPORT -DHAVE_GETTIMEOFDAY -DSNAPSHOT_SUPPORT

ifeq ("$(DEBUG)", "1")
  CXXFLAGS += -g
endif

# Hungarian alg is used for blob matching. Requires munkres library to be installed.
ifeq ("$(HUNGARIAN)", "1")
  DEFINES += -DMUNKRES
  INCLUDES += -I/usr/include/munkres
  LIBS += -lmunkres
endif

ifdef PROFILE
  PROF:= -g -pg -fprofile-arcs -ftest-coverage
  CXXFLAGS+= $(PROF)
# else
#   CXXFLAGS+= -fomit-frame-pointer
endif

# Even more warnings...
#CXXFLAGS+= -pedantic -Wpointer-arith -Wcast-qual -Wconversion
#CXXFLAGS+= -Wshadow -Wimplicit -Wundef -Wnon-virtual-dtor
#CXXFLAGS+= -Wno-reorder -Wwrite-strings -fcheck-new -Wctor-dtor-privacy 

#######################################################################
# Misc stuff - you should never have to edit this                     #
#######################################################################

EXECUTABLE  := ale
LIBRARY := libale.so

all: $(EXECUTABLE) $(LIBRARY)

######################################################################
# Various minor settings
######################################################################

# The name for the directory used for dependency tracking
DEPDIR := .deps

######################################################################
# Module settings
######################################################################

MODULES := $(MODULES)

# After the game specific modules follow the shared modules
MODULES += \
	. \
	src \
	src/emucore \
	src/emucore/m6502 \
	src/common \
	src/control \
	src/os_dependent \
	src/player_agents \
	src/player_agents/models \
	src/player_agents/planners \

######################################################################
# The build rules follow - normally you should have no need to
# touch whatever comes after here.
######################################################################

# Concat DEFINES and INCLUDES to form the CPPFLAGS
CPPFLAGS:= $(DEFINES) $(INCLUDES)

# Include the build instructions for all modules
-include $(addprefix $(srcdir)/, $(addsuffix /module.mk,$(MODULES)))

# Depdir information
# DEPDIRS = $(addsuffix /$(DEPDIR),$(MODULE_DIRS))

$(EXECUTABLE):  $(OBJS)
	$(LD) $(LDFLAGS) $(PRE_OBJS_FLAGS) $+ $(POST_OBJS_FLAGS) $(LIBS) $(PROF) -o $@

$(LIBRARY): $(OBJS)
	$(LD) $(LDFLAGS) $(LIBS) -shared -o $(LIBRARY) $(OBJECTS)

distclean: clean
	$(RM_REC) $(DEPDIRS)
	$(RM) build.rules config.h config.mak config.log

clean:
	$(RM) $(OBJS) $(EXECUTABLE) $(LIBRARY)

.PHONY: all clean dist distclean

.SUFFIXES: .cxx .o .cpp

.cxx.o: $(HEADERS)
	$(CXX) $(CXXFLAGS) -c  -o $@ $<

.cpp.o: $(HEADERS)
	$(CXX) $(CXXFLAGS) -c  -o $@ $<


# Include the dependency tracking files. We add /dev/null at the end
# of the list to avoid a warning/error if no .d file exist
# -include $(wildcard $(addsuffix /*.d,$(DEPDIRS))) /dev/null

install: all
	$(MKDIR) $(INSTALLDIR)
	$(MKDIR) $(INSTALL_LIBDIR)
	$(MKDIR) $(INSTALL_INCLUDEDIR)
	$(MKDIR) $(INSTALL_BINDIR)
	$(INSTALL) $(LIBRARY) $(INSTALL_LIBDIR)
	$(INSTALL) $(EXECUTABLE) $(INSTALL_BINDIR)

uninstall:
	$(RM) $(INSTALL_BINDIR)$(EXECUTABLE)
	$(RM) $(INSTALL_LIBDIR)$(LIBRARY)
	$(RM_REC) $(INSTALL_BINDIR)
	$(RM_REC) $(INSTALL_LIBDIR)	
	$(RM_REC) $(INSTALL_INCLUDEDIR)
	$(RM_REC) $(INSTALLDIR)


