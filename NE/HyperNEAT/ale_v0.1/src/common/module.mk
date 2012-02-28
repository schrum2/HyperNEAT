MODULE := src/common

MODULE_OBJS := \
	src/common/SoundNull.o \
	src/common/export_screen.o \
	src/common/display_screen.o \
	src/common/visual_processor.o \
	src/common/vector_matrix_tools.o \

MODULE_DIRS += \
	src/common

# Include common rules 
include $(srcdir)/common.rules
