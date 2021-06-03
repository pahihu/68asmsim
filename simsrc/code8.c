
/***************************** 68000 SIMULATOR ****************************

File Name: CODE8.C
Version: 1.0

The instructions implemented in this file are the program control operations:

		BCC, DBCC, SCC, BRA, BSR, JMP, JSR, RTE, RTR, RTS, NOP


***************************************************************************/


#include <stdio.h>
#include "extern.h"         /* contains global "extern" declarations */





int	BCC()
{
Long	displacement;
int	condition;

displacement = inst & 0xff;
if (displacement == 0)
	{
	mem_request (&PC, (Long) WORD, &displacement);
	from_2s_comp (displacement, (Long) WORD, &displacement);
	}
else
	from_2s_comp (displacement, (Long) BYTE, &displacement);

condition = (inst >> 8) & 0x0f;

/* perform the BCC operation */
if (check_condition (condition))
	PC = OLD_PC + displacement + 2;		
			/* displacement is relative to the end of the instructin word */

if (check_condition (condition))
	inc_cyc (10);
else
	inc_cyc (((inst & 0xff) != 0) ? 12 : 8);

return SUCCESS;

}




int	DBCC()
{
Long	displacement;
int	reg;

mem_request(&PC, (Long) WORD, &displacement);
from_2s_comp (displacement, (Long) WORD, &displacement);
reg = inst & 0x07;

/* perform the DBCC operation */
if (check_condition ((inst >> 8) & 0x0f))
	inc_cyc (12);
else
	{
	D[reg] = (D[reg] & ~WORD) | ((short)((D[reg] & 0xffff) - 1));
	if ((D[reg] & 0xffff) == -1)
		inc_cyc (14);
	else
		{
		inc_cyc (10);
		PC = OLD_PC + displacement + 2;
			/* displacement is relative to the end of the instructin word */
		}
	}

return SUCCESS;

}


int	SCC()
{
int	condition;

if (eff_addr ((Long) BYTE, DATA_ALT_ADDR, TRUE))
	return (BAD_INST);		/* bad instruction format */

/* perform the SCC operation */
condition = (inst >> 8) & 0x0f;
if (check_condition (condition))
	put (EA1, (Long) BYTE, (Long) BYTE);
else
	put (EA1, (Long) 0, (Long) BYTE);

if ((inst & 0x0030) == 0)
	inc_cyc (check_condition (condition) ? 6 : 4);
else
	inc_cyc (8);

return SUCCESS;

}



int	BRA()
{
Long	displacement;

displacement = inst & 0xff;
if (displacement == 0) 
	{
	mem_request (&PC, (Long) WORD, &displacement);
	from_2s_comp (displacement, (Long) WORD, &displacement);
	}
else
	from_2s_comp (displacement, (Long) BYTE, &displacement);

/* perform the BRA operation */
PC = OLD_PC + displacement + 2;
			/* displacement is relative to the end of the instructin word */

inc_cyc (10);

return SUCCESS;

}



int	BSR()
{
Long	displacement;

displacement = inst & 0xff;
if (displacement == 0) 
	{
	mem_request (&PC, (Long) WORD, &displacement);
	from_2s_comp (displacement, (Long) WORD, &displacement);
	}
else
	from_2s_comp (displacement, (Long) BYTE, &displacement);

/* perform the BSR operation */
A[a_reg(7)] -= 4;
put (&memory[A[a_reg(7)]], PC, LONG);
PC = OLD_PC + displacement + 2;
			/* displacement is relative to the end of the instructin word */

inc_cyc (18);

return SUCCESS;

}



int	JMP()
{

if (eff_addr ((Long) WORD, CONTROL_ADDR, FALSE))
	return (BAD_INST);		/* bad instruction format */

/* perform the JMP operation */
PC = (int) ((long)EA1 - (long)&memory[0]);

switch (eff_addr_code (inst, 0)) {
	case 0x02 : inc_cyc (8);
	            break;
	case 0x05 : inc_cyc (10);
	            break;
	case 0x06 : inc_cyc (14);
	            break;
	case 0x07 : inc_cyc (10);
	            break;
	case 0x08 : inc_cyc (12);
	            break;
	case 0x09 : inc_cyc (10);
	            break;
	case 0x0a : inc_cyc (14);
	            break;
	default   : break;
	}

return SUCCESS;

}





int	JSR()
{

if (eff_addr ((Long) WORD, CONTROL_ADDR, FALSE))
	return (BAD_INST);		/* bad instruction format */

/* push the longword address immediately following PC on the system stack */
/* then change the PC */
A[a_reg(7)] -= 4;
put (&memory[A[a_reg(7)]], PC, LONG);
PC = (int) ((long)EA1 - (long)&memory[0]);

switch (eff_addr_code (inst, 0)) {
	case 0x02 : inc_cyc (16);
	            break;
	case 0x05 : inc_cyc (18);
	            break;
	case 0x06 : inc_cyc (22);
	            break;
	case 0x07 : inc_cyc (18);
	            break;
	case 0x08 : inc_cyc (20);
	            break;
	case 0x09 : inc_cyc (18);
	            break;
	case 0x0a : inc_cyc (22);
	            break;
	default   : break;
	}

return SUCCESS;

}


int	RTE()
{
Long	temp;

if (!(SR & sbit))
	return (NO_PRIVILEGE);

mem_request (&A[8], (Long) WORD, &temp);
SR = temp & WORD;
mem_request (&A[8], LONG, &PC);

inc_cyc (20);

return SUCCESS;

}



int	RTR()
{
Long	temp;

mem_request (&A[a_reg(7)], (Long) BYTE, &temp);
SR = (SR & 0xff00) | (temp & 0xff);

mem_request (&A[a_reg(7)], LONG, &PC);

inc_cyc (20);

return SUCCESS;

}



int	RTS()
{

mem_request (&A[a_reg(7)], LONG, &PC);

inc_cyc (16);

return SUCCESS;

}



int	NOP()
{

inc_cyc (4);

return SUCCESS;

}

