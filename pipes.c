#include <bits/time.h>
#include <wchar.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

struct point {
        short x;
        short y;
};

int prob = 10;
int delay = 10000;
wchar_t charSet[] = L"┃┏┓┛━┗";
int rows, columns;
struct winsize w;

int keyDown() {
  struct timeval tv = {0, 0};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

char getChar() {
  char ch;
  if (read(STDIN_FILENO, &ch, 1) < 0) {
    return '\0';
  }
  return ch;
}

void hide_cursor() {
  wprintf(L"\033[?25l"); // ANSI escape sequence to hide the cursor
  fflush(stdout);
}

void disable_input() {
  struct termios new_settings;
  tcgetattr(STDIN_FILENO, &new_settings);
  new_settings.c_lflag &= ~ICANON;
  new_settings.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

void enable_alternate_buffer() {
  wprintf(L"\033[?1049h"); // Enable alternate buffer
  wprintf(L"\033[?25l");   // Hide cursor
}

void disable_alternate_buffer() {
  wprintf(L"\033[?25h");   // Show cursor
  wprintf(L"\033[?1049l"); // Disable alternate buffer
}

void moveCursorTo(int row, int col) {
  wprintf(L"\033[%d;%dH", row, col); // Move cursor to the (row, col) position
}

void sig_handler(int signal) {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term); // Get current terminal settings

  // Restore terminal settings
  term.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
  disable_alternate_buffer(); // Restore the cursor and terminal
  exit(0);
}

void checkWindowDimensions() {
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  if (rows != w.ws_row || columns != w.ws_col) {
        system("clear");
        rows = w.ws_row, columns = w.ws_col;
  }
}

void init() {
  signal(SIGINT, sig_handler);
  srand(time(NULL));

  disable_input();
  enable_alternate_buffer(); // enable alternative buffering for better screen clearing support
}

int main() {
        init();
        enum direction { Up, Left, Down, Right };
        enum direction dir;
        enum direction lastDir;
        freopen(NULL, "w", stdout);
        setlocale(LC_ALL, "en_US.utf8");

        struct point lastPoint;
        lastPoint.x = columns / 2;
        lastPoint.y = rows / 2;
        int count = 0;
        while (1) {
                if(keyDown()) {
                char c = getChar();
                switch(c) {
                        case '+':
                                if(prob > 2)
                                        prob--;
                                break;
                        case '-':
                                if(prob < 100)
                                        prob++;
                                break;
                        case 'e':
                                if(delay > 500)
                                        delay -= 500;
                                break;
                        case 'q':
                                if(delay < 50000)
                                        delay += 500;
                                break;
                        default:
                                break;
                }
                }
                checkWindowDimensions();
                if(lastPoint.x == -1) {
                        short x = 0, y = 0;
                        if(rand() % 2 == 0) {
                                x = rand() % columns;
                                if(rand() % 2 == 0) {
                                        y = rows;
                                        dir = Up;
                                }
                                else
                                        dir = Down;
                        }
                        else {
                                y = rand() % rows;
                                if(rand() % 2 == 0) {
                                        x = columns;
                                        dir = Left;
                                }
                                else
                                        dir = Right;
                        }
                        lastPoint.x = x;
                        lastPoint.y = y;
                }
                        moveCursorTo(lastPoint.y, lastPoint.x);
                        short i = 0;
                        switch(dir) {
                                case Up:
                                        if(lastDir == Left)
                                                i = 5;
                                        else if(lastDir == Right)
                                                i = 3;

                                        lastPoint.y--;
                                        break;
                                case Down:
                                        if(lastDir == Left)
                                                i = 1;
                                        else if(lastDir == Right)
                                                i = 2;

                                        lastPoint.y++;
                                        break;
                                case Left:
                                        i = 4;
                                        if(lastDir == Down)
                                                i = 3;
                                        else if(lastDir == Up)
                                                i = 2;
                                        lastPoint.x--;
                                        break;
                                case Right:
                                        i = 4;
                                        if(lastDir == Down)
                                                i = 5;
                                        else if(lastDir == Up)
                                                i = 1;
                                        lastPoint.x++;
                                        break;
                        }
                        wprintf(L"%lc", charSet[i]);
                        fflush(stdout);
                        if(lastPoint.x < 0 || lastPoint.x > columns || lastPoint.y < 0 || lastPoint.y > rows) {
                                lastPoint.x = -1; //RESET
                                int rndColor = 90 + rand() % 7 + 1;
                                if(rndColor == 93)
                                        rndColor = 33;
                                wprintf(L"\x1b[%dm", rndColor);
                                if(count > rows * columns / 2) {
                                        system("clear");
                                        count = 0;
                                }
                        }

                lastDir = dir;

                if(rand() % prob == 0) {
                        if(rand() % 2 == 0)
                                dir = (dir + 1) % 4;
                        else
                                dir = (dir - 1 + 4) % 4;
                }
                usleep(delay);
                count++;
        }
        return 0;
}