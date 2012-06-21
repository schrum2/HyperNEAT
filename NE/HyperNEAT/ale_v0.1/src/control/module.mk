MODULE := src/control

MODULE_OBJS := \
	src/control/fifo_controller.o \
	src/control/game_controller.o \
	src/control/internal_controller.o \

MODULE_HEADERS := \
	src/control/fifo_controller.cpp \
	src/control/game_controller.cpp \
	src/control/internal_controller.cpp \

MODULE_SOURCES := \
	src/control/fifo_controller.h \
	src/control/game_controller.h \
	src/control/internal_controller.h \

MODULE_DIRS += \
	src/control

# Include common rules 
include $(srcdir)/common.rules
