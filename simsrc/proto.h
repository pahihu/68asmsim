
/***************************** 68000 SIMULATOR ****************************

File Name: PROTO.H
Version: 1.0

This file contains function prototype definitions for all functions
	in the program

***************************************************************************/



int MOVE(void );
int MOVEP(void );
int MOVEA(void );
int MOVE_FR_SR(void );
int MOVE_TO_CCR(void );
int MOVE_TO_SR(void );
int MOVEM(void );
int MOVE_USP(void );
int MOVEQ(void );
int EXG(void );
int LEA(void );
int PEA(void );
int LINK(void );
int UNLK(void );
int ADD(void );
int ADDA(void );
int ADDI(void );
int ADDQ(void );
int ADDX(void );
int SUB(void );
int SUBA(void );
int SUBI(void );
int SUBQ(void );
int SUBX(void );
int DIVS(void );
int DIVU(void );
int MULS(void );
int MULU(void );
int NEG(void );
int NEGX(void );
int CMP(void );
int CMPA(void );
int CMPI(void );
int CMPM(void );
int TST(void );
int CLR(void );
int EXT(void );
int ABCD(void );
int SBCD(void );
int NBCD(void );
int AND(void );
int ANDI(void );
int ANDI_TO_CCR(void );
int ANDI_TO_SR(void );
int OR(void );
int ORI(void );
int ORI_TO_CCR(void );
int ORI_TO_SR(void );
int EOR(void );
int EORI(void );
int EORI_TO_CCR(void );
int EORI_TO_SR(void );
int NOT(void );
int SHIFT_ROT(void );
int SWAP(void );
int BIT_OP(void );
int TAS(void );
int BCC(void );
int DBCC(void );
int SCC(void );
int BRA(void );
int BSR(void );
int JMP(void );
int JSR(void );
int RTE(void );
int RTR(void );
int RTS(void );
int NOP(void );
int CHK(void );
int ILLEGAL(void );
int RESET(void );
int STOP(void );
int TRAP(void );
int TRAPV(void );
int show_topics(void );
int gethelp(void );
int at(int y,int x);
int home(void );
int clrscr(void );
char chk_buf(void );
int parse(char *str,char * *ptrbuf,int maxcnt);
int iswhite(int c,char *qflag);
int decode_size(Long *result);
int eff_addr(Long size,int mask,int add_times);
int runprog(void );
void exec_inst(void );
void exception(int class,Long loc,int r_w);
int main(void );
void init(void );
void finish(void );
void errmess(void );
void cmderr(void );
void setdis(void );
void scrshow(void );
int mdis(void );
int selbp(void );
int sbpoint(void );
int cbpoint(void );
int dbpoint(void );
int memread(int loc,int MASK);
int memwrite(int loc,Long value);
int alter(void );
int hex_to_dec(void );
int dec_to_hex(void );
int intmod(void );
int portmod(void );
int pcmod(void );
int changemem(Long oldval,Long *result);
void mmod(void );
int regmod(char *regpntr,int data_or_mem);
int mfill(void );
int clear(void );
char *gettext(int word,char *prompt);
int same(char *str1,char *str2);
int eval(char *string);
int eval2(char *string,Long *result);
int getval(int word,char *prompt);
int strcopy(char *str1,char *str2);
char *mkfname(char *cp1,char *cp2,char *outbuf);
int pchange(int oldval);
int to_2s_comp(Long number,Long size,Long *result);
int from_2s_comp(Long number,Long size,Long *result);
int sign_extend(int number,Long size_from,Long *result);
int inc_cyc(int num);
int eff_addr_code(int inst,int start);
int a_reg(int reg_num);
int mem_put(Long data,int loc,Long size);
int mem_req(int loc,Long size,Long *result);
int mem_request(int *loc,Long size,Long *result);
void put(void *dest,Long source,Long size);
void value_of(Long *EA,Long *EV,Long size);
int cc_update(int x,int n,int z,int v,int c,Long source,Long dest,Long result,Long size,int r);
int check_condition(int condition);

