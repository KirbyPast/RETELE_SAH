/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.

   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <math.h>
#include <string.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define BOARD_SIZE 8
#define SQUARE_SIZE (SCREEN_WIDTH / BOARD_SIZE)
/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

typedef enum
{
  WHITES = 0,
  BLACKS = 1
} Colors;

char chessboard[9][9] = {
    {' ', '1', '2', '3', '4', '5', '6', '7', '8'},
    {'1', 't', 'c', 'n', 'q', 'k', 'n', 'c', 't'},
    {'2', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'3', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'4', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'5', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'6', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'7', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'8', 'T', 'C', 'N', 'Q', 'K', 'N', 'C', 'T'}};

int isValidMove(int nr1, int nr2, int nr3, int nr4)
{
  char piece = chessboard[nr1][nr2];
  char destination = chessboard[nr3][nr4];

  if (piece == ' ')
    return 0; // No piece to move

  int dx = abs(nr3 - nr1);
  int dy = abs(nr4 - nr2);


  switch (piece)
  {
  case 'p': // Black Pawn
    if (nr3 == nr1 - 1 && nr4 == nr2 && destination == ' ')
      return 1;
    if (nr1 == 7 && nr3 == 5 && nr4 == nr2 && destination == ' ')
      return 1;
    if (nr3 == nr1 - 1 && abs(nr4 - nr2) == 1 && destination >= 'A' && destination <= 'Z')
      return 1;
    break;

  case 'P': // White Pawn
    if (nr3 == nr1 - 1 && nr4 == nr2 && destination == ' ')
      return 1;
    if (nr1 == 7 && nr3 == 5 && nr4 == nr2 && destination == ' ')
      return 1;
    if (nr3 == nr1 - 1 && abs(nr4 - nr2) == 1 && destination >= 'a' && destination <= 'z')
      return 1;
    break;

  case 't':
  case 'T': // Rook
    if (nr1 == nr3 || nr2 == nr4)
    {
      int xDir = (nr3 > nr1) ? 1 : (nr3 < nr1 ? -1 : 0);
      int yDir = (nr4 > nr2) ? 1 : (nr4 < nr2 ? -1 : 0);
      for (int i = nr1 + xDir, j = nr2 + yDir; i != nr3 || j != nr4; i += xDir, j += yDir)
      {
        if (chessboard[i][j] != ' ')
          return 0;
      }
      return 1;
    }
    break;

  case 'n':
  case 'N': // Bishop
    if (dx == dy)
    {
      int xDir = (nr3 > nr1) ? 1 : -1;
      int yDir = (nr4 > nr2) ? 1 : -1;
      for (int i = nr1 + xDir, j = nr2 + yDir; i != nr3; i += xDir, j += yDir)
      {
        if (chessboard[i][j] != ' ')
          return 0;
      }
      return 1;
    }
    break;

  case 'q':
  case 'Q': // Queen
    if (dx == dy || nr1 == nr3 || nr2 == nr4)
    {
      int xDir = (nr3 > nr1) ? 1 : (nr3 < nr1 ? -1 : 0);
      int yDir = (nr4 > nr2) ? 1 : (nr4 < nr2 ? -1 : 0);
      for (int i = nr1 + xDir, j = nr2 + yDir; i != nr3 || j != nr4; i += xDir, j += yDir)
      {
        if (chessboard[i][j] != ' ')
          return 0;
      }
      return 1;
    }
    break;

  case 'k':
  case 'K': // King
    if (dx <= 1 && dy <= 1)
      return 1;
    break;

  case 'c':
  case 'C': // Knight
    if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2))
      return 1;
    break;
  }

  return 0; // Invalid move
}

int isStartingPosValid(int x, int y, int color)
{

  if (chessboard[x][y] == ' ')
  {
    printf("Move %d%d is invalid because the chessboard is %c\n", x, y, chessboard[x][y]);
    return 0;
  }

  if (color == WHITES)
  {
    if ((chessboard[x][y] >= 'a' && chessboard[x][y] <= 'z'))
    {
      printf("Move %d%d is invalid because the chessboard is %c\n", x, y, chessboard[x][y]);
      return 0;
    }
    else
      return 1;
  }
  else if (color == BLACKS)
  {
    if ((chessboard[x][y] >= 'A' && chessboard[x][y] <= 'Z'))
    {
      printf("Move %d%d is invalid because the chessboard is %c\n", x, y, chessboard[x][y]);
      return 0;
    }
    else
      return 1;
  }

  return 0;
}

int isEndingPosValid(int x, int y, int color)
{

  if (color == WHITES)
  {
    if ((chessboard[x][y] >= 'A' && chessboard[x][y] <= 'Z'))
    {
      // printf("Move %d%d is invalid because the chessboard is %c (You can't capture your own pieces)\n", x, y, chessboard[x][y]);
      return 0;
    }
    else
      return 1;
  }
  else if (color == BLACKS)
  {
    if ((chessboard[x][y] >= 'a' && chessboard[x][y] <= 'z'))
    {
      // printf("Move %d%d is invalid because the chessboard is %c (You can't capture your own pieces)\n", x, y, chessboard[x][y]);
      return 0;
    }
    else
      return 1;
  }

  return 0;
}

int checkConcede(char buf[10], int sd)
{

  if (strcmp(buf, "concede\n") == 0)
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        chessboard[i][j] = '0';
      }
    }
    // Transmitem server-ului tabla de surrender
    if (write(sd, &chessboard, sizeof(chessboard)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }

    // Inchidem conexiunea
    close(sd);

    return 1;
  }

  return 0;
}

void printMatrix(int color)
{
  printf("-------------------------------------------------------------\n");
  if (color == WHITES)
  {
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
  }
  else
  {
    for (int i = 8; i >= 0; i--)
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
  }
  printf("-------------------------------------------------------------\n");
}

int updateMatrix(int nr1, int nr2, int nr3, int nr4, int sd)
{
  if (chessboard[nr3][nr4] == ' ')
  {
    char temp;
    temp = chessboard[nr1][nr2];
    chessboard[nr1][nr2] = chessboard[nr3][nr4];
    chessboard[nr3][nr4] = temp;
  }
  else if (chessboard[nr3][nr4] == 'K' || chessboard[nr3][nr4] == 'k')
  {
    for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
      {
        chessboard[i][j] = '1';
      }
    }
    printf("You Win!\n");
    if (write(sd, &chessboard, sizeof(chessboard)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }
    return -1;
  }
  else
  {
    chessboard[nr3][nr4] = chessboard[nr1][nr2];
    chessboard[nr1][nr2] = ' ';
  }
  return 1;
}

void drawMatrix(Texture2D *assets)
{
  for (int row = 0; row < BOARD_SIZE; row++)
  {
    for (int col = 0; col < BOARD_SIZE; col++)
    {
      // Alternate colors
      Texture2D piece;
      Color color = ((row + col) % 2 == 0) ? DARKGRAY : LIGHTGRAY;
      DrawRectangle(col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);

      if (chessboard[row + 1][col + 1] != ' ')
      {
        switch (chessboard[row + 1][col + 1])
        {
        case 'K':
          piece = assets[0];
          break;
        case 'Q':
          piece = assets[1];
          break;
        case 'T':
          piece = assets[2];
          break;
        case 'C':
          piece = assets[3];
          break;
        case 'N':
          piece = assets[4];
          break;
        case 'P':
          piece = assets[5];
          break;
        case 'k':
          piece = assets[6];
          break;
        case 'q':
          piece = assets[7];
          break;
        case 't':
          piece = assets[8];
          break;
        case 'c':
          piece = assets[9];
          break;
        case 'n':
          piece = assets[10];
          break;
        case 'p':
          piece = assets[11];
          break;
        }
        DrawTextureEx(piece, (Vector2){col * SQUARE_SIZE, row * SQUARE_SIZE}, 0.0f, 1.25f, WHITE);
      }
    }
  }
}

void flipBoard()
{
  char c = ' ';
  for (int i = 1; i < 5; i++)
  {
    for (int j = 1; j < 9; j++)
    {
      // swap(chessboard[i][j],chessboard[9-i][j]);
      c = chessboard[i][j];
      chessboard[i][j] = chessboard[9 - i][j];
      chessboard[9 - i][j] = c;
    }
  }
}

Vector2 getTile(Vector2 click)
{
  Vector2 new = {floor(click.x / SQUARE_SIZE) + 1, floor(click.y / SQUARE_SIZE) + 1};
  return new;
}

int main(int argc, char *argv[])
{

  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
                             // mesajul trimis
  int nr = 0;
  int nr1, nr2, nr3, nr4;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  // Odata ce ne-am conectat, afisam tabla initiala:

  int color;
  // color = 0 (WHITES/upper) color = 1 (BLACKS/lower)
  if (read(sd, &color, sizeof(int)) < 0)
  {
    perror("Eroare la citire!");
    return -1;
  }

  fcntl(sd, F_SETFL, O_NONBLOCK);
  fcntl(0, F_SETFL, O_NONBLOCK);

  printMatrix(color);
  /* citirea mesajului */
  int count = 0;
  int turn = 0;
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chessboard with Raylib");
  Texture2D assets[12];
  assets[0] = LoadTexture("assets/wk.png");
  assets[1] = LoadTexture("assets/wq.png");
  assets[2] = LoadTexture("assets/wr.png");
  assets[3] = LoadTexture("assets/wkn.png");
  assets[4] = LoadTexture("assets/wb.png");
  assets[5] = LoadTexture("assets/wp.png");
  assets[6] = LoadTexture("assets/bk.png");
  assets[7] = LoadTexture("assets/bq.png");
  assets[8] = LoadTexture("assets/br.png");
  assets[9] = LoadTexture("assets/bkn.png");
  assets[10] = LoadTexture("assets/bb.png");
  assets[11] = LoadTexture("assets/bp.png");
  Vector2 start = {-9, -9};
  Vector2 end = {-9, -9};
  int loseFlag = 0;
  int recFlag = 0;
  int winFlag = 0;
  if (color == WHITES)
  {
    turn = 1;

    while (!WindowShouldClose())
    {
      BeginDrawing();
      if (loseFlag == 1)
      {
        ClearBackground(RAYWHITE);
        DrawText("YOU LOSE", SCREEN_HEIGHT / 4, SCREEN_WIDTH / 4, 75, RED);
        // sleep(2);
        // return -1;
      }
      else if (winFlag == 1)
      {
        ClearBackground(RAYWHITE);
        DrawText("YOU WIN", SCREEN_HEIGHT / 4, SCREEN_WIDTH / 4, 75, GREEN);
        // sleep(2);
        // return -1;
      }
      else
      {
        ClearBackground(RAYWHITE);

        // Draw the chessboard
        drawMatrix(assets);
        if(recFlag){
          DrawRectangleLines((start.x - 1)* SQUARE_SIZE, (start.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
        }

        // OData ce am introdus o miscare valida, afisam tabla cu schimbarea facuta.
        if (count == 0 && turn == 1)
        {
          printf("[client]Introduceti o miscare [FORMAT: oldXoldY newXnewY]: \n");
          fflush(stdout);
          count++;
        }
        if (read(0, buf, sizeof(buf)) > 0 && turn == 1)
        {
          // fflush(stdout);
          if (checkConcede(buf, sd))
          {

            /*ClearBackground(RAYWHITE);

            DrawText("YOU WIN", 100, 100, 50, GREEN);
            sleep(2);
            return -1;*/
            winFlag = 1;
          }

          nr1 = buf[0] - '0';
          nr2 = buf[1] - '0';

          nr3 = buf[3] - '0';
          nr4 = buf[4] - '0';

          // De inlocuit aceasta secventa de mai sus cu un check de click.

          count = 0;
          while (!isStartingPosValid(nr1, nr2, color) || !isEndingPosValid(nr3, nr4, color))
          {
            if (!isStartingPosValid(nr1, nr2, color) && count == 0)
            {
              printf("Invalid move! Try again! (You're WHITES, you can only move uppercase pieces.) \n");
            }
            if (!isEndingPosValid(nr3, nr4, color) && count == 0)
            {
              printf("Invalid move! You cannot capture your own pieces!\n");
            }
            if (count == 0)
            {
              printf("[client]Introduceti o miscare [FORMAT: oldXoldY newXnewY]: ");
            }
            count++;
            // fflush(stdout);
            if (read(0, buf, sizeof(buf)) > 0)
            {

              if (checkConcede(buf, sd))
              {
                /*ClearBackground(RAYWHITE);

                DrawText("YOU WIN", 100, 100, 50, GREEN);
                sleep(2);
                return -1;*/
                winFlag = 1;
              }
              nr1 = buf[0] - '0';
              nr2 = buf[1] - '0';

              nr3 = buf[3] - '0';
              nr4 = buf[4] - '0';

              printf("[client] Am citit %d\n", nr);
            }
          }
          if (updateMatrix(nr1, nr2, nr3, nr4, sd) == -1)
          {
            // sleep(2);
            // return -1;
            winFlag = 1;
          }
          if (write(sd, &chessboard, sizeof(chessboard)) > 0)
          {
            printf("super)\n");
          }
          printMatrix(color);
          turn = 0;
          count = 0;
        }
        if (read(sd, &chessboard, sizeof(chessboard)) > 0)
        {

          printMatrix(color);

          if (chessboard[0][0] == '0')
          {
            /*ClearBackground(RAYWHITE);

            printf("I win!\n");
            DrawText("YOU WIN", 100, 100, 50, GREEN);
            sleep(2);
            return -1;*/
            winFlag = 1;
          }
          if (chessboard[0][0] == '1')
          {
            /*
            ClearBackground(RAYWHITE);

            printf("I lose :(\n");
            DrawText("YOU LOSE", 100, 100, 50, RED);
            sleep(2);
            return -1;*/
            loseFlag = 1;
          }
          turn = 1;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && start.x != -9 && turn == 1)
        {
          end = getTile(GetMousePosition());
          printf("Mouse button moved to tile %f,%f\n", end.x, end.y);
          fflush(stdout);

          int ok = 1;

          int nr1 = (int)start.y;
          int nr2 = (int)start.x;

          int nr3 = (int)end.y;
          int nr4 = (int)end.x;

          // printf("%d,%d,%d,%d\n", nr1, nr2, nr3, nr4);

          if (!isStartingPosValid(nr1, nr2, color))
          {
            printf("Invalid move! Try again! (You're WHITES, you can only move uppercase pieces.) \n");
            ok = 0;
          }
          if (!isEndingPosValid(nr3, nr4, color))
          {
            printf("Invalid move! You cannot capture your own pieces!\n");
            ok = 0;
          }
          if (!isValidMove(nr1, nr2, nr3, nr4))
          {
            ok = 0;
          }
          if (ok)
          {

            if (updateMatrix(nr1, nr2, nr3, nr4, sd) == -1)
            {
              /*ClearBackground(RAYWHITE);

              DrawText("YOU WIN", 100, 100, 50, GREEN);
              sleep(2);
              return -1;*/
              winFlag = 1;
            }
            if (write(sd, &chessboard, sizeof(chessboard)) > 0)
            {
              printf("super)\n");
            }
            printMatrix(color);
            turn = 0;

            start.x = -9;
            start.y = -9;
            end.x = -9;
            end.y = -9;
            count = 0;
            recFlag = 0;
            fflush(stdout);
          }
          else
          {
            
            start.x = -9;
            start.y = -9;
            end.x = -9;
            end.y = -9;
          }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && start.x == -9 && turn == 1)
        {
          start = getTile(GetMousePosition());
          printf("Mouse button pressed at tile %f,%f\n", start.x, start.y);
          recFlag = 1;
        }
      }
      EndDrawing();
      // printMatrix(color);
    }
  }
  else if (color == BLACKS)
  {
    turn = 0;
    while (!WindowShouldClose())
    {
      BeginDrawing();
      if (loseFlag == 1)
      {
        ClearBackground(RAYWHITE);
        DrawText("YOU LOSE", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 75, RED);
        // sleep(2);
        // return -1;
      }
      else if (winFlag == 1)
      {
        ClearBackground(RAYWHITE);
        DrawText("YOU WIN", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 75, GREEN);
        // sleep(2);
        // return -1;
      }
      else
      {
        ClearBackground(RAYWHITE);

        flipBoard();
        // Draw the chessboard
        drawMatrix(assets);
        if(recFlag){
          DrawRectangleLines((start.x - 1)* SQUARE_SIZE, (start.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
        }

        flipBoard();


        // printMatrix(color);
        //  Cand suntem negru, mai intai citim chessboard
        if (count == 0 && turn == 1)
        {
          printf("[client]Introduceti o miscare [FORMAT: oldXoldY newXnewY]: \n");
          fflush(stdout);
          count++;
        }
        if (read(sd, &chessboard, sizeof(chessboard)) > 0)
        {
          printMatrix(color);
          if (chessboard[0][0] == '0')
          {
            /*printf("I win!\n");
            ClearBackground(RAYWHITE);

            DrawText("YOU WIN", 100, 100, 50, GREEN);
            sleep(2);
            return -1;*/
            winFlag = 1;
          }
          if (chessboard[0][0] == '1')
          {
            /*ClearBackground(RAYWHITE);

            DrawText("YOU LOSE", 100, 100, 50, RED);
            printf("I lose :(\n");
            sleep(2);
            return -1;*/
            loseFlag = 1;
          }
          turn = 1;
        }
        if (read(0, buf, sizeof(buf)) > 0 && turn == 1)
        {
          if (checkConcede(buf, sd))
          {
            /*ClearBackground(RAYWHITE);

            DrawText("YOU WIN", 100, 100, 50, GREEN);
            sleep(2);
            return -1;*/
            winFlag = 1;
          }

          nr1 = buf[0] - '0';
          nr2 = buf[1] - '0';

          nr3 = buf[3] - '0';
          nr4 = buf[4] - '0';

          count = 0;
          while (!isStartingPosValid(nr1, nr2, color) || !isEndingPosValid(nr3, nr4, color))
          {
            if (!isStartingPosValid(nr1, nr2, color) && count == 0)
            {
              printf("Invalid move! Try again! (You're WHITES, you can only move uppercase pieces.) \n");
            }
            if (!isEndingPosValid(nr3, nr4, color) && count == 0)
            {
              printf("Invalid move! You cannot capture your own pieces!\n");
            }
            if (count == 0)
            {
              printf("[client]Introduceti o miscare [FORMAT: oldXoldY newXnewY]: ");
            }
            count++;
            fflush(stdout);
            if (read(0, buf, sizeof(buf)) > 0)
            {

              if (checkConcede(buf, sd))
              {
                /*
                ClearBackground(RAYWHITE);
                sleep(2);
                DrawText("YOU WIN", 100, 100, 50, GREEN);
                return -1;*/
                winFlag = 1;
              }
              nr1 = buf[0] - '0';
              nr2 = buf[1] - '0';

              nr3 = buf[3] - '0';
              nr4 = buf[4] - '0';

              printf("[client] Am citit %d\n", nr);
            }
          }

          if (updateMatrix(nr1, nr2, nr3, nr4, sd) == -1)
          {
            ClearBackground(RAYWHITE);

            winFlag = 1;
          }
          if (write(sd, &chessboard, sizeof(chessboard)) > 0)
          {
            printf("super\n");
          }
          printMatrix(color);

          turn = 0;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && start.x != -9 && turn == 1)
        {
          flipBoard();
          int ok = 1;
          end = getTile(GetMousePosition());

          printf("Mouse button moved to tile %f,%f\n", end.x, end.y);
          fflush(stdout);

          int nr1 = (int)start.y;
          int nr2 = (int)start.x;

          int nr3 = (int)end.y;
          int nr4 = (int)end.x;

          if (!isStartingPosValid(nr1, nr2, color))
          {
            printf("Invalid move! Try again! (You're WHITES, you can only move uppercase pieces.) \n");
            ok = 0;
          }
          if (!isEndingPosValid(nr3, nr4, color))
          {
            printf("Invalid move! You cannot capture your own pieces!\n");
            ok = 0;
          }
          if (!isValidMove(nr1, nr2, nr3, nr4))
          {
            ok = 0;
          }
          if (ok)
          {

            if (updateMatrix(nr1, nr2, nr3, nr4, sd) == -1)
            {
              /*ClearBackground(RAYWHITE);

              DrawText("YOU WIN", 100, 100, 50, GREEN);
              sleep(2);
              return -1;*/
              winFlag = 1;
            }
            flipBoard();
            if (write(sd, &chessboard, sizeof(chessboard)) > 0)
            {
              printf("super)\n");
            }
            printMatrix(color);
            turn = 0;

            start.x = -9;
            start.y = -9;
            end.x = -9;
            end.y = -9;
            count = 0;
          recFlag = 0;
            fflush(stdout);
          }
          else
          {
            flipBoard();
            start.x = -9;
            start.y = -9;
            end.x = -9;
            end.y = -9;
          }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && start.x == -9 && turn == 1)
        {
          start = getTile(GetMousePosition());
          printf("Mouse button pressed at tile %f,%f\n", start.x, start.y);
          recFlag = 1;
        }
        // Apoi scriem miscarea
        // printf("[client]Introduceti o miscare [FORMAT: oldXoldY newXnewY]: ");
        // fflush(stdout);

        // printMatrix(color);
      }
      EndDrawing();
    }
  }
  /* inchidem conexiunea, am terminat */
  close(sd);
}
