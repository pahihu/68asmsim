
/***************************** 68000 SIMULATOR ****************************

File Name: IOBOX.C
Version: 1.0

This file contains system-dependent calls to manipulate the screen.

***************************************************************************/



#include <stdio.h>
#ifdef MSDOS
#include <conio.h>
#include <dos.h>
union REGS regs;
#else
#include "curterm.h"
#endif
#include "extern.h"


int	row, col;

#ifndef MSDOS
#include <unistd.h>
#include <string.h>

void vt100(char *seq)
{
   char buf[16];

   fflush(stdout);
   sprintf(buf,"\033%s",seq);
   write(1,buf,strlen(buf));
}
#endif


int at(y,x)    /* position cursor at line y (y=1..25), column x (x=1..80) */
int x,y;
{
#ifdef MSDOS
	regs.h.ah = 2;                /* function 2 is cursor position function */
	regs.h.bh = 0;
	regs.h.dh = (y - 1) & 0xff;
	regs.h.dl = (x - 1) & 0xff;
	int86 (0x10, &regs, &regs);
	return (regs.x.cflag);
#else
   GotoXY(x, y);
   return SUCCESS;
#endif
}



int home()
{

	at(1,1);
	return SUCCESS;

}



int	clrscr()            /* clear screen by using bios scroll up function */
{
#ifdef MSDOS
	regs.h.ah = 6;			/* function 6 is scroll-up */
	regs.h.al = 0;			/* entire screen */
	regs.h.bh = 0x2007;	/* use attribute 0x2007 */
	regs.h.bl = 0;
	regs.x.cx = 0;
	regs.h.dh = 24;		/* number of rows - 1 */
	regs.h.dl = 79;		/* number of columns - 1 */
	int86 (0x10, &regs, &regs);

	return (regs.x.cflag);
#else
   ClrScr();
   return SUCCESS;
#endif

}


char chk_buf()                /* read a character from the keyboard buffer */
                                       /* return '\0' if nothing available */
{
#ifdef MSDOS
	if (kbhit())
		return (bdos(0x07) & 0x00ff);
	else
		return ('\0');
#else
   if (HasKey())
      return GetKey();
   else
      return ('\0');
#endif

}


/* function windowLine() gives a newline within the scrollable window */

void	windowLine()
{
#ifdef MSDOS
	regs.h.ah = 3;							/* function 3 = read cursor position */
	regs.h.bh = 0;							/* page 0 */
	int86 (0x10, &regs, &regs);
	if (regs.h.dh < 23) {
		printf ("\n");						/* regular newline */
		return ;
		}

	regs.h.ah = 6;							/* function 6 = scroll screen */
	regs.h.al = 1;							/* scroll 1 line */
	regs.h.bh = 0x07;						/* new line display attribute */
	regs.h.ch = 8;							/* start at line 8 */
	regs.h.cl = 0;							/* "     "  column 0 */
	regs.h.dh = 24;						/* end at line 24 */
	regs.h.dl = 79;						/* "   "  column 79 */

	int86 (0x10, &regs, &regs);		/* call video services BIOS function */

	regs.h.ah = 2;							/* set cursor position */
	regs.h.bh = 0;							/* page 0 */
	regs.h.dh = 23;						/* row 24 */
	regs.h.dl = 0;							/* column 0 */

	int86 (0x10, &regs, &regs);		/* call video services BIOS function */
#else
   vt100("E");
#endif

}


/* function scrollWindow() scrolls the scrolling window up if the text has */
/* moved down to line 25.  This function is called after the user enters */
/* information within the scrolling portion of the screen */

void	scrollWindow()
{
#ifdef MSDOS
	regs.h.ah = 3;							/* function 3 = read cursor position */
	regs.h.bh = 0;							/* page 0 */
	int86 (0x10, &regs, &regs);
	if (regs.h.dh >= 24) {
		regs.h.ah = 6;							/* function 6 = scroll screen */
		regs.h.al = 1;							/* scroll 1 line */
		regs.h.bh = 0x07;						/* new line display attribute */
		regs.h.ch = 8;							/* start at line 8 */
		regs.h.cl = 0;							/* "     "  column 0 */
		regs.h.dh = 24;						/* end at line 24 */
		regs.h.dl = 79;						/* "   "  column 79 */

		int86 (0x10, &regs, &regs);		/* call video services BIOS function */

		regs.h.ah = 2;							/* set cursor position */
		regs.h.bh = 0;							/* page 0 */
		regs.h.dh = 23;						/* row 24 */
		regs.h.dl = 0;							/* column 0 */

		int86 (0x10, &regs, &regs);		/* call video services BIOS function */
		}
#else
   vt100("D");
#endif

}


void	save_cursor()
{
#ifdef MSDOS
	regs.h.ah = 3;							/* function 3 = read cursor position */
	regs.h.bh = 0;							/* page 0 */
	int86 (0x10, &regs, &regs);
	row = regs.h.dh;
	col = regs.h.dl;
#else
   vt100("7");
#endif

}


void	restore_cursor()
{
#ifdef MSDOS
	regs.h.ah = 2;							/* set cursor position */
	regs.h.bh = 0;							/* page 0 */
	regs.h.dh = row;						/* row */
	regs.h.dl = col;						/* column */

	int86 (0x10, &regs, &regs);		/* call video services BIOS function */
#else
   vt100("8");
#endif

}

char *GetS(str, len)
char *str;
int len;
{
   char *ret;

   PrepTerm(0);
	ret = fgets(str, len, stdin);
   PrepTerm(1);
   vt100("[1A");
   return ret;
}

