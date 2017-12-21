/*
** server.c for Programmation et Systemes in /home/tom/Documents/Workspace/L2S3_UCA_Chat
** 
** Made by Tom Partouche
** Login   <tom.partouche@kyrah.fr>
** 
** Started on  Wed Dec 20 15:56:19 2017 Tom Partouche
** Last update Thu Dec 21 15:01:02 2017 Tom Partouche
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CLIENTS 8

typedef enum
{
  GET_USERS,
  SET_USERNAME,
  MESSAGE,
  JOIN,
  EXIT
} msgType;

typedef struct  info
{
  int			socket;
  struct sockaddr_in    address;
  char			username[16];
} info;

typedef struct  message
{
  msgType	type;
  char		username[17];
  char		data[256];
} message;

void	launchServer(info *server, int port)
{
  const	int		optValue = 1;
  const socklen_t	optLen = sizeof(optValue);
  
  if ((server->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror("# ERREUR: Impossible de créer le socket.");
      exit(1);
    }

  server->address.sin_family = AF_INET;
  server->address.sin_addr.s_addr = INADDR_ANY;
  server->address.sin_port = htons(port);

  if (bind(server->socket, (struct sockaddr *)&server->address, sizeof(server->address)) < 0)
    {
      perror("# ERREUR: Liaison impossible.");
      exit(1);
    }

  if (setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, (void*) &optValue, optLen) < 0)
    {
      perror("# ERREUR: Impossible de configurer les options du socket.");
      exit(1);
    }

  if (listen(server->socket, 3) < 0)
    {
      perror("# ERREUR: Ecoute impossible.");
      exit(1);
    }
  
  printf("# En attente de connections entrantes...\n");
}

void	stopServer(info server[])
{
  int	i;

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      close(server[i].socket);
    }
  
  exit(0);
}

void	sendMessage(info clients[], int sender, char *msgData)
{
  message	msg;
  int		i;

  msg.type = MESSAGE;
  
  strncpy(msg.username, clients[sender].username, 16);
  strncpy(msg.data, msgData, 256);

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (i != sender && clients[i].socket != 0)
	{
	  if (send(clients[i].socket, &msg, sizeof(msg), 0) < 0)
	    {
	      perror("# ERREUR: Envoi échoué.");
	      exit(1);
	    }
	}
    }
}

void	sendJoinMsg(info *clients, int sender)
{
  message	msg;
  int		i;

  strncpy(msg.username, clients[sender].username, 17);

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket != 0)
	{
	  if (i == sender)
	    {
	      if (send(clients[i].socket, &msg, sizeof(msg), 0) < 0)
		{
		  perror("# ERREUR: Envoi échoué.");
		  exit(1);
		}
	    }
	  else
	    {
	      if (send(clients[i].socket, &msg, sizeof(msg), 0) < 0)
		{
		  perror("# ERREUR: Envoié échoué.");
		  exit(1);
		}
	    }
	}
    }
}

void	sendExitMsg(info *clients, char *username)
{
  message	msg;
  int		i;
  
  strncpy(msg.username, username, 17);

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket != 0)
	{
	  if (send(clients[i].socket, &msg, sizeof(msg), 0) < 0)
	    {
	      perror("# ERREUR: Envoi échoué.");
	      exit(1);
	    }
	}
    }
}

void	sendUserList(info *clients, int user)
{
  message	msg;
  char		*list = msg.data;
  int		i;

  msg.type = GET_USERS;
  
  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket != 0)
	{
	  list = stpcpy(list, clients[i].username);
	  list = stpcpy(list, "\n");
	}
    }

  if (send(clients[user].socket, &msg, sizeof(msg), 0) < 0)
    {
      perror("# ERREUR: Envoi échoué.");
      exit(1);
    }
}

void	rcvMessage(info clients[], int sender)
{
  int		readSize;
  message	msg;

  if ((readSize = recv(clients[sender].socket, &msg, sizeof(message), 0)) == 0)
    {
      printf("# Déconnexion: %s.\n", clients[sender].username);
      close(clients[sender].socket);                                                                   
      clients[sender].socket = 0;
      
      sendExitMsg(clients, clients[sender].username);
    }
  else
    {
      switch(msg.type)
	{
	  case GET_USERS:
	    sendUserList(clients, sender);
	  break;

	  case SET_USERNAME: ;
	    int	i;
	  
	    for (i = 0; i < MAX_CLIENTS; i++)
	      {
	        if (clients[i].socket != 0 && strcmp(clients[i].username, msg.username) == 0)
		  {
		    close(clients[sender].socket);
		    clients[sender].socket = 0;
  
  		    return;
		  }
	      }
	  
	    strcpy(clients[sender].username, msg.username);
	    printf("# Connexion: %s\n", clients[sender].username);

	    sendJoinMsg(clients, sender);
	  break;

	  case MESSAGE:
	    sendMessage(clients, sender, msg.data);
	  break;

	  default:
	    fprintf(stderr, "# ERREUR: Impossible de traiter le message reçu.");
	  break;
	}
    }
}

void	joinServer(info *server, info clients[])
{
  int	newSocket;
  int	addressLen;
  int	i;

  newSocket = accept(server->socket, (struct sockaddr*)&server->address, (socklen_t*)&addressLen);

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket == 0)
	{
	  clients[i].socket = newSocket;
	  break;
	}
    }
}
  
void	userInput(info clients[])
{
  char	input[255];

  fgets(input, sizeof(input), stdin);

  if (input[0] == 'q')
    {
      stopServer(clients);
    }
}

int	constructFileDescriptors(fd_set *set, info *server, info clients[])
{
  int	maxFileDescriptors = server->socket;
  int	i;
  
  FD_ZERO(set);
  FD_SET(STDIN_FILENO, set);
  FD_SET(server->socket, set);

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket > 0)
	{
	  FD_SET(clients[i].socket, set);

	  if (clients[i].socket > maxFileDescriptors)
	    {
	      maxFileDescriptors = clients[i].socket;
	    }
	}
    }

  return maxFileDescriptors;
}
 
int	main(int argc, char *argv[])
{
  fd_set fileDescriptors;
  info	 server;
  info	 clients[MAX_CLIENTS];
  int	 i;
  
  puts("# Lancement du serveur.");

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      clients[i].socket = 0;
    }

  if (argc != 2)
    {
      fprintf(stderr, "# ERREUR: %s <port>\n", argv[0]);
      exit(1);
    }

  launchServer(&server, atoi(argv[1]));

  while (true)
    {
      int	maxFileDescriptors = constructFileDescriptors(&fileDescriptors, &server, clients);

      if (select(maxFileDescriptors+1, &fileDescriptors, NULL, NULL, NULL) < 0)
	{
	  perror("# ERREUR: Selection échouée.");
	  stopServer(clients);
	}
      
      if (FD_ISSET(STDIN_FILENO, &fileDescriptors))
	{
	  userInput(clients);
	}

      if (FD_ISSET(server.socket, &fileDescriptors))
	{
	  joinServer(&server, clients);
	}

      for (i = 0; i < MAX_CLIENTS; i++)
	{
	  if (clients[i].socket > 0 && FD_ISSET(clients[i].socket, &fileDescriptors))
	    {
	      rcvMessage(clients, i);
	    }
	}
    }

  return 0;
}
