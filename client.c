#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

typedef enum
{
    CONNECT,
    DISCONNECT,
    GET_USERS,
    SET_USERNAME,
    PUBLIC_MESSAGE,
    PRIVATE_MESSAGE,
    TOO_FULL,
    USERNAME_ERROR,
    SUCCESS,
    ERROR
} message_type;

typedef struct
{
    message_type type;
    char username[21];
    char data[256];

} message;

typedef struct connection_info
{
    int socket;
    struct sockaddr_in address;
    char username[20];
} connection_info;

void
trim_newline (char *text)
{
    int len = strlen (text) - 1;
    if (text[len] == '\n')
      {
          text[len] = '\0';
      }
}

void
get_username (char *username)
{
    while (true)
      {
          printf ("Enter a username: ");
          fflush (stdout);
          memset (username, 0, 1000);
          fgets (username, 22, stdin);
          trim_newline (username);

          if (strlen (username) > 20)
            {

                puts ("Username must be 20 characters or less.");

            }
          else
            {
                break;
            }
      }
}

void
set_username (connection_info * connection)
{
    message msg;
    msg.type = SET_USERNAME;
    strncpy (msg.username, connection->username, 20);

    if (send (connection->socket, (void *) &msg, sizeof (msg), 0) < 0)
      {
          perror ("Send failed");
          exit (1);
      }
}

void
stop_client (connection_info * connection)
{
    close (connection->socket);
    exit (0);
}

void
connect_to_server (connection_info * connection, char *address, char *port)
{

    while (true)
      {
          get_username (connection->username);

          if ((connection->socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            {
                perror ("Could not create socket");
            }

          connection->address.sin_addr.s_addr = inet_addr (address);
          connection->address.sin_family = AF_INET;
          connection->address.sin_port = htons (atoi (port));

          if (connect (connection->socket, (struct sockaddr *) &connection->address, sizeof (connection->address)) < 0)
            {
                perror ("Connect failed.");
                exit (1);
            }

          set_username (connection);

          message msg;
          ssize_t recv_val = recv (connection->socket, &msg, sizeof (message), 0);
          if (recv_val < 0)
            {
                perror ("recv failed");
                exit (1);

            }
          else if (recv_val == 0)
            {
                close (connection->socket);
                printf ("The username \"%s\" is taken, please try another name.\n", connection->username);
                continue;
            }
          break;
      }

    puts ("Connected to server.");
    puts ("Type /help for usage.");
}

void
handle_user_input (connection_info * connection)
{
    char input[255];
    fgets (input, 255, stdin);
    trim_newline (input);

    if (strcmp (input, "/q") == 0 || strcmp (input, "/quit") == 0)
      {
          stop_client (connection);
      }
    else if (strcmp (input, "/l") == 0 || strcmp (input, "/list") == 0)
      {
          message msg;
          msg.type = GET_USERS;

          if (send (connection->socket, &msg, sizeof (message), 0) < 0)
            {
                perror ("Send failed");
                exit (1);
            }
      }
    else if (strcmp (input, "/h") == 0 || strcmp (input, "/help") == 0)
      {
          puts ("/quit or /q: Exit the program.");
          puts ("/help or /h: Displays help information.");
          puts ("/list or /l: Displays list of users in chatroom.");
          puts ("@<username> <message> Send a private message to given username.");
      }
    else if (strncmp (input, "@", 1) == 0)
      {
          message msg;
          msg.type = PRIVATE_MESSAGE;

          char *toUsername, *chatMsg;

          toUsername = strtok (input + 1, " ");

          if (toUsername == NULL)
            {
                puts (KRED "The format for private messages is: @<username> <message>" RESET);
                return;
            }

          if (strlen (toUsername) == 0)
            {
                puts (KRED "You must enter a username for a private message." RESET);
                return;
            }

          if (strlen (toUsername) > 20)
            {
                puts (KRED "The username must be between 1 and 20 characters." RESET);
                return;
            }

          chatMsg = strtok (NULL, "");

          if (chatMsg == NULL)
            {
                puts (KRED "You must enter a message to send to the specified user." RESET);
                return;
            }

          strncpy (msg.username, toUsername, 20);
          strncpy (msg.data, chatMsg, 255);

          if (send (connection->socket, &msg, sizeof (message), 0) < 0)
            {
                perror ("Send failed");
                exit (1);
            }

      }
    else
      {
          message msg;
          msg.type = PUBLIC_MESSAGE;
          strncpy (msg.username, connection->username, 20);

          if (strlen (input) == 0)
            {
                return;
            }

          strncpy (msg.data, input, 255);

          if (send (connection->socket, &msg, sizeof (message), 0) < 0)
            {
                perror ("Send failed");
                exit (1);
            }
      }

}

void
handle_server_message (connection_info * connection)
{
    message msg;

    ssize_t recv_val = recv (connection->socket, &msg, sizeof (message), 0);
    if (recv_val < 0)
      {
          perror ("recv failed");
          exit (1);

      }
    else if (recv_val == 0)
      {
          close (connection->socket);
          puts ("Server disconnected.");
          exit (0);
      }

    switch (msg.type)
      {

      case CONNECT:
          printf (KYEL "%s has connected." RESET "\n", msg.username);
          break;

      case DISCONNECT:
          printf (KYEL "%s has disconnected." RESET "\n", msg.username);
          break;

      case GET_USERS:
          printf (KMAG "%s" RESET "\n", msg.data);
          break;

      case PUBLIC_MESSAGE:
          printf (KGRN "%s" RESET ": %s\n", msg.username, msg.data);
          break;

      case PRIVATE_MESSAGE:
          printf (KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
          break;

      case TOO_FULL:
          fprintf (stderr, KRED "Server chatroom is too full to accept new clients." RESET "\n");
          exit (0);
          break;

      default:
          fprintf (stderr, KRED "Unknown message type received." RESET "\n");
          break;
      }
}

int
main (int argc, char *argv[])
{
    connection_info connection;
    fd_set file_descriptors;

    if (argc != 3)
      {
          fprintf (stderr, "Usage: %s <IP> <port>\n", argv[0]);
          exit (1);
      }

    connect_to_server (&connection, argv[1], argv[2]);

    while (true)
      {
          FD_ZERO (&file_descriptors);
          FD_SET (STDIN_FILENO, &file_descriptors);
          FD_SET (connection.socket, &file_descriptors);
          fflush (stdin);

          if (select (connection.socket + 1, &file_descriptors, NULL, NULL, NULL) < 0)
            {
                perror ("Select failed.");
                exit (1);
            }

          if (FD_ISSET (STDIN_FILENO, &file_descriptors))
            {
                handle_user_input (&connection);
            }

          if (FD_ISSET (connection.socket, &file_descriptors))
            {
                handle_server_message (&connection);
            }
      }

    close (connection.socket);
    return 0;
}
