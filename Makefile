NAME	= libgnl.a

CC		= cc
CFLAGS	= -Wall -Wextra -Werror
AR		= ar rcs
RM		= rm -f

SRCS	= get_next_line.c \
		  get_next_line_utils.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $(NAME) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(NAME)
	$(CC) $(CFLAGS) main.c $(NAME) -o test_runner

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME) test_runner

re: fclean all

.PHONY: all clean fclean re test
