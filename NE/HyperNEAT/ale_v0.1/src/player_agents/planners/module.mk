MODULE := src/player_agents/planners

MODULE_OBJS := \
	src/player_agents/planners/node.o \
	src/player_agents/planners/uct.o 

MODULE_HEADERS := \
	src/player_agents/planners/node.h \
	src/player_agents/planners/uct.h 

MODULE_SOURCES := \
	src/player_agents/planners/node.cpp \
	src/player_agents/planners/uct.cpp 

MODULE_DIRS += \
	src/player_agents/planners

# Include common rules 
include $(srcdir)/common.rules
