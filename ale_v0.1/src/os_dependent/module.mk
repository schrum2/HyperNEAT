MODULE := src/os_dependent

MODULE_OBJS := \
	src/os_dependent/FSNodePOSIX.o \
	src/os_dependent/OSystemUNIX.o \
	src/os_dependent/SettingsUNIX.o \

MODULE_HEADERS := \
	src/os_dependent/OSystemUNIX.hxx \
	src/os_dependent/SettingsUNIX.hxx \

MODULE_SOURCES := \
	src/os_dependent/FSNodePOSIX.cxx \
	src/os_dependent/OSystemUNIX.cxx \
	src/os_dependent/SettingsUNIX.cxx \

MODULE_DIRS += \
	src/os_dependent

# Include common rules 
include $(srcdir)/common.rules
