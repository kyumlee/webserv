NAME = test

SRCS = ./main.cpp ../srcs/Utils.cpp ../srcs/config/LocationBlock.cpp\
	../srcs/config/ServerBlock.cpp ../srcs/config/Config.cpp ./Server.cpp\
	
OBJS = $(SRCS:.cpp=.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re