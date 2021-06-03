
/***************************** 68000 SIMULATOR ****************************

File Name: SIM.C
Version: 1.0

This file contains the function 'main()' and various screen management
	routines.

***************************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include "var.h"         /* include declarations for global variables */
#include "extern.h"
#include "curterm.h"


int main()            /* main routine gives command prompt, */
{						/* scans instructions, and directs user interface */
int count, refresh;

PrepTerm(1);
printf ("68000 Simulator\n");
printf ("Version 1.0\n");
init();
while (!same("exit",wordptr[0]))	{
	refresh = TRUE;
	windowLine();
	printf("> ");			/* give prompt */
	if (GetS(lbuf,80) == NULL) 
		exit(0);
	scrollWindow();
	count = scan(lbuf,wordptr,10);	      /* scan command and operands */
	wcount = count;
	if (count == 0)
		refresh = FALSE;					/* if no command entered do nothing */
	else if (same("trace",wordptr[0])) trace = TRUE;
	else if (same("troff",wordptr[0])) trace = FALSE;
	else if (same("ex_on",wordptr[0])) {
		exceptions = TRUE;
		refresh = FALSE;
		}
	 else if (same("ex_off",wordptr[0])) {
		 exceptions = FALSE;
		 refresh = FALSE;
		 }
	 else if (same("version",wordptr[0])) {
		 printf ("68000 Simulator Version %d.%d",
			 MAJOR_VERSION, MINOR_VERSION);
		 windowLine();
		 refresh = FALSE;
		 }
	 else if (same("sstep",wordptr[0])) sstep = TRUE;
	 else if (same("ssoff",wordptr[0])) sstep = FALSE;
	 else if (same("s_on",wordptr[0])) SR |= sbit;
	 else if (same("t_on",wordptr[0])) SR |= tbit;
	 else if (same("x_on",wordptr[0])) SR |= xbit;
	 else if (same("n_on",wordptr[0])) SR |= nbit;
	 else if (same("z_on",wordptr[0])) SR |= zbit;
	 else if (same("v_on",wordptr[0])) SR |= vbit;
	 else if (same("c_on",wordptr[0])) SR |= cbit;
	 else if (same("s_off",wordptr[0])) SR &= ~sbit;
	 else if (same("t_off",wordptr[0])) SR &= ~tbit;
	 else if (same("x_off",wordptr[0])) SR &= ~xbit;
	 else if (same("n_off",wordptr[0])) SR &= ~nbit;
	 else if (same("z_off",wordptr[0])) SR &= ~zbit;
	 else if (same("v_off",wordptr[0])) SR &= ~vbit;
	 else if (same("c_off",wordptr[0])) SR &= ~cbit;
	 else if (same("bp",wordptr[0])) {
		 selbp();
		 refresh = FALSE;
		 }
	 else if (same("help",wordptr[0])) {
			gethelp();
			clrscr();
			home();
			setdis();
			scrshow();
			at (8,1);
			refresh = FALSE;
			}
	 else if (same("clear",wordptr[0])) clear();
	 else if (same("ch",wordptr[0])) alter();
	 else if (same("exit",wordptr[0])) finish();
	 else if (same("refresh",wordptr[0])) { }
	 else if (same("dec",wordptr[0])) refresh = FALSE;
	 else if (same("hex",wordptr[0])) refresh = FALSE;
	 else if (same("go",wordptr[0])) {}
	 else if (same("md",wordptr[0])) {}
	 else if (same("ld",wordptr[0])) {}
	 else {
		 errmess();
		 refresh = FALSE;
		 }
	if (same("md",wordptr[0])) mdis();
	else if (same("ld",wordptr[0])) mfill();
	else if (same("dec",wordptr[0])) hex_to_dec();
	else if (same("hex",wordptr[0])) dec_to_hex();
	else if (same("go",wordptr[0])) runprog();
	if (refresh) {
		save_cursor();
		home();
		setdis();
		scrshow();
		restore_cursor();
		}
	}
   return 0;
}



void init()                     /* initialization for the simulator */
{
	int	i;

	exceptions = FALSE;
	lbuf[0] = '\0';		/* initialize to prevent memory access violations */
	wordptr[0] = lbuf;
	for (i = 0; i <= 7; i++)
		A[i] = D[i] = OLD_A[i] = OLD_D[i] = 0;
	OLD_A[8] = cycles = PC = 0;
	SR = 0x2000;
	A[8] = 0xf00;
	OLD_PC = old_cycles = OLD_SR = -1; /* set different from 'PC' and 'cycles' */
	trace = sstep = FALSE;
	old_trace = old_sstep = TRUE;	  /* set different from 'trace' and 'sstep' */
	clrscr();
	home();
	setdis();
	scrshow();
	at (8,1);
}


void finish()               /* normal simulator exit */
{

	at (24,1);
	printf("end 68000 simulator . . .");
	exit(0);

}



void errmess()		/* error message for invalid input */
{

	printf("invalid input...");
	windowLine();

}



void cmderr()					/* error message for an invalid command */
{

	printf ("invalid command");
	windowLine();
	printf ("type help for list of commands");
	windowLine();

}



void setdis()                          /* shows register display */
{

	printf("<D0>  = %08x  <D4>  = %08x  <A0>  = %08x  <A4> = %08x\n",(int)D[0],
		(int)D[4],(int)A[0],(int)A[4]);
	printf("<D1>  = %08x  <D5>  = %08x  <A1>  = %08x  <A5> = %08x\n",(int)D[1],
		(int)D[5],(int)A[1],(int)A[5]);
	printf("<D2>  = %08x  <D6>  = %08x  <A2>  = %08x  <A6> = %08x\n",(int)D[2],
		(int)D[6],(int)A[2],(int)A[6]);
	printf("<D3>  = %08x  <D7>  = %08x  <A3>  = %08x  <A7> = %08x\n",(int)D[3],
		(int)D[7],(int)A[3],(int)A[7]);
	printf("trace:            sstep:            cycles: %6u    <A7'>= %08x\n"
		,cycles, (int)A[8]);
	printf("         cn  tr  st  rc           T S  INT   XNZVC    <PC> = %08x\n"
		,(int)PC);
	printf("  port1  %02x  %02x  %02x  %02x      SR =\n",
           port1[0],port1[1],port1[2],port1[3]);
	printf("=======================================================");
	printf("========================\n");
	old_sstep = ~sstep;
	old_trace = ~trace;
	OLD_SR = ~SR;

}


void scrshow()					/* display processor registers and cycle counter */
{
	int	j,k;

	k = 9;
	for (j=0;j<=7;j++)		/* update D registers */
		{
		if (D[j] != OLD_D[j])
			{
			OLD_D[j] = D[j];
			if (j >= 4) k = 27; 		/* select the proper column for display */
			at(j%4+1,k);
			printf("%08x",(int)D[j]);
			}
		}

	k = 45;
	for (j=0;j<=8;j++)		/* update A registers */
		{
		if (A[j] != OLD_A[j])
			{
			OLD_A[j] = A[j];
			if (j >= 4) k = 62; 		/* select the proper column for display */
			at(j%4+1,k);
			if (j == 8) at (5,k);
			printf("%08x",(int)A[j]);
			}
		}
	
	/* if port 1 has been written to then update its display */
	if (p1dif)
		{
		at(7,10);
		printf("%02x  %02x  %02x  %02x\n",port1[0],port1[1],port1[2],port1[3]);
		p1dif = FALSE;
		}

	/* update the status register's value */
	if (SR != OLD_SR)
		{
		OLD_SR = SR;
		at(7,35);
		for (j=15;j>=0;j--)
			{
			if ((0x1 << j) & SR)	/* print each bit of SR */
				printf ("1");
			else	printf ("0");
			}
		}

	if (OLD_PC ^ PC)
		{
		OLD_PC = PC;
		at(6, 62);
		printf ("%08x",(int)PC);
		}

	if (sstep ^ old_sstep)
		{
		old_sstep = sstep;
		at (5, 26);
		if (sstep)
			printf ("on ");
		else
			printf ("off");
		}

	if (trace ^ old_trace)
		{
		old_trace = trace;
		at(5,8);
		if (trace)
			printf ("on ");
		else
			printf ("off");
		}

	if (old_cycles ^ cycles)
		{
		old_cycles = cycles;
		at(5,45);
		printf("%6u",cycles);
		}

}

