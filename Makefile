# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/04/13 17:23:17 by ncoursol          #+#    #+#              #
#    Updated: 2023/01/27 22:39:45 by ncoursol         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = gomo
ARENA = mcts_arena

# Directories
GOMO_DIR = ./src/gomo
GAME_DIR = ./src/game
BOT_DIR = ./src/bot
BUILD_DIR = ./build

# External libraries
LIBGLFW3 = ext/glfw-3.3.5/build/src/libglfw3.a
LIBGLAD = ext/glad/libglad.a
LIBFFT = ext/freetype/objs/libfreetype.a

CC = clang
CXX = clang++

FLAGS = -Wall -Wextra -Werror
FLAGS += -I./include -I./ext/glfw-3.3.5/include/ -I./ext/glad/include/ -I./ext/freetype/include/
FLAGS += -g3 -ggdb3
FLAGS += -DNDEBUG
LDFLAGS = -L./ext/glfw-3.3.5/build/src/ -L./ext/glad/ -L./ext/freetype/objs/ -L/usr/lib/gcc/x86_64-linux-gnu/11

UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDLIBS = -lX11 -lpthread -ldl -lm -lglfw3 -lglad -lfreetype -lstdc++
endif

# Source files
C_SRCS = $(wildcard $(GOMO_DIR)/*.c) $(wildcard $(GAME_DIR)/*.c) $(wildcard $(BOT_DIR)/*.c)
# Exclude v1 and v2 from main build (they're for arena only)
C_SRCS := $(filter-out ./src/bot/mcts_v1.c ./src/bot/mcts_v2.c ./src/bot/mcts_arena.c ./src/bot/gui_stubs.c, $(C_SRCS))
CXX_SRCS = $(wildcard $(BOT_DIR)/*.cpp)

# Object files
C_OBJS = $(C_SRCS:./src/%.c=$(BUILD_DIR)/%.o)
CXX_OBJS = $(CXX_SRCS:./src/%.cpp=$(BUILD_DIR)/%.o)
OBJS = $(C_OBJS) $(CXX_OBJS)

# Dependency files
DEPS = $(OBJS:.o=.d)

.PHONY: all clean fclean re arena

all: $(NAME)

arena: $(ARENA)

$(LIBGLFW3):
	cmake -S ./ext/glfw-3.3.5/ -B ./ext/glfw-3.3.5/build/
	make -C ./ext/glfw-3.3.5/build/

$(LIBGLAD):
	make -C ./ext/glad/

$(LIBFFT):
	make -C ./ext/freetype/

-include $(DEPS)

$(BUILD_DIR)/%.o: ./src/%.c Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	@$(CC) $(FLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(BUILD_DIR)/%.o: ./src/%.cpp Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	@$(CXX) $(FLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(NAME): $(LIBGLFW3) $(LIBGLAD) $(LIBFFT) $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CXX) $(LDFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)
	@echo "Build complete!"

# Arena-specific build (without GUI dependencies)
ARENA_GAME_SRCS = $(wildcard $(GAME_DIR)/*.c)
ARENA_BOT_SRCS = $(BOT_DIR)/heuristics.c $(BOT_DIR)/mcts_v1.c $(BOT_DIR)/mcts_v2.c $(BOT_DIR)/mcts_arena.c $(BOT_DIR)/gui_stubs.c
ARENA_TOOLS_SRCS = ./src/tools/mcts_arena_main.c

ARENA_GAME_OBJS = $(ARENA_GAME_SRCS:./src/game/%.c=$(BUILD_DIR)/arena_game/%.o)
ARENA_BOT_OBJS = $(ARENA_BOT_SRCS:./src/bot/%.c=$(BUILD_DIR)/arena_bot/%.o)
ARENA_TOOLS_OBJS = $(ARENA_TOOLS_SRCS:./src/tools/%.c=$(BUILD_DIR)/arena_tools/%.o)
ARENA_OBJS = $(ARENA_GAME_OBJS) $(ARENA_BOT_OBJS) $(ARENA_TOOLS_OBJS)

$(BUILD_DIR)/arena_game/%.o: ./src/game/%.c Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling Arena C: $<"
	@$(CC) $(FLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(BUILD_DIR)/arena_bot/%.o: ./src/bot/%.c Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling Arena C: $<"
	@$(CC) $(FLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(BUILD_DIR)/arena_tools/%.o: ./src/tools/%.c Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling Arena C: $<"
	@$(CC) $(FLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(ARENA): $(ARENA_OBJS)
	@echo "Linking $(ARENA)..."
	@$(CC) $(LDFLAGS) -o $(ARENA) $(ARENA_OBJS) -lpthread -lm
	@echo "Arena build complete!"

clean:
	make clean -C ./ext/glad/
	cd ./ext/freetype/ && make clean
	rm -rf ./build

fclean: clean
	make fclean -C ./ext/glad/
	rm -rf ./ext/glfw-3.3.5/build
	rm -rf ./ext/freetype/objs/*
	rm -f $(NAME) $(ARENA)

re: fclean all

info:
	@echo "C Sources: $(C_SRCS)"
	@echo "C++ Sources: $(CXX_SRCS)"
	@echo "Objects: $(OBJS)"

.PHONY: all clean fclean re
