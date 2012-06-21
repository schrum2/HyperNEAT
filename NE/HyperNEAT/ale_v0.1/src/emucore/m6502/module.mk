MODULE := src/emucore/m6502

MODULE_OBJS := \
	src/emucore/m6502/src/Device.o \
	src/emucore/m6502/src/M6502.o \
	src/emucore/m6502/src/M6502Low.o \
	src/emucore/m6502/src/M6502Hi.o \
	src/emucore/m6502/src/NullDev.o \
	src/emucore/m6502/src/System.o

MODULE_HEADERS := \
	src/emucore/m6502/src/Device.h \
	src/emucore/m6502/src/M6502.h \
	src/emucore/m6502/src/M6502Low.h \
	src/emucore/m6502/src/M6502Hi.h \
	src/emucore/m6502/src/NullDev.h \
	src/emucore/m6502/src/System.h

MODULE_SOURCES := \
	src/emucore/m6502/src/Device.cxx \
	src/emucore/m6502/src/M6502.cxx \
	src/emucore/m6502/src/M6502Low.cxx \
	src/emucore/m6502/src/M6502Hi.cxx \
	src/emucore/m6502/src/NullDev.cxx \
	src/emucore/m6502/src/System.cxx

MODULE_DIRS += \
	src/emucore/m6502/src

# Include common rules 
include $(srcdir)/common.rules
