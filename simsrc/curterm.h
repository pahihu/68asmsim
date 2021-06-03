#ifndef _CURTERM_H
#define _CURTERM_H

void PrepTerm(int);
int  HasKey(void);
int  GetKey(void);
void KbdFlush(void);
int  HasCtrlC(void);
void CBreak(int enable);

void GotoXY(int x, int y);
void ClrScr(void);

#endif
