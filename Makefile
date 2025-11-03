# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/29 21:18:20 by tfregni           #+#    #+#              #
#    Updated: 2025/11/03 11:12:56 by tfregni          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Colors
RED = \033[0;31m
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
CYAN = \033[0;36m
MAGENTA = \033[0;35m
NC = \033[0m

SRC = $(addprefix $(SRC_DIR)/, ft_ping.c network.c parse.c ping.c ip_header.c icmp_packet.c \
	time_utils.c bitmap.c output_format.c output_debug.c)
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc
CC = cc
CFLAGS = -Wall -Wextra -Werror
LDLIBS = -lm
RM = rm -rf
NAME = ft_ping

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
$(OBJ): | $(OBJ_DIR) # '|' is order-only prerequisite: not a change that should trigger rebuild

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(NAME) $(OBJ) $(LDLIBS)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
