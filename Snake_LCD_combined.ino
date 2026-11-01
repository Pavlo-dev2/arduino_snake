#include <Wire.h>
#include <math.h>
#include <stdlib.h>
#include <LiquidCrystal_I2C.h>

#define LINE_WITH 4
#define LINE_HIGHT 2
#define CHAR_WITH 5
#define CHAR_HIGHT 8

#define X_JOISTICK_PIN 3
#define Y_JOISTICK_PIN 2
#define EXIT_BUTTON_PIN 8

#define HEIGTH 16
#define LENGTH 16

#define SPEEDDEV 0.75
#define MINTIME 0.1
#define STARTTIME 0.5

LiquidCrystal_I2C lcd(0x27, 16, 2);

//Score
int applecount = 0;

// Pixel buffer from LCD_pixel_control.ino
byte display[LINE_WITH][LINE_HIGHT * CHAR_HIGHT] = {{B0, B0}, {B11111111, B0}};

// Snake linked-list element
typedef struct listelement
{
  long length;
  void *el;
  char type;
  struct listelement *next;
} listelement;

// Linked-list functions used by the original Snake logic
listelement* append(void *value, long length, char type, listelement *pn, long pos);
listelement* pop(listelement *pn, long pos);
int copyarr(void *to, void *from, char type, long length);
long len(listelement *pn);
int rmlist(listelement *pn);
listelement* retel(listelement *st, long pos);
int printel(listelement* pn, char btw[]);

// Snake game functions
listelement *movesnake(char d, listelement *st, int x, int y, int length, int heigth, char apple);
int checksnake(listelement *snake);
int createapple(char *feld, int length, int heigth, int snakelen, int r);
int checkapple(listelement *snake, int apple, int length);

// Field functions
char *createfeld(int heigth, int length);
int updatesnake(listelement *snake, char *feld, int length, int heigth);
int drawfeld(char *st, int heigth, int length);

// LCD joystick and pixel functions
char get_x_direction();
char get_y_direction();
byte checkcords(int x, int y);
void setpixel(int x, int y, byte value);
void fillwithvalue(byte value);
void draw(byte display[LINE_WITH][LINE_HIGHT * CHAR_HIGHT]);

// Game input
char getdir(long double time, int snakelen);

// -----------------------------------------------------------------------------
// LCD_pixel_control.ino: joystick direction
// -----------------------------------------------------------------------------

char get_x_direction()
{
  int x_val = map(analogRead(X_JOISTICK_PIN), 0, 1014, 1, 7);

  if (x_val > 5)
    return 'r';
  else if (x_val < 3)
    return 'l';
  else
    return 'm';
}

char get_y_direction()
{
  int y_val = map(analogRead(Y_JOISTICK_PIN), 0, 1014, 7, 1);

  if (y_val > 5)
    return 'u';
  else if (y_val < 3)
    return 'd';
  else
    return 'm';
}

// -----------------------------------------------------------------------------
// LCD_pixel_control.ino: pixel buffer functions
// -----------------------------------------------------------------------------

byte checkcords(int x, int y)
{
  byte xline = display[(x - (x % CHAR_WITH)) / CHAR_WITH][y];
  xline = xline >> (CHAR_WITH - (x % CHAR_WITH) - 1);
  xline = xline << 7;

  switch (xline)
  {
    case B10000000:
      return B1;
    case B00000000:
      return B0;
  }

  return B0;
}

void setpixel(int x, int y, byte value)
{
  int power_x = (8 - CHAR_WITH) - ((x % CHAR_WITH) - 1);

  if (checkcords(x, y) == value)
    return;

  switch (value)
  {
    case B0:
      display[(x - (x % CHAR_WITH)) / CHAR_WITH][y] =
        display[(x - (x % CHAR_WITH)) / CHAR_WITH][y] & ~(1 << power_x);
      return;

    case B1:
      display[(x - (x % CHAR_WITH)) / CHAR_WITH][y] =
        display[(x - (x % CHAR_WITH)) / CHAR_WITH][y] | (1 << power_x);
      return;
  }
}

void fillwithvalue(byte value)
{
  for (int y = 0; y < LINE_HIGHT * CHAR_HIGHT; y++)
  {
    for (int x = 0; x < LINE_WITH; x++)
    {
      display[x][y] = value;
    }
  }
}

void draw(byte display[LINE_WITH][LINE_HIGHT * CHAR_HIGHT])
{
  int slot = 0;
  int charpos_x, charpos_y;

  for (charpos_y = 0; charpos_y < LINE_HIGHT; charpos_y++)
  {
    for (charpos_x = 0; charpos_x < LINE_WITH; charpos_x++)
    {
      byte customChar[8] = {B0};
      int pixelpos_x, pixelpos_y;

      for (pixelpos_y = 0; pixelpos_y < CHAR_HIGHT; pixelpos_y++)
      {
        customChar[pixelpos_y] =
          display[charpos_x][charpos_y * CHAR_HIGHT + pixelpos_y];
      }

      lcd.createChar(slot, customChar);
      lcd.setCursor(charpos_x, charpos_y);
      lcd.write(byte(slot));
      slot++;
    }
  }
  //show right border
  lcd.setCursor(4, 0);
  lcd.print("Score:");
  lcd.setCursor(4, 1);
  lcd.print(applecount);
}

// -----------------------------------------------------------------------------
// linkedlistlib.c: functions required by the Snake implementation
// -----------------------------------------------------------------------------

listelement* append(void *value, long length, char type, listelement *pn, long pos)
{
  listelement *st = pn;
  listelement *bf = NULL;
  listelement *af = NULL;

  listelement *newElement = (listelement*)malloc(sizeof(listelement));
  if (newElement == NULL)
    return NULL;

  newElement->type = type;
  newElement->length = length;

  void *pvalue = NULL;

  switch (type)
  {
    case 'f':
      pvalue = malloc(sizeof(float) * length);
      break;
    case 'c':
      pvalue = malloc(sizeof(char) * length);
      break;
    case 'i':
      pvalue = malloc(sizeof(int) * length);
      break;
    default:
      free(newElement);
      return NULL;
  }

  if (pvalue == NULL)
  {
    free(newElement);
    return NULL;
  }

  copyarr(pvalue, value, type, length);
  newElement->el = pvalue;
  newElement->next = NULL;

  if (pn == NULL)
  {
    if (pos != 0 && pos != -1)
    {
      free(newElement->el);
      free(newElement);
      return NULL;
    }
    return newElement;
  }

  if (pos == 0)
  {
    af = pn;
    st = newElement;
    st->next = af;
    return st;
  }
  else if (pos == 1)
  {
    bf = pn;
    if (bf->next == NULL)
      bf->next = newElement;
    else
    {
      af = bf->next;
      bf->next = newElement;
      bf->next->next = af;
    }
    return st;
  }
  else if (pos == -1)
  {
    listelement *ae;
    for (ae = pn; ae->next != NULL; ae = ae->next)
      ;
    ae->next = newElement;
    return st;
  }
  else if (pos > 1)
  {
    if (pos > len(pn))
    {
      free(newElement->el);
      free(newElement);
      return NULL;
    }

    long ap = 0;
    listelement *ae = pn;
    while (ap + 1 != pos)
    {
      ae = ae->next;
      ap++;
      if (ae == NULL)
      {
        free(newElement->el);
        free(newElement);
        return NULL;
      }
    }

    bf = ae;
    af = ae->next;
    bf->next = newElement;
    bf->next->next = af;
    return pn;
  }

  free(newElement->el);
  free(newElement);
  return NULL;
}

listelement* pop(listelement* pn, long pos)
{
  if (pn == NULL)
    return NULL;
  if (pos > len(pn) - 1)
    return NULL;

  listelement *st = pn;
  listelement *bf = NULL;
  listelement *af = NULL;
  long apos = 0;

  if (pos == 0 || (pos == -1 && st->next == NULL))
  {
    free(st->el);
    if (st->next != NULL)
    {
      af = st->next;
      free(st);
      return af;
    }
    free(st);
    return NULL;
  }
  else if (pos > 0 || pos == -1)
  {
    for (listelement *ae = st; (apos < pos || pos == -1) && ae->next != NULL; apos++)
    {
      bf = ae;
      ae = ae->next;
    }

    if (bf->next->next != NULL)
      af = bf->next->next;
    else
      af = NULL;

    free(bf->next->el);
    free(bf->next);
    bf->next = af;
    return st;
  }

  return NULL;
}

int copyarr(void *to, void *from, char type, long length)
{
  while (length > 0)
  {
    switch (type)
    {
      case 'f':
        ((float*)to)[length - 1] = ((float*)from)[length - 1];
        break;
      case 'c':
        ((char*)to)[length - 1] = ((char*)from)[length - 1];
        break;
      case 'i':
        ((int*)to)[length - 1] = ((int*)from)[length - 1];
        break;
    }
    length--;
  }
  return 0;
}

long len(listelement *pn)
{
  long length = 0;
  while (pn != NULL)
  {
    length++;
    pn = pn->next;
  }
  return length;
}

int rmlist(listelement *pn)
{
  listelement *next;
  while (pn != NULL)
  {
    next = pn->next;
    free(pn->el);
    free(pn);
    pn = next;
  }
  return 0;
}

listelement* retel(listelement *st, long pos)
{
  if (st == NULL)
    return NULL;
  if (pos > (len(st) - 1) || pos < 0)
    return NULL;

  listelement* act = st;
  long ap = 0;
  while (ap < pos)
  {
    ap++;
    act = act->next;
  }
  return act;
}

int printel(listelement* pn, char btw[])
{
  if (pn == NULL)
    return 1;

  long i = 0;
  long elementLength = pn->length;

  switch (pn->type)
  {
    case 'f':
      while (i < elementLength)
      {
        Serial.print(((float*)pn->el)[i]);
        Serial.print(btw);
        i++;
      }
      break;

    case 'c':
      while (i < elementLength)
      {
        Serial.print(((char*)pn->el)[i]);
        Serial.print(btw);
        i++;
      }
      break;

    case 'i':
      while (i < elementLength)
      {
        Serial.print(((int*)pn->el)[i]);
        Serial.print(btw);
        i++;
      }
      break;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// snake.c: original game logic
// -----------------------------------------------------------------------------

listelement *movesnake(char d, listelement *st, int x, int y, int length, int heigth, char apple)
{
  char p = 0;
  int new_x, new_y;

  if (st == NULL)
  {
    p = 1;
    new_y = y;
    new_x = x;
  }
  else
  {
    new_x = ((int*)(retel(st, 0)->el))[0];
    new_y = ((int*)(retel(st, 0)->el))[1];

    switch (d)
    {
      case 'u':
        new_y--;
        break;
      case 'd':
        new_y++;
        break;
      case 'r':
        new_x++;
        break;
      case 'l':
        new_x--;
        break;
    }
  }

  if (new_x < 0 || new_y < 0 || new_x > 15 || new_y > 15)
  {
    rmlist(st);
    return NULL;
  }

  int cords_arr[] = {new_x, new_y};
  st = append(cords_arr, 2, 'i', st, 0);

  if ((!apple) && !p)
    st = pop(st, -1);

  return st;
}

int checksnake(listelement *snake)
{
  int tot, act, length;
  length = len(snake);
  tot = act = 0;

  while (tot < length)
  {
    int totvelx = *(int*)(retel(snake, tot)->el);
    int totvely = ((int*)(retel(snake, tot)->el))[1];
    listelement actel;

    for (act = tot + 1; act < length; act++)
    {
      actel = *retel(snake, act);
      if (totvelx == *(int*)actel.el && totvely == ((int*)actel.el)[1])
        return 1;
    }
    tot++;
  }
  return 0;
}

int createapple(char *feld, int length, int heigth, int snakelen, int r)
{
  if (r == -1)
  {
    // Count free cells first, then locate the randomly selected free cell.
    // This avoids a large temporary array on the Arduino's limited SRAM.
    int available = 0;

    for (int i = 0; i < length * heigth; i++)
    {
      if (feld[i] != '#')
        available++;
    }

    if (available == 0)
      return -1;

    int selected = rand() % available;
    for (int i = 0; i < length * heigth; i++)
    {
      if (feld[i] != '#')
      {
        if (selected == 0)
        {
          feld[i] = '$';
          return i;
        }
        selected--;
      }
    }

    return -1;
  }

  feld[r] = '$';
  return r;
}

int checkapple(listelement *snake, int apple, int length)
{
  if (snake != NULL &&
      ((*(int*)snake->el) + (length * (((int*)(snake->el))[1])) == apple))
    return 1;

  return 0;
}

// -----------------------------------------------------------------------------
// feld.c: game field and rendering adapted to the LCD pixel buffer
// -----------------------------------------------------------------------------

char *createfeld(int heigth, int length)
{
  char *st = (char*)malloc(sizeof(char) * heigth * length);
  if (st == NULL)
    return NULL;

  for (int i = 0; i < heigth * length; i++)
    st[i] = '.';

  return st;
}

int updatesnake(listelement *snake, char *feld, int length, int heigth)
{
  for (int i = 0; i < heigth * length; i++)
    feld[i] = '.';

  while (snake != NULL)
  {
    int x = ((int*)snake->el)[0];
    int y = ((int*)snake->el)[1];
    feld[x + (y * length)] = '#';
    snake = snake->next;
  }

  return 0;
}

int drawfeld(char *st, int heigth, int length)
{
  fillwithvalue(B0);

  for (int y = 0; y < heigth; y++)
  {
    for (int x = 0; x < length; x++)
    {
      if (st[x + (y * length)] == '#' || st[x + (y * length)] == '$')
        setpixel(x, y, B1);
    }
  }

  draw(display);
  return 0;
}

// -----------------------------------------------------------------------------
// main.c: getdir adapted from Linux keyboard input to the joystick
// -----------------------------------------------------------------------------

char getdir(long double time, int snakelen)
{
  static char dir = 'u';

  delay((unsigned long)(time * 1000));

  if (digitalRead(EXIT_BUTTON_PIN) == LOW)
    return 'E';

  char x_dir = get_x_direction();
  char y_dir = get_y_direction();
  char requested = dir;

  // Prefer a vertical joystick movement if both axes are moved.
  if (y_dir == 'u' || y_dir == 'd')
    requested = y_dir;
  else if (x_dir == 'l' || x_dir == 'r')
    requested = x_dir;

  // Keep the original game's rule: the snake cannot reverse into itself.
  switch (requested)
  {
    case 'u':
      if (dir != 'd' || snakelen == 0)
        dir = 'u';
      break;

    case 'r':
      if (dir != 'l' || snakelen == 0)
        dir = 'r';
      break;

    case 'd':
      if (dir != 'u' || snakelen == 0)
        dir = 'd';
      break;

    case 'l':
      if (dir != 'r' || snakelen == 0)
        dir = 'l';
      break;
  }

  return dir;
}

// -----------------------------------------------------------------------------
// Arduino setup and game loop
// -----------------------------------------------------------------------------

void setup()
{
  pinMode(EXIT_BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  fillwithvalue(B0);

  srand(analogRead(A0));

  char *feld = createfeld(HEIGTH, LENGTH);
  if (feld == NULL)
  {
    lcd.clear();
    lcd.print("Memory error");
    while (true) {}
  }

  listelement *snake = movesnake('u', NULL, 0, 15, LENGTH, HEIGTH, 0);
  if (snake == NULL)
  {
    lcd.clear();
    lcd.print("Snake error");
    while (true) {}
  }

  // Put the initial snake on the field before generating the apple.
  updatesnake(snake, feld, LENGTH, HEIGTH);

  int apple = createapple(feld, LENGTH, HEIGTH, len(snake), -1);

  long double slt = STARTTIME;
  char a = checkapple(snake, apple, LENGTH);

  drawfeld(feld, HEIGTH, LENGTH);

  while (true)
  {
    char c = getdir(slt, applecount);

    if (c == 'E')
      break;

    if (a)
    {
      applecount++;
      if (slt * SPEEDDEV >= MINTIME)
        slt *= SPEEDDEV;
    }

    snake = movesnake(c, snake, 0, 0, LENGTH, HEIGTH, a);

    if (snake == NULL)
      break;

    if (checksnake(snake))
    {
      Serial.println("You hit yourself...");
      break;
    }

    updatesnake(snake, feld, LENGTH, HEIGTH);

    a = checkapple(snake, apple, LENGTH);

    if (a)
      apple = createapple(feld, LENGTH, HEIGTH, len(snake), -1);
    else
      apple = createapple(feld, LENGTH, HEIGTH, len(snake), apple);

    if (apple == -1)
      break;

    drawfeld(feld, HEIGTH, LENGTH);
  }

  Serial.print("Score: ");
  Serial.println(applecount);

  fillwithvalue(B0);
  draw(display);
  lcd.setCursor(0, 0);
  lcd.print("Game over");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(applecount);

  rmlist(snake);
  free(feld);
}

void loop()
{
  // The game is executed once from setup().
  // Reset the Arduino to start a new game.
}
