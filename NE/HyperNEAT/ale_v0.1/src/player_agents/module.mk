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
	src/player_agents/model_agent.o \
	src/player_agents/self_detection_agent.o \

MODULE_DIRS += \
	src/player_agents

# Include common rules 
include $(srcdir)/common.rules
