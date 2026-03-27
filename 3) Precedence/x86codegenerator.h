#ifndef X86CODEGENERATOR_H
#define X86CODEGENERATOR_H


#include "globals.h"
#include "gen.h"

int alloc_register(void);
void free_register(int reg);

int cgload(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);

void cgprintint(int r);
void cgpreamble(void);
void cgpostamble(void);

#endif
