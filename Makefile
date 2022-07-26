NAME			= webserv
CPP				= c++
CPPFLAGS		= -Wall -Wextra -Werror -std=c++98
SRCS			= ./main.cpp \
				  ./srcs/Utils.cpp \
				  ./srcs/config/Config.cpp \
				  ./srcs/config/LocationBlock.cpp \
				  ./srcs/config/ServerBlock.cpp \
				  ./srcs/header/EntityHeader.cpp \
				  ./srcs/header/GeneralHeader.cpp \
				  ./srcs/header/RequestHeader.cpp \
				  ./srcs/header/ResponseHeader.cpp \
				  ./srcs/cgi.cpp ./srcs/Server.cpp ./srcs/Response.cpp
OBJS			= $(SRCS:.cpp=.o)
RM				= rm -f

FG_RED			= \033[31m
FG_GREEN		= \033[32m
FG_YELLOW		= \033[33m
FG_BLUE			= \033[34m
FG_MAGENTA		= \033[35m

NO_COLOR =		\033[0m

%.o :			%.cpp compile
				@$(CPP) $(CPPFLAGS) -c $< -o $(<:.cpp=.o)

all :			$(NAME)

$(NAME) :		$(OBJS)
				@echo "${FG_YELLOW} Linking object file(s) . . .\n${NO_COLOR}"
				@$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME) -I.
				@echo "${FG_GREEN} OK ${NO_COLOR}"

clean :
				@echo "${FG_MAGENTA}\n Removing object file(s) . . .${NO_COLOR}"
				@$(RM) $(OBJS)

fclean :		clean
				@echo "${FG_RED}\n Removing [$(NAME)] file . . .${NO_COLOR}"
				@$(RM) $(NAME)

re :			fclean all

compile :
				@echo "${FG_BLUE}\n Compiling . . .\n${NO_COLOR}"

.PHONY :		all clean fclean re

.INTERMEDIATE :	compile
