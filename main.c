#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>

typedef struct Screen {
	HANDLE out;
	HANDLE in;
} Console;

#define CONSOLE_BLACK 0
#define CONSOLE_BLUE 1
#define CONSOLE_GREEN 2
#define CONSOLE_CYAN 3
#define CONSOLE_RED 4
#define CONSOLE_MAGENTA 5
#define CONSOLE_YELLOW 6
#define CONSOLE_WHITE 7
#define CONSOLE_GRAY 8
#define CONSOLE_BRIGHT_BLUE 9
#define CONSOLE_BRIGHT_GREEN 10
#define CONSOLE_BRIGHT_CYAN 11
#define CONSOLE_BRIGHT_RED 12
#define CONSOLE_BRIGHT_MAGENTA 13
#define CONSOLE_BRIGHT_YELLOW 14
#define CONSOLE_BRIGHT_WHITE 15

#define FATAL(message)  ErrorExit(__FILE__, __func__, __LINE__, message)
#define DO_OR_DIE(m, z) if(!z) {FATAL(m);}

void print_error(TCHAR* message) {
	HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
	DWORD written = 0;
	DWORD size = (DWORD)lstrlen(message);
	WriteConsole(hStderr, message, size, &written, NULL);
}

void ErrorExit(LPCTSTR file_name, LPCTSTR function_name, int lineno, LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code
	LPTSTR lpMsgBuf;
	LPTSTR lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf,
		0, NULL);
	// Size of generated message
	SIZE_T gen_messages = ((SIZE_T)lstrlen((LPCTSTR)lpMsgBuf)) + ((SIZE_T)lstrlen((LPCTSTR)lpszFunction));
	// Size of resulting buffer with gen_messages + 50 for the mask
	SIZE_T debug_info = ((SIZE_T)lstrlen(file_name)) + ((SIZE_T)lstrlen(function_name));
	SIZE_T newbuffersize = (gen_messages + debug_info + 50);
	SIZE_T newbuffersize_in_bytes = newbuffersize * sizeof(TCHAR);

	// Display the error message and exit the process
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, newbuffersize_in_bytes);
	if (lpDisplayBuf == NULL) {
		print_error("Fatal, OOM");
	}
	else {
		StringCchPrintf((LPTSTR)lpDisplayBuf,
			newbuffersize,
			TEXT("%s:%s:%d %s failed with error %d: %s"), file_name, function_name, lineno, lpszFunction, dw, lpMsgBuf);
			MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK | MB_ICONERROR);
			LocalFree(lpDisplayBuf);
    }
    LocalFree(lpMsgBuf);
	ExitProcess(dw);
}


/*
Modified clear the screen
Source: https://docs.microsoft.com/en-us/windows/console/clearing-the-screen
*/
void console_clear(Console *screen)
{
   COORD coordScreen = { 0, 0 };    // home for the cursor 
   DWORD cCharsWritten;
   CONSOLE_SCREEN_BUFFER_INFO csbi; 
   DWORD dwConSize;
   // Get the number of character cells in the current buffer. 
   
   DO_OR_DIE("ScreenBufferInfo", GetConsoleScreenBufferInfo(screen->out, &csbi ))
   	   
   dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
   printf("%d %d", csbi.dwSize.X, csbi.dwSize.Y);
   
   DO_OR_DIE("Fill the entire screen with blanks", FillConsoleOutputCharacter(screen->out,        // Handle to console screen buffer 
                                                           (TCHAR) ' ',     // Character to write to the buffer
                                                           dwConSize,       // Number of cells to write 
                                                           coordScreen,     // Coordinates of first cell 
                                                           &cCharsWritten))// Receive number of characters written
      
   DO_OR_DIE("Get the current text attribute.", GetConsoleScreenBufferInfo(screen->out, &csbi))
   

   // Set the buffer's attributes accordingly.
   DO_OR_DIE("Set the buffer's attributes accordingly.",
		     FillConsoleOutputAttribute(screen->out,      // Handle to console screen buffer 
                                        csbi.wAttributes, // Character attributes to use
                                        dwConSize,        // Number of cells to set attribute 
                                        coordScreen,      // Coordinates of first cell 
                                        &cCharsWritten )) // Receive number of characters written
   // Put the cursor at its home coordinates.
   SetConsoleCursorPosition(screen->out, coordScreen);
}

DWORD check_console(Console* screen) {
	DWORD mode;
	GetConsoleMode(screen->in, &mode);
#ifdef DEBUG
	printf("Console mode: %lu\n", mode);
	printf("ENABLE_ECHO_INPUT: %d\n", (mode & ENABLE_ECHO_INPUT) != 0);
	printf("ENABLE_INSERT_MODE: %d\n", (mode & ENABLE_INSERT_MODE) != 0);
	printf("ENABLE_LINE_INPUT : %d\n", (mode & ENABLE_LINE_INPUT) != 0);

	printf("ENABLE_MOUSE_INPUT  : %d\n", (mode & ENABLE_MOUSE_INPUT) != 0);
	printf("ENABLE_PROCESSED_INPUT  : %d\n", (mode & ENABLE_PROCESSED_INPUT) != 0);
	printf("ENABLE_QUICK_EDIT_MODE  : %d\n", (mode & ENABLE_QUICK_EDIT_MODE) != 0);
	
	printf("ENABLE_WINDOW_INPUT  : %d\n", (mode & ENABLE_WINDOW_INPUT) != 0);
	printf("ENABLE_VIRTUAL_TERMINAL_INPUT  : %d\n", (mode & ENABLE_VIRTUAL_TERMINAL_INPUT) != 0);
	
	printf("ENABLE_PROCESSED_OUTPUT : %d\n", (mode & ENABLE_PROCESSED_OUTPUT) != 0);
	printf("ENABLE_WRAP_AT_EOL_OUTPUT  : %d\n", (mode & ENABLE_WRAP_AT_EOL_OUTPUT) != 0);
	printf("ENABLE_VIRTUAL_TERMINAL_PROCESSING  : %d\n", (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0);
	printf("DISABLE_NEWLINE_AUTO_RETURN  : %d\n", (mode & DISABLE_NEWLINE_AUTO_RETURN) != 0);
	printf("ENABLE_LVB_GRID_WORLDWIDE   : %d\n", (mode & ENABLE_LVB_GRID_WORLDWIDE) != 0);
#endif
	return mode;
}

void console_init(Console* screen) {
	screen->out = GetStdHandle(STD_OUTPUT_HANDLE);
	screen->in = GetStdHandle(STD_INPUT_HANDLE);
}

void console_goto_xy(Console*screen, int x, int y) {
	COORD new_position = { x, y };
	SetConsoleCursorPosition(screen->out, new_position);
}

void console_set_color(Console* screen, int color) {
	SetConsoleTextAttribute(screen->out, color);
}

void console_set_foreground(Console* screen, int color) {
	CONSOLE_SCREEN_BUFFER_INFO cinfo;
	GetConsoleScreenBufferInfo(screen->out, &cinfo);	
	console_set_color(screen, color | (cinfo.wAttributes & 0xfff0));
}

void console_set_background(Console* screen, int color) {
	CONSOLE_SCREEN_BUFFER_INFO cinfo;
	GetConsoleScreenBufferInfo(screen->out, &cinfo);	
	console_set_color(screen, color << 4 | (cinfo.wAttributes & 0xff0f));
}


void console_pause(Console* screen) {
	DWORD mode, iread;
	INPUT_RECORD input;
	GetConsoleMode(screen->in, &mode);	
    SetConsoleMode(screen->in, mode & ~ENABLE_ECHO_INPUT);
	ReadConsoleInput(screen->in, &input, 1, &iread);	
	SetConsoleMode(screen->in, mode);
}

void console_set_title(TCHAR* title) {
	SetConsoleTitle(title);
}

int main(int argc, char **argv)
{
	Console screen;
	console_init(&screen);
	console_set_title("ConsoleUtils");
	printf("Compiled on %s %s\n", __DATE__, __TIME__);
	if (check_console(&screen) == 0)
	{
		perror("This version does not support non native windows consoles.\n");
		return 1;
	}
	
	console_clear(&screen);
		
	for (int c = 0; c < 16; c++) {
		console_set_color(&screen, c);
		printf("Color %d\n", c);		
	}
	console_set_foreground(&screen, CONSOLE_RED);
	printf("RED\n");
	console_set_background(&screen, CONSOLE_YELLOW);
	printf("YELLOW\n");
	console_set_foreground(&screen, CONSOLE_BRIGHT_RED);
	printf("RED\n");
	console_set_background(&screen, CONSOLE_BRIGHT_YELLOW);
	printf("YELLOW 2\n");
	console_goto_xy(&screen, 20, 10);
	printf("X");
	console_goto_xy(&screen, 25, 5);
	printf("Y");
	console_goto_xy(&screen, 0, 20);
	console_pause(&screen);
	return 0;
}