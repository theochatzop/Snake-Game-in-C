#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define DOWN 0
#define UP 1
#define RIGHT 2
#define LEFT 3
#define QUIT 4

#define HEIGHT 25
#define WIDTH 50

const int speed = 100000;

typedef struct Fruit
{
  int fx, fy;
} fruit;

typedef struct Snake
{
  int length;
  int headX, headY, prev_headX, prev_headY;
  int bodyX, bodyY;
  int snake_array[HEIGHT * WIDTH][2];
} snake;

typedef struct Direction
{
  char dir, prev_dir;
} direction;

typedef struct Score
{
  int total_score;
} score;

void clrscr(void);
void gotoxy(int x, int y);
int kbhit(void);
void welcome(void);
int menu(snake *s, fruit *f, direction *d, score *scr);
void instructions(snake *s, fruit *f, direction *d, score *scr);
void init_game(snake *s, direction *d, score *scr);
void generate_fruit(snake *s, fruit *f);
void print_fruit(fruit *f);
void draw_boundary(void);
void print_snake(snake *s);
int eat_fruit(snake *s, fruit *f, score *scr);
void check_input(direction *d);
void move_head(snake *s, direction *d);
void move_snake(snake *s, direction *d);
int touch_borders(snake *s);
int detect_collision(int x_pos, int y_pos, snake *s);
int detect_exit(direction *d);
void game_over(score *scr);
void print_score(score *scr);
void load_game(snake *s, fruit *f, direction *d, score *scr);

int main(int argc, char *argv[])
{

  fruit f;
  snake s;
  direction d;
  score scr;
  
  /* Hide Cursor */
  printf("\e[?25l"); 

  welcome();
  menu(&s, &f, &d, &scr);

  return 0;
}

/* Clear Screen */
void clrscr(void)
{
  system("clear");
  return;
}

/* Move Cursor at Specified Location (x,y) in the Screen */
void gotoxy(int x, int y)
{
  /* 0x1B: The Escape Character (ESC) in the ASCII Coding System. So, What Follows Becomes an ANSI Escape Sequence
     '[': Control Sequence Introducer (CSI) (Standard Escape Codes are Prefixed With ESC Followed by the Command, Somtimes Delimited by CSI
     'f': Indicates Cursor Positioning
     <ESC>[{ROW};{COLUMN}f : Move Cursor to Row #, Column #  */
  printf("%c[%d;%df", 0x1B, y, x);
}

/* Determine if a Key has been Pressed */
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  /* tcgetattr: Gets the Parameters of the Current Terminal
     STDIN_FILENO: Tells tcgetattr that it should write the settings of stdin to oldt */
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt; // Copy to newt
  /* Turn off the Following c_lflag: ECHO, ICANON
     ECHO: Displays the Input of the User in the Terminal
     ICANON: Enables canonical Mode (Input is Made Available Line by Line) */
  newt.c_lflag &= ~(ICANON | ECHO);
  /* tcsetattr: Sets the Following Settings to STDIN
     TCSANOW: The Change Occurs Immediately */
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  /* Get STDIN's Flags */
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  /* Make STDIN Non-Blocking */
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  /* Write the Old Settings Back to STDIN */
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  /* Restore STDIN's Old Flags */
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    /* Take a Single Character and Push it back to STDIN */
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

/* Welcome Screen */
void welcome(void)
{

  clrscr();

  do
  {

    printf("\n");
    printf("\t            /^\\/^\\ \n");
    printf("\t         _ |__|  O| \n");
    printf("\t \\/     /~     \\_/ \\ \n");
    printf("\t  \\____|__________/  \\ \n");
    printf("\t        \\_______      \\ \n");
    printf("\t                `\\     \\    \n");
    printf("\t                   |     |                  \\ \n");
    printf("\t                 /      /                    \\ \n");
    printf("\t                /     /                       \\\\ \n");
    printf("\t              /      /                         \\ \\ \n");
    printf("\t             /     /                            \\  \\ \n");
    printf("\t           /     /             _----_            \\   \\ \n");
    printf("\t          /     /           _-~      ~-_         |   | \n");
    printf("\t         (      (        _-~    _--_    ~-_     _/   | \n");
    printf("\t          \\      ~-____-~    _-~    ~-_    ~-_-~    / \n");
    printf("\t            ~-_           _-~          ~-_       _-~ \n");
    printf("\t               ~--______-~                ~-___-~ \n");

    printf("\n\n\n");
    printf("\t		Welcome to the SNAKE GAME!			\n");
    printf("\t			    Press Enter To Continue...	\n");
    printf("\n");

  } while (getchar() != '\n');

  return;
}

/* Display Menu */
int menu(snake *s, fruit *f, direction *d, score *scr)
{

  int input;

  clrscr();

  printf("\n\n\t Start: 1\n");
  printf("\t Instructions: 2\n");
  printf("\t Exit: 3\n");

  scanf("%d", &input);

  switch (input)
  {

  case 1:
    load_game(s, f, d, scr);
    break;

  case 2:
    instructions(s, f, d, scr);
    break;

  case 3:
    exit(-1);
    break;

  default:
    break;
  }
}

/* Display Instructions */
void instructions(snake *s, fruit *f, direction *d, score *scr)
{

  int input;

  clrscr();

  printf("\n\n");

  printf("\t      _              _                       _    _ \n");
  printf("\t (_) _ __   ___ | |_  _ __  _   _   ___ | |_ (_)  ___   _ __   ___  _ \n");
  printf("\t | || '_ \\ / __|| __|| '__|| | | | / __|| __|| | / _ \\ | '_ \\ / __|(_)\n");
  printf("\t | || | | |\\__ \\| |_ | |   | |_| || (__ | |_ | || (_) || | | |\\__ \\ _ \n");
  printf("\t |_||_| |_||___/ \\|  |_|    \\__,_| \\___| \\__||_| \\___/ |_| |_||___/(_)\n");

  printf("\n\n");
  printf("\t * Use the 'w' key to go up\n");
  printf("\t * Use the 'a' key to turn left\n");
  printf("\t * Use the 's' key to go down\n");
  printf("\t * Use the 'd' key to turn right\n");
  printf("\t * Use the 'x' key to quit\n");
  printf("\n");
  printf("\t DO NOT touch the borders\n");
  printf("\t DO NOT reverse movement\n");
  printf("\t DO NOT eat your own tail\n");
  printf("\n");
  printf("\t Main Menu: 0\n");

  scanf("%d", &input);

  switch (input)
  {

  case 0:
    menu(s, f, d, scr);
    break;

  default:
    break;
  }
}

/* Initialize Snake Position and Total Score */
void init_game(snake *s, direction *d, score *scr)
{

  s->length = 2;
  s->bodyX = 29;
  s->bodyY = 10;
  s->headY = 10;
  s->headX = 30;

  d->dir = RIGHT;
  d->prev_dir = DOWN;

  scr->total_score = 0;
}

/* Generate Fruit (Random x,y Coordinates) Within the Boundary */
void generate_fruit(snake *s, fruit *f)
{

  do
  {
    srand(time(NULL));

    f->fx = rand() % (WIDTH - 2) + 2;
    f->fy = rand() % (HEIGHT - 6) + 2;
  
  /* Prevent Food From Getting Printed onto Snake */
  } while (detect_collision(f->fx, f->fy, s)); 
}

/* Print Fruit */
void print_fruit(fruit *f)
{

  gotoxy(f->fx, f->fy);
  printf("X");
}

/* Draw Boundary */
void draw_boundary(void)
{

  clrscr();

  for (int j = 0; j < HEIGHT - 4; j++)
  {
    gotoxy(1, j);
    printf("│");

    gotoxy(WIDTH, j);
    printf("│");
  }

  for (int i = 0; i < WIDTH; i++)
  {
    gotoxy(i, 1);
    printf("─");

    gotoxy(i, HEIGHT - 4);
    printf("─");
  }

  /* Upper Left Corner */
  gotoxy(1, 1);
  printf("┌");
  
  /* Upper Right Corner */
  gotoxy(WIDTH, 1);
  printf("┐");
  
  /* Lower Left Corner */
  gotoxy(1, HEIGHT - 4);
  printf("└");
  
  /* Lower Right Corner */
  gotoxy(WIDTH, HEIGHT - 4);
  printf("┘");

  return;
}

/* Print Snake */
void print_snake(snake *s)
{
  int i;

  /* Print Snake's Head */
  gotoxy(s->headX, s->headY);
  printf("@");

  /* Print Snake's Tail */
  for (i = 0; i < s->length; i++)
  {
    s->snake_array[i][0] = s->bodyX;
    s->snake_array[i][1] = s->bodyY;
    gotoxy(s->bodyX, s->bodyY);
    printf("*");
    s->bodyX -= 1;
  }
}

/* Check if Snake has Eaten the Fruit */
int eat_fruit(snake *s, fruit *f, score *scr)
{

  if (((s->headX == f->fx) && (s->headY == f->fy)))
  { 
    /* Beep Sound */
    printf("\7"); 
    /* Increase Score by 10 Points */         
    scr->total_score += 10; 
    return 1;
  }

  return 0;
}

/* Take Input From the Keyboard (W: UP, A: LEFT, S: DOWN, D: RIGHT, X: QUIT) */
void check_input(direction *d)
{

  if (kbhit())
  {

    switch (getchar())

    {

    case 'a':
    case 'A':
      if (d->dir == RIGHT)
        break;

      d->prev_dir = d->dir;
      d->dir = LEFT;
      break;

    case 'd':
    case 'D':
      if (d->dir == LEFT)
        break;

      d->prev_dir = d->dir;
      d->dir = RIGHT;
      break;

    case 'w':
    case 'W':
      if (d->dir == DOWN)
        break;

      d->prev_dir = d->dir;
      d->dir = UP;
      break;

    case 's':
    case 'S':
      if (d->dir == UP)
        break;

      d->prev_dir = d->dir;
      d->dir = DOWN;
      break;

    case 'x':
    case 'X':
      d->dir = QUIT;
      break;

    default:
      break;
    }
  }
}

/* Move Snake's Head According to Keyboard Input */
void move_head(snake *s, direction *d)
{
  switch (d->dir)
  {

  case RIGHT:
    s->prev_headX = s->headX;
    s->prev_headY = s->headY;
    s->headX++;
    break;

  case LEFT:
    s->prev_headX = s->headX;
    s->prev_headY = s->headY;
    s->headX--;
    break;

  case UP:
    s->prev_headX = s->headX;
    s->prev_headY = s->headY;
    s->headY--;
    break;

  case DOWN:
    s->prev_headX = s->headX;
    s->prev_headY = s->headY;
    s->headY++;
    break;

  default:
    break;
  }
}

/* Move Snake According to Keyboard Input */
void move_snake(snake *s, direction *d)
{
  int i;

  move_head(s, d);

  /* Remove Last Cell of Tail */
  gotoxy(s->snake_array[(s->length) - 1][0], s->snake_array[(s->length) - 1][1]);
  printf(" ");

  /* Print Head */
  gotoxy(s->headX, s->headY);
  printf("@");

  /* Replace Head By Cell of Tail */
  gotoxy(s->prev_headX, s->prev_headY);
  printf("*");

  /* Move Snake Body */
  for (i = (s->length) - 1; i > 0; i--)
  {
    s->snake_array[i][0] = s->snake_array[i - 1][0];
    s->snake_array[i][1] = s->snake_array[i - 1][1];
  }

  /* Move First Cell of Tail */
  s->snake_array[0][0] = s->prev_headX;
  s->snake_array[0][1] = s->prev_headY;

  /* Update the Upper Left Corner of the Boundary */
  gotoxy(1, 1);
  printf("┌");
}

/* Check if Snake's Head has Touched the Boundary */
int touch_borders(snake *s)
{

  if ((s->headX == WIDTH) || (s->headX == 1) || (s->headY == HEIGHT - 4) || (s->headY == 1))
  {
    return 1;
  }

  return 0;
}

/* Check if Snake's Tail has Collided With its Head or With the Fruit and if Snake's Head has Touched the Boundary */
int detect_collision(int x_pos, int y_pos, snake *s)
{
  int i;
  int collision = 0;

  for (i = 0; i < s->length - 1; i++)
  {
    if ((s->snake_array[i][0] == x_pos) && (s->snake_array[i][1] == y_pos))
    {
      collision = 1;
      break;
    }
  }

  if (collision || (touch_borders(s)))
    return 1;

  return 0;
}

/* Check if Player has Pressed 'X' Key (Quit) */
int detect_exit(direction *d)
{

  if (d->dir == QUIT)
    return 1;

  return 0;
}

/* Game Over */
void game_over(score *scr)
{
  gotoxy(1, 1);

  printf("\n\n");
  printf("  _______  _______  _______  _______    _______           _______  _______ \n");
  printf("(  ____ \\(  ___  )(       )(  ____ \\  (  ___  )|\\     /|(  ____ \\(  ____ )\n");
  printf("| (    \\/| (   ) || () () || (    \\/  | (   ) || )   ( || (    \\/| (    )|\n");
  printf("| |      | (___) || || || || (__      | |   | || |   | || (__    | (____)|\n");
  printf("| | ____ |  ___  || |(_)| ||  __)     | |   | |( (   ) )|  __)   |     __)\n");
  printf("| | \\_  )| (   ) || |   | || (        | |   | | \\ \\_/ / | (      | (\\ (   \n");
  printf("| (___) || )   ( || )   ( || (____/\\  | (___) |  \\   /  | (____/\\| ) \\ \\__\n");
  printf("(_______)|/     \\||/     \\|(_______/  (_______)   \\_/   (_______/|/   \\__/\n");
  printf("\n\n");

  printf("Final Score: %d\n", scr->total_score);
}

/* Print Total Score */
void print_score(score *scr)
{

  gotoxy(5, 26);
  printf("Score: %d", scr->total_score);
  return;
}

/* Start Game */
void load_game(snake *s, fruit *f, direction *d, score *scr)
{

  init_game(s, d, scr);
  generate_fruit(s, f);

  /* Turn Off Input Echoing */
  system("stty -echo");

  draw_boundary();
  print_snake(s);
  print_fruit(f);
  print_score(scr);

  /* Set Input to Raw (Input is Directly Sent to STDIN Without Having Been Processed) */
  system("/bin/stty raw");

  while (!(detect_collision(s->headX, s->headY, s)) && !(detect_exit(d)))
  {
    /* Set the Speed Of Snake Movement */
    usleep(speed);
    check_input(d);
    move_snake(s, d);

    if (eat_fruit(s, f, scr))
    {

      s->length++;
      generate_fruit(s, f);
      print_fruit(f);
      print_score(scr);
    }
  }
  /* Reset Terminal Behaviour to Normal Behaviour */
  system("/bin/stty cooked");
  /* Turn On Input Echoing */
  system("stty echo");
  /* \033: ESC (Octal)
     <ESC>c: Reset all Terminal Settings to Default
     Used to Clear Screen */
  printf("\033c");

  game_over(scr);
}
