MODULE := src/player_agents/models

MODULE_OBJS := \
	src/player_agents/models/model.o \
	src/player_agents/models/baseline_model.o \
	src/player_agents/models/block_grid.o \
	src/player_agents/models/freeway_model.o \
	src/player_agents/models/basic_dbn.o
#	src/player_agents/models/logistic.o

MODULE_HEADERS := \
	src/player_agents/models/model.h \
	src/player_agents/models/baseline_model.h \
	src/player_agents/models/block_grid.h \
	src/player_agents/models/freeway_model.h \
	src/player_agents/models/basic_dbn.h

MODULE_SOURCES := \
	src/player_agents/models/model.cpp \
	src/player_agents/models/baseline_model.cpp \
	src/player_agents/models/block_grid.cpp \
	src/player_agents/models/freeway_model.cpp \
	src/player_agents/models/basic_dbn.cpp

MODULE_DIRS += \
	src/player_agents/models

# Include common rules 
include $(srcdir)/common.rules
