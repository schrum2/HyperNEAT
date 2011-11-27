MODULE := src/player_agents/planners

MODULE_OBJS := \
	src/player_agents/planners/node.o \
	src/player_agents/planners/uct.o 

MODULE_DIRS += \
	src/player_agents/planners

# Include common rules 
include $(srcdir)/common.rules
