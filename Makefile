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
SRCS_DIR = ./src/$(NAME)
BUILD_DIR = ./build/$(NAME)

LIBGLFW3 = ext/glfw-3.3.5/build/src/libglfw3.a
LIBGLAD = ext/glad/libglad.a
LIBFFT = ext/freetype/objs/libfreetype.a

CC = clang #-fsanitize=address -g

CFLAGS = -Wall -Wextra -Werror
CFLAGS += -I./src/ -I./ext/glfw-3.3.5/include/ -I./ext/glad/include/ -I./ext/freetype/include/
CFLAGS += -g3 -ggdb3
CFLAGS += -DNDEBUG
LDFLAGS = -L./ext/glfw-3.3.5/build/src/ -L./ext/glad/ -L./ext/freetype/objs/

UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDLIBS = -lX11 -lpthread -ldl -lm -lglfw3 -lglad -lfreetype
endif

SRCS = $(shell find $(SRCS_DIR) -type f -name "*.c")
OBJS = $(SRCS:$(SRCS_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(NAME)

$(LIBGLFW3):
	cmake -S ./ext/glfw-3.3.5/ -B ./ext/glfw-3.3.5/build/
	make -C ./ext/glfw-3.3.5/build/

$(LIBGLAD):
	make -C ./ext/glad/

$(LIBFFT):
	make -C ./ext/freetype/

-include $(DEPS)

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(NAME): $(LIBGLFW3) $(LIBGLAD) $(LIBFFT) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

clean:
	make clean -C ./ext/glad/
	cd ./ext/freetype/ && make clean
	rm -rf ./build

fclean: clean
	make fclean -C ./ext/glad/
	rm -rf ./ext/glfw-3.3.5/build
	rm -rf ./ext/freetype/objs/*
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
