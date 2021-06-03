/*
   D68K - 68K disassembler

   (c) 2021, Andras Pahi
   
   History
   =======
   210603AP hex/mnemo display separated
   210602AP integrated into sim68k, *.asm examples working, MOVEM issues
   210601AP initial revision

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char hexbuf[128];
static char mnemobuf[128];

static
void X(const char *fmt,...) {
   va_list ap;
   char *ptr;

   ptr = &hexbuf[0] + strlen(hexbuf);
   va_start(ap,fmt);
   vsprintf(ptr,fmt,ap);
   va_end(ap);
}

static
void O(const char *fmt,...) {
   va_list ap;
   char *ptr;

   ptr = &mnemobuf[0] + strlen(mnemobuf);
   va_start(ap,fmt);
   vsprintf(ptr,fmt,ap);
   va_end(ap);
}

typedef int addr_t;

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef int long_t;

#ifdef STANDALONE
static byte_t *Mem;
#endif
static addr_t pc;

static
void giveup(void) {
   exit(1);
}

static
void illegal(addr_t pc, word_t op) {
   O("%08X: %04X illegal op",pc,op);giveup();
}

static
long_t sext16(word_t u) {
   long_t ret;

   ret = u;
   if (u > 32767) ret -= 65536;
   return ret;
}

static
long_t sext8(byte_t u) {
   long_t ret;

   ret = u;
   if (u > 127) ret -= 256;
   return ret;
}

static
word_t readWord(addr_t *padr) {
   word_t ret = 0;

#ifdef STANDALONE
   addr_t adr;

   adr = *padr;
   ret = Mem[adr++];
   ret = (ret << 8) + Mem[adr++];
   *padr = adr;
#else
   extern int mem_request(int*,long,long*);
   long lret;
   mem_request(padr, 0xffff, &lret);
   ret = lret;
#endif

   X("%04X ",ret);
   
   return ret;
}

static
long_t readLong(addr_t *padr) {
   long_t ret;

   ret = readWord(padr);
   ret = (ret << 16) + readWord(padr);

   return ret;
}

static
long_t readS(addr_t *padr, int s) {
   long_t ret;

   ret = 0xDEADBEEF;
   switch(s) {
   case 0: ret=readWord(padr) & 0xFF; break;
   case 1: ret=readWord(padr); break;
   case 2: ret=readLong(padr); break;
   default: O("readS: invalid s=%d",s);giveup();
   }
   return ret;
}

static long_t Xdisp;
static int Xs, XR, XM;

static
void BriefExtWord() {
   word_t w;

   w = readWord(&pc);
   Xdisp = w & 0xFF;     w >>= 8;
   // 68020 <scale>0, <scale>1 - base-disp
   if (w & 7) { O("BriefExtWord: invalid scale/sel=%d",w&7); giveup(); }
   w >>= 3;
   Xs = w & 1;           w >>= 1;
   XR = w & 7;           w >>= 3;
   XM = w; // 0 - D, 1 - A
}

static int OS;

static
void AdrMode(int m,int xn) {
   switch (m) {
   case 0: O("D%d",xn);break;
   case 1: O("A%d",xn);break;
   case 2: O("(A%d)",xn);break;
   case 3: O("(A%d)+",xn);break;
   case 4: O("-(A%d)",xn);break;
   case 5: 
      Xdisp = readWord(&pc);
      O("(%d,A%d)",Xdisp,xn);
      break;
   case 6:
      BriefExtWord();
      O("(%d,A%d,%c%d.%c)",Xdisp,xn,XM? 'A':'D',XR,Xs? 'L':'W');
      break;
   case 7:
      switch(xn){
      case 2:
         Xdisp = readWord(&pc);
         O("(%d,PC)",Xdisp);
         break;
      case 3:
         BriefExtWord();
         O("(%d,PC,%c%d.%c)",Xdisp,XM? 'A':'D',XR,Xs? 'L':'W');
         break;
      case 0: 
         Xdisp = readWord(&pc);
         O("($%X).W",Xdisp);
         break;
      case 1:
         Xdisp = readLong(&pc);
         O("($%X).L",Xdisp);
         break;
      case 4:
         Xdisp = readS(&pc,OS); // OS set by OpSizeX()
         O("#$%X",Xdisp);
         break;
      default: O("AdrModeM: m=%d xn=%d",m,xn);giveup();
      }
   }
}

static
void OpSize0(int s) {
  switch(s) {
  case 0: O(".B ");break;
  case 1: O(".W ");break;
  case 2: O(".L ");break;
  default: O("OpSize0: invalid s=%d",s);giveup();
  }
  OS=s;
}

static
void OpSize1(int s) {
  O(0==s? ".W " : ".L ");
  OS=1+s;
}

static
void OpSize2(int s) {
  switch(s) {
  case 1: O(".B ");OS=0;break;
  case 3: O(".W ");OS=1;break;
  case 2: O(".L ");OS=2;break;
  default: O("OpSize2: s=%d",s);giveup();
  }
}

static
void GreenDir(int d) {
  O(0==d? "R" : "L");
}

static
void RedMode(int m,int n) {
  if(0==m) O("D%d",n);
  else O("-(A%d)",n);
}

static
void Cond(int cc) {
   static char* mnemo[] = {
      "T ", "F ", "HI", "LS",
      "CC", "CS", "NE", "EQ",
      "VC", "VS", "PL", "MI",
      "GE", "LT", "GT", "LE"
   };
   O("%s ", mnemo[cc & 15]);
}

static char *M2R = "A7A6A5A4A3A2A1A0D7D6D5D4D3D2D1D0";
static char *R2M  = "D0D1D2D3D4D5D6D7A0A1A2A3A4A5A6A7";

static
void RegMask(char* ord,word_t mask) {
   char AD;
   int first;

   AD = ord[0]; first = 1;
   while(mask) {
      if (mask&0x8000) {
         if (ord[0] != AD) {
            O("/"); AD = ord[0];
            first=1;
         }
         if (!first) O(",");
         O("%c%c",ord[0],ord[1]);
         first = 0;
      }
      mask <<= 1; ord += 2;
   }
}

static int R, M, S, s, r, L;

//           1
// 0123456789012345
// LLLLrrrsSSMMMRRR
static
void decode(word_t w) {
   R = w &  7; w >>= 3;
   M = w &  7; w >>= 3;
   S = w &  3; w >>= 2;
   s = w &  1; w >>= 1;
   r = w &  7; w >>= 3;
   L = w & 15;
}

void dasm(addr_t p_pc) {
   word_t op;
   char *mnemo;
   long_t disp, imm;
   int cc, sel;
   word_t mask;
   static char *bits[] = {"BTST", "BCHG", "BCLR", "BSET"};

   hexbuf[0] = '\0'; mnemobuf[0] = '\0';
   X("\n%04X:",p_pc);
   pc = p_pc; mnemo="<ILLEGAL>";
   op = readWord(&pc); decode(op);
   switch(L) {
   case 0x0:
      if(0==s) {
         switch(r) {
         case 0:
            mnemo=" ORI";
LxxxI_R:    if(7==M && 4==R){
               if(0==S) {
                  imm=readWord(&pc) & 0xFF;
                  O("%s.B #$%X,CCR",mnemo,imm);
               }
               else if(1==S){
                  imm=readWord(&pc);
                  O("%s.W #$%X,SR",mnemo,imm);
               }
            }
            else {
LxxxI:         imm=readS(&pc,S);
               O("%s",mnemo);OpSize0(S);
               O(" #$%X,",imm);
               AdrMode(M,R);
            }
            break;
         case 1: mnemo="ANDI"; goto LxxxI_R;
         case 2: mnemo="SUBI"; goto LxxxI;
         case 3: mnemo="ADDI"; goto LxxxI;
         case 4:
            imm = sext8(readWord(&pc) & 0xFF);
            mnemo = bits[S];
            O("%s.B #$%X,",mnemo,imm);AdrMode(M,R);
            break;
         case 5: mnemo="EORI"; goto LxxxI_R;
         case 6: mnemo="CMPI"; goto LxxxI;
         }
      }
      else { // s = 1
         if (M == 0x1) { // MOVEP
            disp = readWord(&pc);
            O("MOVEP");OpSize1(S & 1);
            if (S & 2) // reg2mem
               O("D%d,(%d,A%d)",r,disp,R);
            else
               O("(%d,A%d),D%d",disp,R,r);
         }
         else {
            mnemo = bits[S];
            O("%s.L D%d,",mnemo,r);AdrMode(M,R);
         }
      }
      break;
   case 0x1:
   case 0x2:
   case 0x3: // MOVEA,MOVE
      S += (s << 2);
      O(1==S? "MOVEA":"MOVE");OpSize2(L & 3);AdrMode(M,R);O(",");AdrMode(S,r);
      break;
   case 0x4:
      if (0 == s) {
         switch(r) {
         case 0:
            if(3==S){ O("MOVE SR,");AdrMode(M,R); }
            else { mnemo="NEGX";goto LxxxSMR; }
            break;
         case 1:
            mnemo="CLR";
LxxxSMR:    O("%s",mnemo);OpSize0(S);AdrMode(M,R);
            break;
         case 2:
            if(3==S) { O("MOVE ");AdrMode(M,R);O(",CCR"); }
            else { mnemo="NEG"; goto LxxxSMR; }
            break;
         case 3:
            if(3==S) { O("MOVE ");AdrMode(M,R);O(",SR"); }
            else { mnemo="NOT"; goto LxxxSMR; }
            break;
         case 4:
            switch(S){
            case 0:
               O("NBCD ");AdrMode(M,R);
               break;
            case 1:
               if(0==M) O("SWAP D%d",R);
               else { O("PEA ");AdrMode(M,R); }
               break;
            default:
               if(0==M) { O("EXT");OpSize1(S & 1);O(" D%d",R); }
               else {
                  mask = readWord(&pc);
                  O("MOVEM");OpSize1(S & 1);
                  RegMask(R2M,mask);O(",");AdrMode(M,R);
               }
            }
            break;
         case 5:
            if(3==S && 7==M && 4==R) O("ILLEGAL");
            else {
               if(3==S) { O("TAS ");AdrMode(M,R); }
               else { mnemo="TST"; goto LxxxSMR; }
            }
            break;
         case 6:
            mask = readWord(&pc);
            O("MOVEM");OpSize1(S & 1);
            AdrMode(M,R);O(",");RegMask(M2R,mask);
            break;
         case 7:
            switch(S) {
            case 0: illegal(p_pc,op);break;
            case 1:
               switch(M >> 1) {
               case 0: O("TRAP #%d",((M & 1)<<3)+R);break;
               case 1:
                  if(0==(M & 1)) {
                     disp = readWord(&pc);
                     O("LINK A%d,#$%X",R,disp);
                  }
                  else
                     O("UNLK A%d",R);
                  break;
               case 2:
                  O("MOVE ");
                  if(M & 1) O("USP,A%d",R);
                  else O("A%d,USP",R);
                  break;
               case 3:
                  if(M & 1) illegal(p_pc,op);
                  switch(R) {
                  case 0:mnemo="RESET";break;
                  case 1:mnemo="NOP";break;
                  case 2:mnemo="STOP";imm=readWord(&pc);break;
                  case 3:mnemo="RTE";break;
                  case 4:illegal(p_pc,op);break;
                  case 5:mnemo="RTS";break;
                  case 6:mnemo="TRAPV";break;
                  case 7:mnemo="RTR";break;
                  }
                  O("%s",mnemo);if(2==R) O(" #$%X",imm);
                  break;
               }
               break;
            case 2: O("JSR ");AdrMode(M,R);break;
            case 3: O("JMP ");AdrMode(M,R);break;
            }
            break;
         }
      }
      else { // s = 1
         if(S<2) illegal(p_pc,op);
         if(3==S) { O("LEA ");AdrMode(M,R);O(",A%d",r); }
         else { O("CHK ");AdrMode(M,R);O(",D%d",r); }
      }
      break;
   case 0x5:
      switch(S) {
      case 0:
      case 1:
      case 2:
         mnemo=s? "SUBQ":"ADDQ";
         O("%s",mnemo);OpSize0(S);O(" #$%X,",r);AdrMode(M,R);
         break;
      case 3:
         cc = (r << 1) + s;
         if(1==M) {
            disp = pc;
            imm = sext16(readWord(&pc));
            disp += imm;
            O("DB");Cond(cc);O("D%d,$%X",R,disp);
         }
         else {
            O("S");Cond(cc);AdrMode(M,R);
         }
      }
      break;
   case 0x6:
      cc = (r << 1) + s;
      disp = pc;
      imm = sext8(op & 0xFF);
      if (0 == imm) imm = sext16(readWord(&pc));
      disp += imm;
      if (0 == cc) O("BRA ");
      else if (1 == cc) O("BSR ");
      else { O("B");Cond(cc); }
      O(" $%X",disp);
      break;
   case 0x7:
      if(s) illegal(p_pc,op);
      O("MOVEQ #$%X,D%d",op & 0xFF,r);
      break;
   case 0x8:
      if(3 == S) {
         mnemo=s? "DIVS":"DIVU";
Lmuldiv:
         O("%s ",mnemo);AdrMode(M,R);O(",D%d",r);
      }
      else if (0 == S && 1 == s && M < 2) {
         mnemo="SBCD";
Lbcd:
         O("%s ",mnemo);RedMode(M,R);O(",");RedMode(M,r);
      }
      else {
         mnemo="OR";
LxxxDSMR:
         O("%s",mnemo);OpSize0(S);
         if(s) { O("D%d,",r);AdrMode(M,R); }
         else { AdrMode(M,R);O(",D%d",r); }
      }
      break;
   case 0x9:
      mnemo="SUB";
Lsubadd:
      if (3 == S) { // SUBA
         O("%sA",mnemo);OpSize1(s);AdrMode(M,R);O(",A%d",r);
      }
      else if (M < 2 && s) { // SUBX
         O("%sX",mnemo);OpSize0(S);
         RedMode(M,R);O(",");RedMode(M,r);
      }
      else // SUB
         goto LxxxDSMR;
      break;
   case 0xA: O("Line A"); illegal(p_pc,op); break;
   case 0xB:
      if(3 ==S) {
         O("CMPA");OpSize1(s);AdrMode(M,R);O(",A%d",r);
      }
      else if (1 == M && s) {
         O("CMPM");OpSize0(S);O("(A%d)+,(A%d)+",R,r);
      }
      else if (s) {
         O("EOR");OpSize0(S);O("D%d,",r);AdrMode(M,R);
      }
      else {
         O("CMP");OpSize0(S);AdrMode(M,R);O(",D%d",r);
      }
      break;
   case 0xC:
      if(3 == S) {
         mnemo=s? "MULS":"MULU"; goto Lmuldiv;
      }
      else if (0 == S && 1 == s && M < 2) {
         mnemo="ABCD"; goto Lbcd;
      }
      else if (1 ==s && M < 2) { // EXG
         //             SSMMM
         // D<r>,D<R>   01000
         // A<r>,A<R>   01001
         // D<r>,A<R>   10001

         if(0==S) illegal(p_pc,op);
         O("EXG ");
         if(1==S) {
            if(M&1) O("A%d,A%d",r,R);
            else O("D%d,D%d",r,R);
         }
         else // 2==S
            O("D%d,A%d",r,R);
      }
      else {
         mnemo="AND"; goto LxxxDSMR;
      }
      break;
   case 0xD: mnemo="ADD"; goto Lsubadd;
   case 0xE:
      sel = 3 == S? r : (M & 3);
      switch(sel) {
      case 0: mnemo="AS";break;
      case 1: mnemo="LS";break;
      case 2: mnemo="ROX";break;
      case 3: mnemo="RO";break;
      }
      if(3 == S){
         O("%s",mnemo);GreenDir(s);O(" ");AdrMode(M,R);
      }
      else {
         O("%s",mnemo);GreenDir(s);OpSize0(S);
         if(M & 4) O("D%d,D%d",r,R);
         else O("#$%X,D%d",r?r:8,R);
      }
      break;
   case 0xF: O("Line F"); illegal(p_pc,op);break;
   }

   printf("%s  %s",hexbuf,mnemobuf);
}
