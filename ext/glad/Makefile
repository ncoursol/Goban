NAME = libglad.a

CC = clang
CFLAGS = -Wall -Wextra -Werror -I./include/

SRCS = $(shell find ./src/ -type f -name "*.c")

OBJS = $(SRCS:.c=.o)

.PHONY: all
all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(NAME): $(OBJS)
	ar -rcs $(NAME) $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS)

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all
