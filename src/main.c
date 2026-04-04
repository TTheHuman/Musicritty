#include<sys/ioctl.h>
#include<stdio.h>
#include<ncurses.h>

static void setMode();
static void endProgram();
static void updateMenu(int val);
static void displayMenu();
static void displayPlayer();

struct winsize w;
unsigned short centerX, centerY;
unsigned short y, x;
const int TITLEOFFSETX     = 11;
const int TITLEOFFSETY     = 6;
const int QUITCUTOFFSETX   = 7;
const int QUITCUTOFFSETY   = 2;
const int VERSIONOFFSETX   = 6;
const int VERSIONOFFSETY   = 3;
const int PLAYERCUTOFFSETX = 11;
const int PLAYERCUTOFFSETY = 3;
const char titleText[]          = "Welcome to Musicritty!";
const char quitShortcutText[]   = "<S-q> to quit";
const char versionText[]        = "vers. 0.0.1";
const char playerShortcutText[] = "<S-e> to enter player";

typedef enum {
  BACK = 81,				         // Shift + q
  CONTINUE = 69 			       // Shift + e
} keys;
typedef enum {
  EXIT,
  MENU,
  CREATION,
  PLAYER
} modes;

modes currentMode;

int main(int** argc, char argv[]) {
  int ch;
  
  ioctl(0, TIOCGWINSZ, &w);

  initscr();				         // Init library
  noecho();				           // Disable echong input in terminal
  cbreak();				           // Dsable buffering input (for security reasons)
  keypad(stdscr, TRUE); 		 // Allow extented key input (function keys, numpad, etc)
  curs_set(1);				       // Allow cursor, set normal apperance
  
  if(has_colors() == FALSE) {
    endwin();
    printf("Your terminal doesn't support colors! Exiting...");
    return 1;
  }

  start_color();
  //init_pair(1, COLOR_WHITE, COLOR_BLUE); // Fore and background colors
  
  //attron(COLOR_PAIR(1));

  getmaxyx(stdscr, y, x);
  centerX = ((w.ws_col / 2));
  centerY = ((w.ws_row / 2));

  currentMode = MENU;
  displayMenu();

  //attroff(COLOR_PAIR(1));
  
  do {
    ch = wgetch(stdscr);
    switch(ch) {
      case CONTINUE:
        currentMode++;
        updateMenu(currentMode);
        break;
      case BACK:
        currentMode--;
        if (currentMode == 0) goto exit;
        updateMenu(currentMode); 
        break;
      default:
        break;
    }
  }while(true);

  exit:
    endwin();
    return 0;
}

static void endProgram() {
    endwin();
}

static void displayMenu() {
  werase(stdscr);
  mvwprintw(stdscr, (centerY - TITLEOFFSETY), (centerX - TITLEOFFSETX), titleText);
  mvwprintw(stdscr, (centerY - VERSIONOFFSETY), (centerX - VERSIONOFFSETX), versionText);
  mvwprintw(stdscr, (centerY + QUITCUTOFFSETY), (centerX - QUITCUTOFFSETX), quitShortcutText);
  mvwprintw(stdscr, (centerY + PLAYERCUTOFFSETY), (centerX - PLAYERCUTOFFSETX), playerShortcutText);
  refresh();
}

static void displayPlayer() {
  werase(stdscr);
//  mvwprintw(stdscr, , 0, "test");
  mvwprintw(stdscr, centerY, centerX, "debug");
  refresh();
}

static void updateMenu(int val) {
  switch(val) {
    case 1:
      displayMenu();
      break;
    case 2:
      displayPlayer();
      break;
    default:
      currentMode--;
      break;
  }
}

