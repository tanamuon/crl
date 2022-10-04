#ifndef SKILLS_H
#define SKILLS_H

#include <stdio.h>

//struct Unit *unit;

struct Skill {
	char name[MAXNSIZE];
	char desc[DESCSIZE];
	bool color;
	bool *pval;
};

extern void learn_ability();
extern bool select_ability();
extern bool leap(struct Unit *unit, int dx, int dy);
extern bool dash_or_smash(struct Unit *unit, int dx, int dy);
extern bool dash(struct Unit *unit, int dx, int dy);
extern bool punt(struct Unit *unit, int dx, int dy, int range);

#endif
