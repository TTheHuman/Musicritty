#define SDL_MAIN_HANDLED

#include<dirent.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<ncurses.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_mixer.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

typedef struct {
  char *title;
} song;

typedef struct {
  song list[5012];
} queue;

// ---- Frontend ---- //
WINDOW *newWindow(int rows, int cols, int y, int x);
static void updateMenu(int val);
static void displayMenu();
static void displayFilechooser();
static void clearBuffer();
static void destroyWindow(WINDOW *localWin);
static void endProgram();

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
// ---- -------- ---- //

// ---- Backend ---- //
queue *initQueue(song *songs[]);
static void readConfig();
static void loadMusicDir(song *_song[5012]);
static void playFromQueue(Mix_Music *mus, queue *_queue);

char path[1024];
char configFilepath[1024] = "";
char musicDir[1024];
// --- ------- ---- //

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

  readConfig();
  //loadMusicDir(_song); 

  //playFromQueue(music, musicQueue);

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
        //playFromQueue(music, musicQueue);
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

// -------- Frontend -------- //
WINDOW *newWindow(int rows, int cols, int y, int x) {
  WINDOW *localWin;

  localWin = newwin(rows, cols, y, x);
  box(localWin, 0, 0);

  wrefresh(localWin);
  return localWin;
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

static void displayFilechooser() {
  WINDOW *filechooser = newWindow(centerY, centerX, 0, 0);
  refresh();
}

static void clearBuffer() {
  werase(stdscr);
  refresh();
}

static void destroyWindow(WINDOW *localWin) {
  wborder(localWin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  
  wrefresh(localWin);
  delwin(localWin);
}

static void endProgram() {
    endwin();
}
// -------- -------- -------- //

// -------- Backend -------- //
queue *initQueue(song *songs[]) {
  queue *_queue = (queue *)malloc(sizeof(*_queue));
  if(!_queue) { printf("malloc failed"); endwin(); }
  for(int i = 0; i < (sizeof(*_queue->list) / sizeof(_queue->list[0])); i++) {
    _queue->list[i] = *songs[i];
    printf("%s", songs[i]->title);
  }
  return _queue;
}

static void readConfig() {
  size_t bytesRead;
  char *homeDir = getenv("HOME");
  snprintf(path, sizeof(path), "%s/.config/musicritty/", homeDir); 

  char tempConfigDir[1024] = "";  // Empty string to avoid null-terminator issues
  char configFileName[1024] = "config";

  strcat(tempConfigDir, path);
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
  while((bytesRead = fread(musicDir, 1, sizeof(musicDir) - 1, fptr)) > 0) {
    musicDir[bytesRead] = '\0';
  } 
  fclose(fptr);
}

static void loadMusicDir(song *_song[5012]) {
  struct dirent *de;

  printf(musicDir);

  DIR *dir = opendir(musicDir); // TODO: this doesn't work right now because *_song is a single type and you need to create a fucntion that will 
                               // return an array of *song to then initalize the queue with

  if(dir == NULL) { printf("couldn't open current directory!"); endwin(); }

 // For loops are simply too easy for a guy like me....
  while((de = readdir(dir)) != NULL) {
    //_song[0]->title = de->d_name;
    printf("%s", "debug");
  }
  closedir(dir);
}

static void playFromQueue(Mix_Music *mus, queue *_queue) {
  int index = 0;
  char *p = _queue[index].list[index].title;
  printf(p);
  mus = Mix_LoadMUS(p);
  Mix_PlayMusic(mus, 0);
}
// -------- ------- -------- //

