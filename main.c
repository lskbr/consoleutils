#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>

#define FATAL(message)  ErrorExit(__FILE__, __func__, __LINE__, message)
#define DO_OR_DIE(m, z) if(!z) {FATAL(m);}

void print_error(TCHAR* message) {
	HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
	DWORD written = 0;
	DWORD size = (DWORD)strlen(message);
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
void cls(HANDLE hConsole)
{
   COORD coordScreen = { 0, 0 };    // home for the cursor 
   DWORD cCharsWritten;
   CONSOLE_SCREEN_BUFFER_INFO csbi; 
   DWORD dwConSize;
   // Get the number of character cells in the current buffer. 
   
   DO_OR_DIE("ScreenBufferInfo", GetConsoleScreenBufferInfo( hConsole, &csbi ))
   	   
   dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
   printf("%d %d", csbi.dwSize.X, csbi.dwSize.Y);
   
   DO_OR_DIE("Fill the entire screen with blanks", FillConsoleOutputCharacter( hConsole,        // Handle to console screen buffer 
                                                           (TCHAR) ' ',     // Character to write to the buffer
                                                           dwConSize,       // Number of cells to write 
                                                           coordScreen,     // Coordinates of first cell 
                                                           &cCharsWritten))// Receive number of characters written
      
   DO_OR_DIE("Get the current text attribute.", GetConsoleScreenBufferInfo(hConsole, &csbi))
   

   // Set the buffer's attributes accordingly.
   DO_OR_DIE("Set the buffer's attributes accordingly.",
		     FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer 
                                        csbi.wAttributes, // Character attributes to use
                                        dwConSize,        // Number of cells to set attribute 
                                        coordScreen,      // Coordinates of first cell 
                                        &cCharsWritten )) // Receive number of characters written
   // Put the cursor at its home coordinates.
   SetConsoleCursorPosition(hConsole, coordScreen);
}

void check_console(HANDLE h) {
	DWORD mode;
	GetConsoleMode(h, &mode);
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
	

}

int main(int argc, char **argv)
{
	SetConsoleTitle("Hello Console");
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	cls(hStdout);
	check_console(hStdout);
	
	print_error("Hello!");

	return 0;
}