#define SDL_MAIN_HANDLED

#include<sys/ioctl.h>
#include<stdio.h>
#include<ncurses.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_mixer.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

WINDOW *newWindow(int rows, int cols, int y, int x);
static void endProgram();
static void displayMenu();
static void clearBuf();
static void displayFilechooser();
static void fileLogic();
static void updateMenu(int val);

char path[1024];
char configFilepath[1024] = "";
char musicDir[1024];

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
const char versionText[]        = "vers. 0.1.0";
const char playerShortcutText[] = "<S-e> to enter player";

typedef enum {
  BACK     = 81,				     // Shift + q
  CONTINUE = 69, 			       // Shift + e
  PLAY     = 80,             // Shift + p
  PAUSE    = 83              // Shift + s
} keys;
typedef enum {
  EXIT,
  MENU, 
  PLAYER
} modes;

modes currentMode;

int main(int** argc, char argv[]) {
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Music* music = NULL;
  Mix_OpenAudio(41000, MIX_DEFAULT_FORMAT, 2, 4096);

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

  if(SDL_Init(SDL_INIT_AUDIO) == -1) {
    printf("could not initalize SDL: %s. \n", SDL_GetError());
    goto exitError;
  }

  getmaxyx(stdscr, y, x);
  centerX = ((w.ws_col / 2));
  centerY = ((w.ws_row / 2));

  currentMode = MENU;
  displayMenu();

  fileLogic();
  
  do {
    ch = wgetch(stdscr);
    switch(ch) {
      case CONTINUE:
        currentMode++;
        updateMenu(currentMode);
        break;
      case BACK:
        currentMode--;
        if (currentMode == EXIT) goto exit;
        updateMenu(currentMode); 
        break;
      case PLAY:
        if (currentMode == MENU) break;
        break;
      default:
        break;
    }
  }while(true);

  exit:
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
    endwin();
    return 0;
  exitError:
    endwin();
    printf("Unknown error");
    return 1;
}

WINDOW *newWindow(int rows, int cols, int y, int x) {
  WINDOW *localWin;

  localWin = newwin(rows, cols, y, x);
  box(localWin, 0, 0);

  wrefresh(localWin);
  return localWin;
}

static void destroyWindow(WINDOW *localWin) {
  wborder(localWin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  
  wrefresh(localWin);
  delwin(localWin);
}

static void endProgram() {
    endwin();
}

static void displayMenu() {
  werase(stdscr);
  mvwprintw(stdscr, 
           (centerY - TITLEOFFSETY), 
           (centerX - TITLEOFFSETX), 
           titleText);
  mvwprintw(stdscr, 
           (centerY - VERSIONOFFSETY), 
           (centerX - VERSIONOFFSETX), 
           versionText);
  mvwprintw(stdscr, 
           (centerY + QUITCUTOFFSETY), 
           (centerX - QUITCUTOFFSETX), 
           quitShortcutText);
  mvwprintw(stdscr, 
           (centerY + PLAYERCUTOFFSETY), 
           (centerX - PLAYERCUTOFFSETX), 
           playerShortcutText);
  refresh();
}

static void clearBuffer() {
  werase(stdscr);
  refresh();
}

static void displayFilechooser() {
  WINDOW *filechooser = newWindow(centerY, centerX, 0, 0);
  refresh();
}

static void fileLogic() {
  char *homeDir = getenv("HOME");
  snprintf(path, sizeof(path), "%s/.config/musicritty/", homeDir);

  char tempConfigDir[1024] = "";  // Empty string to avoid null-terminator issues
  char configFileName[1024] = "config";

  strcat(tempConfigDir, path); //TODO: this
  strcat(tempConfigDir, configFileName);
  strcat(configFilepath, tempConfigDir);

  FILE *fptr = fopen(configFilepath, "r");

  if (mkdir(path, 
            S_IRWXU | 
            S_IRWXG | 
            S_IRWXO) == -1 
            && errno != EEXIST) {
      printf("Error: %s\n", strerror(errno));
  }

  if(fptr == NULL) {
    fptr = fopen(configFilepath, "w");
    snprintf(musicDir, sizeof(musicDir), "%s/Music", homeDir);
    fprintf(fptr, musicDir);
  }
  fclose(fptr);
}

static void updateMenu(int val) {
  switch(val) {
    case 1:
      displayMenu();
      break;
    case 2:
      clearBuffer(); 
      displayFilechooser();
      break;
    default:
      currentMode--;  // To prevent user from going to impossible menus
      break;
  }
}

