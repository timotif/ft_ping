# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/29 21:18:20 by tfregni           #+#    #+#              #
#    Updated: 2025/10/29 21:18:52 by tfregni          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRC = ft_ping.c socket.c parse.c packet.c ping.c ip.c icmp.c utils.c
OBJ = $(SRC:.c=.o)
CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDLIBS = -lm
RM = rm -rf
NAME = ft_ping

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDLIBS)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
