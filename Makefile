NAME	:= webserv

SOURCES_FOLDER = sources/
SOURCES =	main.cpp \
			Configuration.cpp \
			Parser.cpp \
			Cluster.cpp \
			Socket.cpp \
			Connection.cpp \
			Utility.cpp \
			Http.cpp \
			HttpMessage.cpp \
			HttpRequest.cpp \
			HttpString.cpp \
			HttpBody.cpp \
			HttpHeader.cpp \
			HttpResponse.cpp \
			HttpStatus.cpp \
			CGIPython.cpp \
			EnvVar.cpp \
			TimeManage.cpp \
			ft_signal.cpp \
			Autoindex.cpp \
			FileData.cpp \

SOURCES_PREFIXED = $(addprefix $(SOURCES_FOLDER), $(SOURCES))

INCLUDES_FOLDER = includes/
INCLUDES = 	WebServ.hpp \
			Configuration.hpp \
			Parser.hpp \
			Cluster.hpp \
			Server.hpp \
			Socket.hpp \
			Connection.hpp \
			Utility.hpp \
			Http.hpp \
			HttpInfo.hpp \
			HttpMessage.hpp \
			HttpRequest.hpp \
			HttpString.hpp \
			HttpBody.hpp \
			HttpHeader.hpp \
			HttpResponse.hpp \
			HttpStatus.hpp \
			CGIPython.hpp \
			EnvVar.hpp \
			TimeManage.hpp \
			ft_signal.hpp \
			Autoindex.hpp \
			FileData.hpp \

INCLUDES_PREFIXED = $(addprefix $(INCLUDES_FOLDER), $(INCLUDES))

OBJECTS_FOLDER = objects/
OBJECT = $(SOURCES:.cpp=.o)
OBJECTS = $(addprefix $(OBJECTS_FOLDER), $(OBJECT))
DEPENDENCIES = $(OBJECTS:.o=.d)

CXX	:= clang++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -MMD -MP

PYTHON := ./python/python3.8

DEBUG_ACTIVE = 0

ifeq ($(shell uname),Linux)
	OS_LINUX = 1
else
	OS_LINUX = 0
endif


$(OBJECTS_FOLDER)%.o :	$(SOURCES_FOLDER)%.cpp
	@mkdir -p $(OBJECTS_FOLDER)
	@echo "Compiling : $<"
	@$(CXX) $(CXXFLAGS) -D OS_LINUX=$(OS_LINUX) -D DEBUG_ACTIVE=$(DEBUG_ACTIVE) -c $< -o $@ -I$(INCLUDES_FOLDER)

$(NAME): $(PYTHON) $(OBJECTS)
	@echo "Create    : $(NAME)"
	@$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

all: $(NAME)

-include $(DEPENDENCIES)

debug: fclean
	@echo "Debugging Mode Enabled"
	@make DEBUG_ACTIVE=1

clean:
	@rm -rf $(OBJECTS_FOLDER)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

# 自動フォーマット、設定ファイルはrootに置いてあります
format:
	@clang-format -i $(SOURCES_PREFIXED) $(INCLUDES_PREFIXED)

# vimでC++の定義jumpに必要なおまじないなので気にしないで下さいm(_ _)m
bear:
	@bear -- make re

$(PYTHON):
ifeq ($(OS_LINUX), 1)
	@cp python/python3.8_Linux $@
	@echo "Create $@ for Linux"
else
	@cp python/python3.8_Mac $@
	@echo "Create $@ for Mac"
endif


.PHONY	: all clean fclean re format
