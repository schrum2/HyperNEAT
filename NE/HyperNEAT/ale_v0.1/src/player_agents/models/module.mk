MODULE := src/player_agents/models

MODULE_OBJS := \
	src/player_agents/models/model.o \
	src/player_agents/models/baseline_model.o \
	src/player_agents/models/block_grid.o \
	src/player_agents/models/freeway_model.o \
	src/player_agents/models/basic_dbn.o
#	src/player_agents/models/logistic.o

MODULE_DIRS += \
	src/player_agents/models

# Include common rules 
include $(srcdir)/common.rules
