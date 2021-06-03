#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "curterm.h"

static int ctrl_c_event;
static int prepared = 0;

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <conio.h>

static HANDLE hStdin;
static HANDLE hStdout;

void GotoXY(int x, int y)
{
	COORD xy;

   fflush(stdout);
	xy.X = x;
	xy.Y = y;
	SetConsoleCursorPosition(hStdout, xy);
}

void ClrScr(void)
{
	CONSOLE_SCREEN_BUFFER_INFO scr;
	COORD ul = {0, 0};
	DWORD n, size;

	GetConsoleScreenBufferInfo(hStdout, &scr);
	size = scr.dwSize.X * scr.dwSize.Y;
	FillConsoleOutputCharacter(hStdout, ' ', size, ul, &n);
	FillConsoleOutputAttribute(hStdout,   0, size, ul, &n);
	SetConsoleCursorPosition(hStdout, ul);
}

BOOL CtrlC(DWORD ct)
{
	if (CTRL_C_EVENT == ct) {
		ctrl_c_event = 1;
		return TRUE;
	}
	return FALSE;
}

void initterm()
{
	static int _init = 0;

	if (!_init) {
		hStdin  = GetStdHandle(STD_INPUT_HANDLE);
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		_init = 1;
	}
}

void CBreak(int enable)
{
	initterm();

	ctrl_c_event = 0;
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlC, enable ? FALSE : TRUE)) 
	{
		printf("failed to install Ctrl-C handler\n");
		exit(1);
	}

}

void PrepTerm(int dir)
{
	static DWORD told, tnew;

	initterm();

	if (dir) {
		if (prepared == 0) {
			GetConsoleMode(hStdin, &told);
			tnew  = told;
			tnew &= ~ENABLE_PROCESSED_INPUT;
			SetConsoleMode(hStdin, tnew);
			prepared = 1;
		}
	}
	else if (prepared) {
		SetConsoleMode(hStdin, told);
		prepared = 0;
	}

	return;
}

int HasKey()
{
   fflush(stdout);
	return kbhit();
}

int GetKey(void)
{
	int ch;

   fflush(stdout);
	ch = getch();
	if (0 == ch)
		ch = 0xF0;
	if (0xF0 == ch || 0xE0 == ch)
		ch = (ch << 8) + getch();

	return ch;
}

#else

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>

static
void deprepterm(void)
{
   PrepTerm(0);
}

void PrepTerm(int dir)
{
	static struct termios told, tnew;
   static int installed = 0;

   if (!isatty(STDIN_FILENO))
      return;

	if (dir) {
		if (prepared == 0) {
			tcgetattr(STDIN_FILENO, &told);
			tnew = told;
	                tnew.c_iflag &= ~ICRNL;
			tnew.c_lflag &= ~(ICANON | ECHO );
			tcsetattr(STDIN_FILENO, TCSANOW | TCSAFLUSH, &tnew);
			prepared = 1;
         if (!installed) {
            atexit(deprepterm);
            installed = 1;
         }
		}
	} else if (prepared) {
		tcsetattr(STDIN_FILENO, TCSANOW | TCSAFLUSH, &told);
		prepared = 0;
   }
}

int has_key_timeout(int timeout)
{
	struct timeval tv;
	fd_set rd;
   int ret;

	tv.tv_sec  = 0;
	tv.tv_usec = timeout;

	FD_ZERO(&rd);
	FD_SET(STDIN_FILENO, &rd);

	ret = select(STDIN_FILENO + 1, &rd, NULL, NULL, &tv);
   if (0 == ret)  /* timeout */
      return 0;
   if (-1 == ret) /* error */
      return 0;
	return FD_ISSET(STDIN_FILENO, &rd);
}

int HasKey(void)
{
   fflush(stdout);
   return has_key_timeout(0);
}

#define ESC          '\033'
#define ESC_TIMEOUT  25000

/*
 * NB. using getchar() returns the ESC, but checking for the ESC-sequence
 *     chars doesn't work. So we read the raw chars, but needs to flush
 *     previously emitted chars.
 */
static
unsigned char
get_char(void)
{
   char buf;

   fflush(stdout);
   (void) read(STDIN_FILENO, &buf, sizeof(char));
   return (unsigned char)buf;
}

int GetKey(void)
{
	int ch;

   fflush(stdout);
	ch = get_char();
	if (ESC == ch) {
      if (!has_key_timeout(ESC_TIMEOUT))
         return ESC;
		ch = (unsigned char)get_char();
		ch = (ch << 8) + (unsigned char)get_char();
        if (HasKey())
            ch = (ch << 8) + (unsigned char)get_char();
	}

	return ch;
}

void GotoXY(int x, int y)
{
	char buf[16];

   fflush(stdout);
	sprintf(buf, "%c[%d;%dH", ESC, y, x);
	(void) write(1, buf, strlen(buf));
}

void ClrScr(void)
{
	char buf[16];

	sprintf(buf, "%c[2J", ESC);
	(void) write(1, buf, strlen(buf));
	GotoXY(0, 0);
}

void ctrlc_handler(int sig)
{
	ctrl_c_event = 1;
}

void CBreak(int enable)
{
	if (enable)
		signal(SIGINT, SIG_DFL);
	else
		if (SIG_ERR == signal(SIGINT, ctrlc_handler)) {
			printf("failed to install Ctrl-C handler\n");
			exit(1);
		}
}

#endif

#ifdef MAIN
int main(int argc, char *argv[])
{
	int i, c;

	c = 0;
	PrepTerm(1);
   ClrScr();
   for (i = 1; i < 10; i++)
      printf("%d\n", i);
   GotoXY(10, 6); printf("hello");
	while (c != 'q') {
		if (HasKey()) {
         GotoXY(35, 7);
			printf("%d ", (c = GetKey()));
			if (27 == c) {
				printf("%d ", (c = GetKey()));
				printf("%d ", (c = GetKey()));
			}
			printf("\n");
		}
	}
	PrepTerm(0);
	return 0;
}
#endif

void KbdFlush(void)
{
	while (HasKey())
		GetKey();
}

int HasCtrlC(void)
{
	if (ctrl_c_event) {
		ctrl_c_event = 0;
		return 1;
	}
	return 0;
}
