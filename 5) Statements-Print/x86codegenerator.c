#include<stdio.h>
#include<stdlib.h>
#include"globals.h"
#include"token.h"

// Forward declaration
int cgcompare_and_set(int r1, int r2, char *how);

static char *reglist[4]= { "%r8", "%r9", "%r10", "%r11" };
static char *breglist[4]= { "%r8b", "%r9b", "%r10b", "%r11b" };

static int freereg[4] = {1, 1, 1, 1};  // 1 = free, 0 = used

int alloc_register(void) {
  for (int i = 0; i < 4; i++) {
    if (freereg[i]) {
      freereg[i] = 0;
      return i;
    }
  }
  fprintf(stderr, "Out of registers!\n");
  exit(1);
}

void free_register(int r) {
  if (freereg[r] != 0)
    fprintf(stderr, "Error: register already free!\n");
  freereg[r] = 1;
}

// Assembly header
void cgpreamble(void) {
  fprintf(Outfile, "\t.text\n\t.globl main\nmain:\n");
}

// Footer with call to exit
void cgpostamble(void) {
  fprintf(Outfile, "\tmovq\t$0, %%rdi\n\tcall\texit\n");
}

int cgload(int value) {
  int r= alloc_register();
  // Print out the code to initialise it
  fprintf(Outfile, "\tmovq\t$%d, %s\n", value, reglist[r]);
  return(r);
}

// Add two registers together and return
// the number of the register with the result
int cgadd(int r1, int r2) {
  fprintf(Outfile, "\taddq\t%s, %s\n", reglist[r1], reglist[r2]);
  free_register(r1);
  return(r2);
}

// Multiply two registers together and return
// the number of the register with the result
int cgmul(int r1,int r2){
    fprintf(Outfile,"\timulq\t%s, %s\n", reglist[r1], reglist[r2]);
    free_register(r1);  // Fixed: free the source operand
    return r2;  // Fixed: result is in r2
}

// Subtract the second register from the first and
// return the number of the register with the result
int cgsub(int r1, int r2) {
  fprintf(Outfile, "\tsubq\t%s, %s\n", reglist[r2], reglist[r1]);
  free_register(r2);
  return(r1);
}

// Divide the first register by the second and
// return the number of the register with the result
int cgdiv(int r1, int r2) {
  fprintf(Outfile, "\tmovq\t%s,%%rax\n", reglist[r1]);
  fprintf(Outfile, "\tcqo\n");
  fprintf(Outfile, "\tidivq\t%s\n", reglist[r2]);
  fprintf(Outfile, "\tmovq\t%%rax,%s\n", reglist[r1]);
  free_register(r2);
  return(r1);
}

void cgprintint(int r) {
  fprintf(Outfile, "\tmovq\t%s, %%rdi\n", reglist[r]);
  fprintf(Outfile, "\tcall\tprintint\n");
  free_register(r);
}

// Helper function for comparisons - fixed operand sizes
int cgcompare_and_set(int r1, int r2, char *how) {
  fprintf(Outfile, "\tcmpq\t%s, %s\n", reglist[r2], reglist[r1]);
  fprintf(Outfile, "\t%s\t%s\n", how, breglist[r2]);  // Use byte register
  fprintf(Outfile, "\tmovzbq\t%s, %s\n", breglist[r2], reglist[r2]);  // Zero-extend byte to quad
  free_register(r1);
  return r2;
}

// Comparison operations
int cgequal(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "sete");
}

int cgnotequal(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "setne");
}

int cgless(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "setl");
}

int cggreater(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "setg");
}

int cglessequal(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "setle");
}

int cggreaterequal(int r1, int r2) {
  return cgcompare_and_set(r1, r2, "setge");
}

// Compare and jump operations
void cgcompare_and_jump(int op, int r, int label) {
  fprintf(Outfile, "\tcmpq\t$0, %s\n", reglist[r]);
  
  switch (op) {
    case A_EQ:
      fprintf(Outfile, "\tje\tL%d\n", label);
      break;
    case A_NE:
      fprintf(Outfile, "\tjne\tL%d\n", label);
      break;
    case A_LT:
      fprintf(Outfile, "\tjl\tL%d\n", label);
      break;
    case A_GT:
      fprintf(Outfile, "\tjg\tL%d\n", label);
      break;
    case A_LE:
      fprintf(Outfile, "\tjle\tL%d\n", label);
      break;
    case A_GE:
      fprintf(Outfile, "\tjge\tL%d\n", label);
      break;
  }
}

// Generate a label
void cglabel(int l) {
  fprintf(Outfile, "L%d:\n", l);
}

// Generate an unconditional jump
void cgjump(int l) {
  fprintf(Outfile, "\tjmp\tL%d\n", l);
}