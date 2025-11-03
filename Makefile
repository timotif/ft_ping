# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tfregni <tfregni@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/29 21:18:20 by tfregni           #+#    #+#              #
#    Updated: 2025/11/03 11:48:27 by tfregni          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ================================ Colors ====================================
RED = \033[0;31m
GREEN = \033[0;32m
BLUE = \033[0;34m
YELLOW = \033[0;33m
CYAN = \033[0;36m
MAGENTA = \033[0;35m
BOLD = \033[1m
DIM = \033[2m
RESET = \033[0m

# ============================== Variables ===================================
SRC = $(addprefix $(SRC_DIR)/, ft_ping.c network.c parse.c ping.c ip_header.c \
	icmp_packet.c time_utils.c bitmap.c output_format.c output_debug.c)
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc
CC = cc
CFLAGS = -Wall -Wextra -Werror
LDLIBS = -lm
RM = rm -rf
NAME = ft_ping

# Suppress Make's built-in error messages
MAKEFLAGS += --no-print-directory

# Progress tracking
TOTAL_FILES := $(words $(SRC))
CURRENT_FILE = 0

# =============================== Rules ======================================
all: $(NAME)

.PHONY: banner
banner:
	@clear
	@echo "$(CYAN)$(BOLD)"
	@echo "    ███████╗████████╗     ██████╗ ██╗███╗   ██╗ ██████╗ "
	@echo "    ██╔════╝╚══██╔══╝     ██╔══██╗██║████╗  ██║██╔════╝ "
	@echo "    █████╗     ██║        ██████╔╝██║██╔██╗ ██║██║  ███╗"
	@echo "    ██╔══╝     ██║        ██╔═══╝ ██║██║╚██╗██║██║   ██║"
	@echo "    ██║        ██║███████╗██║     ██║██║ ╚████║╚██████╔╝"
	@echo "    ╚═╝        ╚═╝╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝ "
	@echo "$(DIM)                                        tfregni@42Berlin$(RESET)"
	@echo "$(MAGENTA)    ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"
	@echo ""

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "$(BLUE)⚡ Compiling$(RESET) $(CYAN)%-30s$(RESET)" "$(notdir $<)"
	@OUTPUT=$$($(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@ 2>&1); \
	if [ $$? -ne 0 ]; then \
		echo " $(RED)✗$(RESET)"; \
		echo ""; \
		echo "$(RED)$(BOLD)╔═══════════════════════════════════════════════════════════════╗$(RESET)"; \
		echo "$(RED)$(BOLD)║$(RESET)  $(BOLD)Compilation Failed: $(notdir $<)$(RESET)"; \
		echo "$(RED)$(BOLD)╚═══════════════════════════════════════════════════════════════╝$(RESET)"; \
		echo ""; \
		echo "$$OUTPUT" | sed 's/^/  /'; \
		echo ""; \
		exit 1; \
	else \
		echo " $(GREEN)✓$(RESET)"; \
	fi

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@$(MAKE) --no-print-directory banner
	@echo "$(DIM)📁 Creating object directory...$(RESET)"

$(OBJ): | $(OBJ_DIR)

$(NAME): $(OBJ)
	@echo ""
	@echo "$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"
	@printf "$(YELLOW)🔗 Linking binary$(RESET) $(BOLD)$(NAME)$(RESET)..."
	@OUTPUT=$$($(CC) $(CFLAGS) -I$(INC_DIR) -o $(NAME) $(OBJ) $(LDLIBS) 2>&1); \
	if [ $$? -ne 0 ]; then \
		echo " $(RED)✗$(RESET)"; \
		echo ""; \
		echo "$(RED)$(BOLD)╔═══════════════════════════════════════════════════════════════╗$(RESET)"; \
		echo "$(RED)$(BOLD)║$(RESET)  $(BOLD)Linking Failed$(RESET)"; \
		echo "$(RED)$(BOLD)╚═══════════════════════════════════════════════════════════════╝$(RESET)"; \
		echo ""; \
		echo "$$OUTPUT" | sed 's/^/  /'; \
		echo ""; \
		exit 1; \
	else \
		echo " $(GREEN)✓$(RESET)"; \
	fi
	@echo ""
	@echo "$(GREEN)$(BOLD)    ✨ Build completed successfully! ✨$(RESET)"
	@echo ""
	@echo "$(DIM)    Binary: $(RESET)$(BOLD)./$(NAME)$(RESET)"
	@echo "$(DIM)    Usage:  $(RESET)./$(NAME) -? or ./$(NAME) --usage"
	@echo ""
	@echo "$(MAGENTA)    ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)"
	@echo ""

clean:
	@echo "$(YELLOW)🧹 Cleaning object files...$(RESET)"
	@$(RM) $(OBJ_DIR)
	@echo "$(GREEN)✓ Clean complete$(RESET)"

fclean: clean
	@echo "$(YELLOW)🗑️  Removing binary...$(RESET)"
	@$(RM) $(NAME)
	@echo "$(GREEN)✓ Full clean complete$(RESET)"

re: fclean
	@echo ""
	@$(MAKE) --no-print-directory all 2>&1 | grep -v "make\[" || true

.PHONY: all clean fclean re banner
