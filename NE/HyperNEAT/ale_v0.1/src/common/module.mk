MODULE := src/common

MODULE_OBJS := \
	src/common/SoundNull.o \
	src/common/export_screen.o \
	src/common/display_screen.o \
	src/common/visual_processor.o \
	src/common/vector_matrix_tools.o \

MODULE_SOURCES := \
	src/common/SoundNull.cxx \
	src/common/export_screen.cpp \
	src/common/display_screen.cpp \
	src/common/visual_processor.cpp \
	src/common/vector_matrix_tools.cpp \

MODULE_HEADERS := \
	src/common/SoundNull.hxx \
	src/common/export_screen.h \
	src/common/display_screen.h \
	src/common/visual_processor.h \
	src/common/vector_matrix_tools.h \

MODULE_DIRS += \
	src/common

# Include common rules 
include $(srcdir)/common.rules
