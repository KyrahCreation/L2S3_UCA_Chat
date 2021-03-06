##
## Makefile for Programmation et Systemes in /home/tom/Documents/Workspace/L2S3_UCA_Chat
## 
## Made by Tom Partouche
## Login   <tom.partouche@kyrah.fr>
## 
## Started on  Wed Dec 20 15:57:21 2017 Tom Partouche
## Last update Thu Dec 21 14:41:36 2017 Tom Partouche
##

NAMES	=	client		\
		server

SRCS	=	client.c	\
		server.c

OBJS	=	$(SRCS:.c=.o)

all:		$(NAMES)

$(NAMES):	$(OBJS)
		gcc -Wall -c $(SRCS)
		gcc -o client client.o
		gcc -o server server.o

clean:
		rm -f $(OBJS)

fclean:		clean
		rm -f $(NAMES)

re:		fclean all
