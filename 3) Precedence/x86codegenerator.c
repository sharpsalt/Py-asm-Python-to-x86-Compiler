#include<stdio.h>
#include<stdlib.h>
#include"globals.h"

static char *reglist[4]= { "%r8", "%r9", "%r10", "%r11" };

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
    free_register(r2);
    return r1;
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