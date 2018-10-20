/*
  Minimal Text Editor
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f) // sets first 3 bits to 0, sets value of bit 5 and 6

struct termios og_settings;

// toggles terminal flags
struct termios  toggleFlags(struct termios *_terminal) {
  struct termios terminal = *_terminal;
  terminal.c_cc[VMIN] = 0;
  terminal.c_cc[VTIME] = 1;

  terminal.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  terminal.c_oflag &= ~(OPOST);
  terminal.c_cflag |= (CS8);
  terminal.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  return terminal;
}

/*** terminal ***/
void die(const char *s) {
  editorClearScreen();

  perror(s);
  exit(1);
}

void runSetAttr(struct termios *terminal) {
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH, terminal) == -1)
    die("tcsetarr");
}

void runGetAttr(struct termios *terminal) {
  if (tcgetattr(STDIN_FILENO, terminal) == -1)
    die("tcgetattr");
}

void resetTerminal() {
  runSetAttr(&og_settings);
}

void editorEscapeSequence(char seq[], int numOfBytes) {
  write(STDOUT_FILENO, seq, numOfBytes);
}

// sets the cursor to the left screen
void editorCursorLeftScreen() {
  editorEscapeSequence("\x1b[H", 3);
}

// clears the terminal.
void editorClearScreen() {
  // j clears screen.
  editorEscapeSequence("\x1b[2j", 4);
  editorCursorLeftScreen();
}

// adds a column of tildes on the left column.
void editorDrawRows() {
  struct winsize win_dimensions;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &win_dimensions);
  int ws_row = win_dimensions.ws_row;
  for (int i = 0; i < ws_row; i ++) {
    editorEscapeSequence("~\r\n", 3);
  }
}

// refreshes the screen.
void editorRefreshScreen() {
  editorClearScreen();

  editorDrawRows();

  editorCursorLeftScreen();
}

// toggles terminal between cooked and raw mode.
void manageTerminalMode() {
   runGetAttr(&og_settings);

  // at exit, reset the terminal.
  tcgetattr(STDIN_FILENO, &og_settings);
  atexit(resetTerminal);
  
  struct termios terminal = og_settings;
  // retrieves attributes from the terminal.
  tcgetattr(STDIN_FILENO, &terminal);

  // toggle flags.
  terminal = toggleFlags(&terminal);
  
  // sets new attributes to the terminal
  // TCAFLUSH specifies changes to be made when all output
  // has been written
  runSetAttr(&terminal);
}

/**
 * Terminal
 */
// reads keys from terminal
readKey() {
  int r;
  char c;
  // waits for key press
  while((r = read(STDIN_FILENO, &c, 1)) != 1){
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");
  }

  return c;
}

// maps keys to editor functions
processKeypress() {
  char c = readKey();

  switch (c) {
    case CTRL_KEY('q'):
      editorClearScreen();
      exit(0);
      break;
  }
  
}


/*** init ***/

int main() {
  manageTerminalMode();
  while (1){
   processKeypress();
  }

  return 0;
}
