NAME = webserv

CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -g3

DIR_SRCS = srcs
DIR_CONFIG = srcs/config
DIR_INCS = includes
DIR_OBJS = .objs

MAIN_SRCS    =  main.cpp
CONFIG_SRCS  = configParse.cpp configParseException.cpp locationCheck.cpp serverCheck.cpp tokenization.cpp config_utils.cpp
RESPONSE_SRCS  = response_utils.cpp HTTPResponse.cpp
HTTPREQUEST_SRCS = HTTPRequest.cpp CheckRequest.cpp CheckRequestUtils.cpp InvalidHeaderException.cpp
METHOD_SRCS = auto_index.cpp get.cpp method_utils.cpp delete.cpp put.cpp post.cpp multipart.cpp patch.cpp
SERVER_SRCS = Server.cpp ServerManager.cpp Session.cpp SessionManager.cpp
CLIENT_SRCS = Client.cpp
CGI_SRCS = CGI.cpp

LST_SRCS    =   $(addprefix config/, ${CONFIG_SRCS}) \
				$(addprefix request/, ${HTTPREQUEST_SRCS}) \
				$(addprefix response/, ${RESPONSE_SRCS}) \
				$(addprefix method/, ${METHOD_SRCS}) \
				$(addprefix server/, ${SERVER_SRCS}) \
				$(addprefix client/, ${CLIENT_SRCS}) \
				$(addprefix cgi/, ${CGI_SRCS}) \
                main.cpp

LST_OBJS = $(LST_SRCS:.cpp=.o)
LST_INCS = Client.hpp Colors.hpp ConfigFile.hpp HTTPRequest.hpp HTTPResponse.hpp Server.hpp InvalidHeaderException.hpp CGI_Data.hpp ServerManager.hpp

SRCS = $(addprefix ${DIR_SRCS}/, ${LST_SRCS})
OBJS = $(addprefix $(DIR_OBJS)/, ${LST_OBJS})
INCS = $(addprefix $(DIR_INCS)/, ${LST_INCS})

ERASE		=	\033[2K\r
BLUE		=	\033[34m
YELLOW		=	\033[33m
GREEN		=	\033[32m
END			=	\033[0m

$(DIR_OBJS)/%.o: ${DIR_SRCS}/%.cpp $(INCS) Makefile
	@ mkdir -p $(DIR_OBJS)
	@ mkdir -p $(DIR_OBJS)/config
	@ mkdir -p $(DIR_OBJS)/response
	@ mkdir -p $(DIR_OBJS)/request
	@ mkdir -p $(DIR_OBJS)/method
	@ mkdir -p $(DIR_OBJS)/server
	@ mkdir -p $(DIR_OBJS)/client
	@ mkdir -p $(DIR_OBJS)/cgi
	@ $(CC) $(CFLAGS) -I $(DIR_INCS) -c $< -o $@
	@ printf "$(ERASE)$(BLUE) > Compilation :$(END) $<"

all : ${NAME}

$(NAME):	$(OBJS)
	@ $(CC) $(CFLAGS) $(OBJS) -o $@
	@ printf "$(ERASE)$(GREEN)$@ made\n$(END)"

clean:
	@ printf "$(YELLOW)$(DIR_OBJS) removed$(END)\n"
	@ rm -rdf $(DIR_OBJS)

fclean:		clean
	@ printf "$(YELLOW)$(NAME) removed$(END)\n"
	@ rm -rf $(NAME) fsanitize g3

re:	fclean all

fsanitize :
	$(CC) -fsanitize=address -g $(SRCS) -I $(DIR_INCS) $(CFLAGS) -o $@

g3 :
	$(CC) -g3 $(SRCS) -I $(DIR_INCS) $(CFLAGS) -o $@

.PHONY: all clean fclean re fsanitize g3
