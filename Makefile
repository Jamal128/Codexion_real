NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC =	main.c \
		threads.c \
		parse_args.c \
		init_codexion.c \
		init_dongles.c \
		init_coders.c \
		queue_init.c \
		destroy.c \
		coder_routine.c \
		coder_actions.c \
		dongle_helpers.c \
		dongle_acquire.c \
		dongle_take.c \
		dongle_release.c \
		heap_compare.c \
		heap_insert.c \
		heap_remove.c \
		stop_check.c \
		monitor.c \
		print_logs.c \
		utils.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
