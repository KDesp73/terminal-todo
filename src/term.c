#include <termios.h>
#include <unistd.h>
#include "todo.h"

void enable_input_buffering()
{
    #ifdef _WIN32
        // Enable console input buffering
        HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

        DWORD consoleMode;
        GetConsoleMode(hConsoleInput, &consoleMode);

        consoleMode |= ENABLE_LINE_INPUT;

        SetConsoleMode(hConsoleInput, consoleMode);
    #else
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= (ICANON | ECHO | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);

    #endif
    ANSI_SHOW_CURSOR();
}

void disable_input_buffering()
{
    #ifdef _WIN32
        // Disable console input buffering
        DWORD mode;
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hInput, &mode);
        SetConsoleMode(hInput, mode & ~ENABLE_ECHO_INPUT & ~ENABLE_LINE_INPUT);
    #else
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~(ICANON | ECHO | ISIG);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);

    #endif
    ANSI_HIDE_CURSOR();
}
