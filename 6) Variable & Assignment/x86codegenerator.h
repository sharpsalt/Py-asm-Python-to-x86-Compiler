#ifndef X86CODEGENERATOR_H
#define X86CODEGENERATOR_H

#include "globals.h"
// #include "gen.h"

int alloc_register(void);
void free_register(int reg);

int cgload(int value);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);

// Comparison operations
int cgequal(int r1, int r2);
int cgnotequal(int r1, int r2);
int cgless(int r1, int r2);
int cggreater(int r1, int r2);
int cglessequal(int r1, int r2);
int cggreaterequal(int r1, int r2);
int cgcompare_and_set(int r1, int r2, char *how);

// Control flow operations
void cgcompare_and_jump(int op, int r, int label);
void cglabel(int l);
void cgjump(int l);

void cgprintint(int r);
void cgpreamble(void);
void cgpostamble(void);

int get_global_offset(const char *name);
int cgloadglob(const char *name);
void cgstorglob(const char *name, int r);

#endif