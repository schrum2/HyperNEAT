MODULE := src/player_agents

MODULE_OBJS := \
	src/player_agents/game_settings.o \
	src/player_agents/player_agent.o \
	src/player_agents/random_agent.o \
	src/player_agents/rl_sarsa_lambda.o \
	src/player_agents/tiles2.o \
	src/player_agents/mountain_car_test.o \
	src/player_agents/ram_agent.o \
	src/player_agents/class_agent.o \
	src/player_agents/blob_object.o \
	src/player_agents/class_shape.o \
	src/player_agents/single_action_agent.o \
	src/player_agents/common_constants.o \
	src/player_agents/freeway_agent.o \
	src/player_agents/background_detector.o \
	src/player_agents/class_discovery.o \
	src/player_agents/region_manager.o \
	src/player_agents/shape_tools.o \
	src/player_agents/blob_class.o \
	src/player_agents/grid_screen_agent.o \
	src/player_agents/search_agent.o \
	src/player_agents/tree_node.o \
	src/player_agents/search_tree.o \
	src/player_agents/full_search_tree.o \
	src/player_agents/uct_search_tree.o \
	src/player_agents/actions_summary_agent.o \
	src/player_agents/bass_agent.o \
	src/player_agents/self_detection_agent.o \
	src/player_agents/qlearning_agent.o

MODULE_HEADERS := \
	src/player_agents/game_settings.h \
	src/player_agents/player_agent.h \
	src/player_agents/random_agent.h \
	src/player_agents/rl_sarsa_lambda.h \
	src/player_agents/tiles2.h \
	src/player_agents/mountain_car_test.h \
	src/player_agents/ram_agent.h \
	src/player_agents/class_agent.h \
	src/player_agents/blob_object.h \
	src/player_agents/class_shape.h \
	src/player_agents/single_action_agent.h \
	src/player_agents/common_constants.h \
	src/player_agents/freeway_agent.h \
	src/player_agents/background_detector.h \
	src/player_agents/class_discovery.h \
	src/player_agents/region_manager.h \
	src/player_agents/shape_tools.h \
	src/player_agents/blob_class.h \
	src/player_agents/grid_screen_agent.h \
	src/player_agents/search_agent.h \
	src/player_agents/tree_node.h \
	src/player_agents/search_tree.h \
	src/player_agents/full_search_tree.h \
	src/player_agents/uct_search_tree.h \
	src/player_agents/actions_summary_agent.h \
	src/player_agents/bass_agent.h \
	src/player_agents/self_detection_agent.h \
	src/player_agents/qlearning_agent.h

MODULE_SOURCES := \
	src/player_agents/game_settings.cpp \
	src/player_agents/player_agent.cpp \
	src/player_agents/random_agent.cpp \
	src/player_agents/rl_sarsa_lambda.cpp \
	src/player_agents/tiles2.cpp \
	src/player_agents/mountain_car_test.cpp \
	src/player_agents/ram_agent.cpp \
	src/player_agents/class_agent.cpp \
	src/player_agents/blob_object.cpp \
	src/player_agents/class_shape.cpp \
	src/player_agents/single_action_agent.cpp \
	src/player_agents/common_constants.cpp \
	src/player_agents/freeway_agent.cpp \
	src/player_agents/background_detector.cpp \
	src/player_agents/class_discovery.cpp \
	src/player_agents/region_manager.cpp \
	src/player_agents/shape_tools.cpp \
	src/player_agents/blob_class.cpp \
	src/player_agents/grid_screen_agent.cpp \
	src/player_agents/search_agent.cpp \
	src/player_agents/tree_node.cpp \
	src/player_agents/search_tree.cpp \
	src/player_agents/full_search_tree.cpp \
	src/player_agents/uct_search_tree.cpp \
	src/player_agents/actions_summary_agent.cpp \
	src/player_agents/bass_agent.cpp \
	src/player_agents/self_detection_agent.cpp \
	src/player_agents/qlearning_agent.cpp

MODULE_DIRS += \
	src/player_agents

# Include common rules 
include $(srcdir)/common.rules
