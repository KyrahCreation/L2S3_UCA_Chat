/*
** client.c for Programmation et Systemes in /home/tom/Documents/Workspace/L2S3_UCA_Chat
** 
** Made by Tom Partouche
** Login   <tom.partouche@kyrah.fr>
** 
** Started on  Wed Dec 20 15:56:38 2017 Tom Partouche
** Last update Sat Dec 30 21:04:02 2017 Tom Partouche
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef enum
{
  GET_USERS,
  SET_USERNAME,
  MESSAGE,
  JOIN,
  EXIT
} msgType;

typedef struct	info
{
  int			socket;
  struct sockaddr_in	address;
  char			username[16];
} info;

typedef struct	message
{
  msgType	type;
  char		username[17];
  char		data[256];
} message;

void	trimLine(char *txt)
{
  int	len;

  len = strlen(txt) - 1;

  if (txt[len] == '\n')
    {
      txt[len] = '\0';
    }
}

void    getUsername(char *username)
{
  while (true)
    {
      fflush(stdout);
      memset(username, 0, 1000);

      printf("# Entrez un nom d'utilisateur: ");
      fgets(username, 18, stdin);
      trimLine(username);
      
      if (strlen(username) > 16)
        {
          puts("# ERREUR: Entrez un nom d'utilisateur de 16 caractères ou moins.");
        }
      else
        {
          break;
        }
    }
}

void    sendUsername(info *connection)
{
  message       msg;

  msg.type = SET_USERNAME;
  strncpy(msg.username, connection->username, 16);

  if (send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0)
    {
      perror("# ERREUR: Envoi échoué.");
      exit(1);
    }
}

void	joinServer(info *connection, char *address, char *port)
{
  while (true)
    {
      getUsername(connection->username);

      if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
	{
	  perror("# ERREUR: Impossible de créer le socket.");
	}

      connection->address.sin_addr.s_addr = inet_addr(address);
      connection->address.sin_family = AF_INET;
      connection->address.sin_port = htons(atoi(port));

      if (connect(connection->socket, (struct sockaddr *)&connection->address, sizeof(connection->address)) < 0)
       {
	 perror("# ERREUR: Connexion échouée.");
	 exit(1);
       }

       sendUsername(connection);

       message   msg;
       ssize_t   msgSize = recv(connection->socket, &msg, sizeof(message), 0);
       
       if (msgSize < 0)
	 {
	   perror("# ERREUR: Réception échouée.");
	   exit(1);
	 }
       else if (msgSize == 0)
	 {
	   close(connection->socket);
	   printf("# ERREUR: Nom d'utilisateur déjà attribué.");
	   continue;
	 }
       
       break;
    }

  puts("# Connexion réussie. <Appuyer sur ENTRER>");
}

void	exitChat(info *connection)
{
  close(connection->socket);
  exit(0);
}

void	userInput(info *connection)
{
  char		input[255];
  message	msg;

  strncpy(msg.username, connection->username, 16);
  printf("[%s]: ", msg.username);
  fgets(input, 255, stdin);
  trimLine(input);
  
  if (strcmp(input, "/q") == 0 || strcmp(input, "/quit") == 0)
    {
      exitChat(connection);
    }
  else if (strcmp(input, "/l") == 0 || strcmp(input, "/list") == 0)
    {
      message	msg;
      
      msg.type = GET_USERS;
      
      if (send(connection->socket, &msg, sizeof(message), 0) < 0)
	{
	  perror("# ERREUR: Envoi échoué.");
	  exit(1);
	}
    }
  else
    {
      message	msg;

      msg.type = MESSAGE;
      strncpy(msg.username, connection->username, 16);

      if (strlen(input) == 0)
	{
	  return;
	}

      strncpy(msg.data, input, 255);
      
      if (send(connection->socket, &msg, sizeof(message), 0) < 0)
	{
	  perror("# ERREUR: Envoi échoué.");
	  exit(1);
	}
    }
}

void	serverOutput(info *connection)
{
  message	msg;
  ssize_t	msgSize = recv(connection->socket, &msg, sizeof(message), 0);

  if (msgSize < 0)
    {
      perror("# ERREUR: Réception échouée.");
      exit(1);
    }
  else if (msgSize == 0)
    {
      close(connection->socket);
      puts("# ERREUR: Serveur déconnecté.");
      exit(0);
    }

  switch(msg.type)
    {
      case JOIN:
	printf("# %s s'est connecté.\n", msg.username);
      break;

      case EXIT:
	printf("# %s s'est déconnecté.\n", msg.username);
      break;

      case GET_USERS:
	printf("# %s", msg.data);
      break;

      case MESSAGE:
	printf("[%s]: %s\n", msg.username, msg.data);
      break;

      default:
	fprintf(stderr, "# ERREUR: Impossible de traiter le message reçu.\n");
      break;
    }
}

int	main(int argc, char *argv[])
{
  info		connection;
  fd_set	fileDescriptors;

  if (argc != 3)
    {
      fprintf(stderr, "# ERREUR: %s <IP> <port>\n", argv[0]);
      exit(1);
    }

  joinServer(&connection, argv[1], argv[2]);

  while (true)
    {
      FD_ZERO(&fileDescriptors);
      FD_SET(STDIN_FILENO, &fileDescriptors);
      FD_SET(connection.socket, &fileDescriptors);
      fflush(stdin);

      if (select(connection.socket+1, &fileDescriptors, NULL, NULL, NULL) < 0)
	{
	  perror("# ERREUR: Selection échouée.");
	  exit(1);
	}
      
      if (FD_ISSET(STDIN_FILENO, &fileDescriptors))
	{
	  userInput(&connection);
	}
      
      if (FD_ISSET(connection.socket, &fileDescriptors))
	{
	  serverOutput(&connection);
	}
    }

  close(connection.socket);
  return 0;
}
