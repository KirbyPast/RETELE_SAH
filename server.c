/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
  Intoarce corect identificatorul din program al thread-ului.


   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept
  int cl2;
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

char chessboard[9][9];

int main()
{
  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  // int nr; // mesajul primit de trimis la client
  int sd; // descriptorul de socket
  // int pid;
  pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
  int i = 0;

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    int client2;
    thData *td; // parametru functia executata de thread
    unsigned int length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // client= malloc(sizeof(int));
    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    if ((client2 = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */

    // int idThread; //id-ul threadului
    // int cl; //descriptorul intors de accept

    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;
    td->cl2 = client2;
    pthread_create(&th[i], NULL, &treat, td);

  } // while
};
static void *treat(void *arg)
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());

  raspunde((struct thData *)arg);
  /* am terminat cu acest client, inchidem conexiunea */
  close(tdL.cl);
  return (NULL);
};

void raspunde(void *arg)
{
  // int nr;
  struct thData tdL;
  tdL = *((struct thData *)arg);

  int color = 0;
  if (write(tdL.cl, &color, sizeof(color)) < 0)
  {
    printf("[Thread %d]\n", tdL.idThread);
    perror("Eroare la write() la client.\n");
  }
  color++;

  if (write(tdL.cl2, &color, sizeof(color)) < 0)
  {
    printf("[Thread %d]\n", tdL.idThread);
    perror("Eroare la write() la client.\n");
  }

  while (1)
  {
    // Client1

    if (read(tdL.cl, &chessboard, sizeof(chessboard)) <= 0)
    {
      printf("[Thread %d]\n", tdL.idThread);
      perror("Eroare la read() de la client.\n");
    }

    printf("[Thread %d]Mesajul a fost receptionat...\n", tdL.idThread);
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        if (chessboard[i][j] == ' ')
        {
          printf("_ ");
        }
        else
        {
          printf("%c ", chessboard[i][j]);
        }
      }
      printf("\n");
    }

    if (chessboard[0][0] == '0')
    {
      printf("Player has conceded.\n");
      //char buff[10] = "You win!";
      write(tdL.cl2, &chessboard, sizeof(chessboard));
      return;
    }
    else if (chessboard[0][0] == '1')
    {
      printf("Player has won.\n");
      //char buff[10] = "You lose!";
      write(tdL.cl2, &chessboard, sizeof(chessboard));
      return;
    }
    else
    {
      /*pregatim mesajul de raspuns */
      printf("[Thread %d]Trimitem mesajul inapoi...\n", tdL.idThread);
      //char buff[10] = "Am primit!";
      /* returnam mesajul clientului */
      if (write(tdL.cl2, &chessboard, sizeof(chessboard)) <= 0)
      {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
      }
      else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    }

    // Client2

    if (read(tdL.cl2, &chessboard, sizeof(chessboard)) <= 0)
    {
      printf("[Thread %d]\n", tdL.idThread);
      perror("Eroare la read() de la client.\n");
    }

    printf("[Thread %d]Mesajul a fost receptionat...\n", tdL.idThread);
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        if (chessboard[i][j] == ' ')
        {
          printf("_ ");
        }
        else
        {
          printf("%c ", chessboard[i][j]);
        }
      }
      printf("\n");
    }

    if (chessboard[0][0] == '0')
    {
      printf("Player has conceded.\n");
      //char buff[10] = "You win!";
      write(tdL.cl, &chessboard, sizeof(chessboard));

      return;
    }
    else if (chessboard[0][0] == '1')
    {
      printf("Player has won.\n");
      //char buff[10] = "You lose!";
      write(tdL.cl, &chessboard, sizeof(chessboard));
      return;
    }
    else
    {
      /*pregatim mesajul de raspuns */
      printf("[Thread %d]Trimitem mesajul inapoi...\n", tdL.idThread);
      //char buff[10] = "Am primit!";
      /* returnam mesajul clientului */
      if (write(tdL.cl, &chessboard, sizeof(chessboard)) <= 0)
      {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
      }
      else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
    }
  }
}

// In loc de sah mat, daca mananci regele ai castigat
