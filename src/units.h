#ifndef UNITS_H
#define UNITS_H

#include <stdbool.h>
#include "../include/libtcod.h"

struct Inventory {
	int medi;
	int stim;
	int pcell;
	int berserk;
	int smoke;
	int glue;
	int paint;
	int barrier;
	int frag;
	int rockets;
	int drill;
	int flare;
	int noisemaker;
};

struct Weapon {
	// replace the bools with flags
	char name[MAXNSIZE];
	char desc[DESCSIZE];
	bool is_ranged;
	int projpershot;
	int dmgdie;
	int dmgsides;
	int acc;
	int acc_pen;
	int exp;
	bool set_req;
	int setupx;
	int setupy;
	int AP;
	int critmod;
	//bool cleave;
	int id;
	int handedness;
	int nmods;
	int inmods[12];
	bool dupes;
	bool nodisarm;
	bool suppressed;
	bool knockback; // remove
	bool tagger;
	bool bipod;
	bool elec;
};

struct Explosive {
	char name[MAXNSIZE];
	char desc[DESCSIZE];
	bool map_alt;
	bool smoke;
	bool glue;
	bool paint;
	bool barrier;
	bool flare;
	int dmgdie;
	int dmgsides;
	int radius;

};

struct Gear {
	struct Weapon wep;
	struct Gear *next;
	struct Gear *prev;
};

struct Armor {
	char name[MAXNSIZE];
	char type;
	char desc[DESCSIZE];
	int acdie;
	int acsides;
	int crit_res;
	// simplify
	int ev_pen;
	int melee_pen;
	int acc_pen;
	int stealth_pen;
	int per_pen;
	//
	int exp;
};

struct Unit {
	int fac;
	char name[MAXNSIZE];
	char desc[DESCSIZE];
	
	// int spawnInfo[3]
	int mindepth;
	int maxdepth;
	int spawnweight;
	
	int c;
	// int rgb[3];
	int r;
	int g;
	int b;
	
	int x;
	int y;
	
	int fov;
	
	int exp;
	int abilities;
	
	// int moveRecord[8]
	// feel like there's some duplication
	int ltx;
	int lty;
	int lt_dx;
	int lt_dy;
	int destx;
	int desty;
	int plastx;
	int plasty;
	
	int uhp;
	int mhp;
	int chp;
	int mpow;
	int cpow;
	
	// int skills[5];
	int ev;
	int melee;
	int ranged;
	int per;
	int stealth;
	//
	int turns_seen;
	int berserk;
	int commwait;
	
	// Skills
	int ownedAbilities1;
	int activeAbilities1;
	int ownedAbilities2;
	int activeAbilities2;
	
	// v - turn these into an int
	int statusFlags1;
	bool silenced;
	bool noretal;
	bool riposting;
	bool camo;
	bool fam;
	bool hunting;
	bool flanking;
	bool stuck;
	bool tagged;
	bool aware;
	bool dest;
	bool blind;
	bool immobilizer;
	bool noevade;
	bool running;
	bool crippled;
	bool comms;
	bool noescape; // dumb hack
	bool lit;
	bool turnspent;
	// some of this can be done better
	int conc_pen;
	int ducking;
	int spturn;
	int sturns;
	int bturns;
	int regen;
	int oc;
	int spos;
	// this can be done better
	struct Inventory inv;
	struct Gear *gear;
	struct Weapon wep;
	struct Weapon mwep;
	struct Weapon rwep;
	struct Armor b_arm;
	struct Armor h_arm;
	struct Armor a_arm;
	struct Armor l_arm;
};

extern void gen_allowed_units(struct Unit allowed_units[MAXUNITS], int *dunits, int *sweight_total);
extern void gen_unit_list();
extern struct Armor calc_armor(struct Unit unit);
extern int check_crit_res(struct Unit unit);
extern int check_stealth(struct Unit unit, struct Unit observer);
extern int check_perception(struct Unit unit);
extern int check_fov(struct Unit unit);
extern int check_cover(struct Unit unit);
extern int check_concealment(struct Unit unit);
extern int roll_die(int sides, int atk, int def);
extern bool weld_door(struct Unit *unit, int dx, int dy);
extern bool camo(struct Unit *unit);
extern void dec_pow(struct Unit *unit);
extern void regen(struct Unit *unit);
extern void die(struct Unit *unit, bool m);
extern void take_dmg(struct Unit *attacker, struct Unit *defender, struct Weapon wep);
extern void take_indirect_dmg(struct Unit *defender, struct Explosive wep);
extern void explode(struct Explosive bomb, int x, int y);
extern void enemies_in_view(struct Unit *unit);
extern bool check_if_in_view(int x);
extern void shoot_info(int t_ind);
extern bool shoot(struct Unit *unit, TCOD_key_t skey);
extern int find_unit_ind(int x, int y);
extern bool move_or_attack(struct Unit *unit, int dx, int dy);
extern bool move_towards(struct Unit *mover, int x, int y);
//extern void ai_unit_turn(struct Unit *unit);
extern void ai_turn();
extern void draw_unit(struct Unit unit);
extern void draw_units();
extern void draw_units_om();

#endif
