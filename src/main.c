#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "../include/libtcod.h"
#include "system.h"
#include "items.h"
#include "skills.h"
#include "units.h"
#include "map.h"
#include "main.h"

int last_hit;
int utypes;
struct Unit *unit_types;
int wtypes;
struct Weapon *wep_types;
int mtypes;
struct Weapon *mod_types;
int atypes;
struct Armor *armor_types;
struct Unit unit_list[MAXUNITS];
struct Unit player;
int unit_num;
struct Tile map[MAPX][MAPY];
TCOD_map_t tmap;
int turn;
int depth;
char msgs[MAXMSGS][MSGSIZE];
char smsgs[STORYMSGS][MSGSIZE];
int msgs_s;
int units_in_view[MAXUNITS]; // this gives the unit_list index of all units in player fov
int nunits;
struct Unit splayer;
int ndoors = 0;
int doorsxy[MAPX*MAPY][2];


void read_json(char *filename, int strsize, void (*callback)(char *, char *, int *, int *, int) );
void load_map(char *key, char *val, int *smi, int *smj, int strsize);
void load_units(char *key, char *val, int *i, int *j, int strsize);
void load_player(char *key, char *val, int *sui, int *j, int strsize);
void load_game();
extern void gen_json_lists();

struct Unit * read_units() {
	struct Unit * unitList;
	char line[DESCSIZE*2];
	char *fn = "../json/enemies.txt";
	FILE *fp = fopen(fn, "r");
	if (fp == NULL) {
		printf("Error: could not find %s", fn);
	}
	int unitTypes = 0;
	int i, f;	
	char name[DESCSIZE*2];
	while (fgets(line, DESCSIZE*2, fp) != NULL) {
		if (line[0]!='#') {	
			if (line[0]=='I') unitTypes++;
			else if (line[0]=='N') printf("name:%s\n", line);
			else if (line[0]=='D') printf("description:%s\n", line);			
		}
	}
	fclose(fp);
	return unitList;
}

void menu(int n, char *strings[n], int *val[n], struct Unit *unit, bool xpreq, int xpflat, int xpfac, int cx, int cy, bool callback(int *, int *, struct Unit *, TCOD_key_t, int, int)) {
	int cursx = 0; int cursy = 0;
	int xp = unit->exp;
	int valcopy[n];
	for (int j=0; j<n; j++) {
		valcopy[j] = *val[j];
	}
	TCOD_console_clear(NULL);
	for (int i=0; i<n; i++) TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, strings[i], *val[i]);
	TCOD_console_print_rect(NULL, cursx, cursy, MAPX+HUDX, MAPY+MSGY, "> ");
	if (xpreq) TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", xp, (valcopy[cursy]+1)*xpfac + xpflat);
	TCOD_console_flush();
	TCOD_key_t skey = TCOD_console_wait_for_keypress(true);
	while (true) {
		TCOD_console_clear(NULL);
		TCOD_console_print_rect(NULL, cursx, cursy, MAPX+HUDX, MAPY+MSGY, "  ");
		inc_curs(skey, &cursx, &cursy, n);
		int v = valcopy[cursy];
		bool used = callback(&valcopy[cursy], val[cursy], unit, skey, cursx, cursy);
		if (used && xpreq) {
			if (valcopy[cursy] > v) {
				if (valcopy[cursy]*xpfac + xpflat <= xp) {
					xp -= valcopy[cursy]*xpfac + xpflat;
				} else {
					valcopy[cursy] = v;
				}
			} else if (valcopy[cursy] < v) {
				xp += (valcopy[cursy]+1)*xpfac + xpflat;
			}
		}
		if (used && !xpreq) return;
		if (skey.vk==TCODK_ENTER) {
			for (int k=0; k<n; k++) *val[k] = valcopy[k];
			if (xpreq) unit->exp = xp;
			return;
		} else if (skey.vk == TCODK_ESCAPE && !unit->noescape) { // dumb hack coming through
			return;
		}
		for (int i=0; i<n; i++) {
			TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, strings[i], valcopy[i]);
		}
		TCOD_console_print_rect(NULL, cursx, cursy, MAPX+HUDX, MAPY+MSGY, "> ");
		if (xpreq) TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", xp, (valcopy[cursy]+1)*xpfac + xpflat);
		TCOD_console_flush();
		skey = TCOD_console_wait_for_keypress(true);
	}
}

bool menu_5game(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.vk == TCODK_ENTER) {
		switch(cursy) {
			case 0: draw_all(); return false;
			case 1: save_units(); return false; 
			case 2: load_game(); return false;
			case 3: start_game(); return false;
			case 4: exit(0);
		}
	}
	return false;
}

bool menu_3game(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.vk == TCODK_ENTER) {
		switch(cursy) {
			case 0: start_game(); return false;
			case 1: draw_all(); return false;
			case 2: exit(0);
		}
	}
	return false;
}

bool menu_basic(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.vk==TCODK_RIGHT || skey.vk==TCODK_KP6 || skey.c=='l') {
		*valcopy = *valcopy + 1;
		return true;
	} else if ( (skey.vk==TCODK_LEFT || skey.vk==TCODK_KP4 || skey.c=='h') && *valcopy > *val) {
		*valcopy = *valcopy - 1;
		return true;
	}
	return false;
}

bool menu_char(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.vk==TCODK_ENTER) {
		if (cursy == 0) {
			craft_wep(&player);
		} else if (cursy == 1) {
		char *strings[NUMITEMS] = {"Medigel:%d", "Stim Pack:%d", "Power Cell:%d", "Berserk Cell:%d", "Smoke Grenades:%d", "Glue Grenades:%d", "Paint Grenades:%d", "Barrier Grenades:%d", "Frag Grenades:%d", "Rockets:%d", "Drill Rockets:%d", "Flares:%d", "Noisemakers:%d"};
		int *vals[NUMITEMS] = {&(player.inv.medi), &(player.inv.stim), &(player.inv.pcell), &(player.inv.berserk), &(player.inv.smoke), &(player.inv.glue), &(player.inv.paint), &(player.inv.barrier), &(player.inv.frag), &(player.inv.rockets), &(player.inv.drill), &(player.inv.flare), &(player.inv.noisemaker)};
		menu(NUMITEMS, strings, vals, &player, true, 500, 0, cursx, cursy, menu_basic);
		} else if (cursy == 2) {
			view_weps(true, unit);
		} else if (cursy == 3) {
			upgrade_armor(unit);
		} else if (cursy == 4) {
			int *val[5] = {&(player.melee), &(player.ranged), &(player.stealth), &(player.ev), &(player.per)};
			char *strings[5] = {"Melee:%d", "Ranged:%d", "Stealth:%d", "Evasion:%d", "Perception:%d"};
			menu(5, strings, val, unit, true, 0, 100, cursx, cursy, menu_basic);
		} else if (cursy == 5) {
			learn_ability();
		}
	}
	return false;
}

bool menu_forge(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
		if (cursy == 0 && skey.vk==TCODK_ENTER) {
			craft_wep(unit);
		} else if (cursy == 1 && skey.vk==TCODK_ENTER) {
			view_weps(true, unit);
		} else if (cursy == 2 && skey.vk==TCODK_ENTER) {
			upgrade_armor(unit);
		}
		return false;
}

bool menu_victory(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.vk == TCODK_ENTER) {
		print_msg(smsgs[cursy+5]);
	}
	return false;	
}

void draw_all () {
	TCOD_console_clear(NULL);
	draw_map(); 
	//draw_map_om(); //
	draw_unit(player); 
	draw_units();
	//draw_units_om(); //
	draw_msgs();
	draw_info();
	TCOD_console_flush();
}

void gen_inven() {
	player.gear = (struct Gear*)calloc(1, sizeof(struct Gear));
	player.gear->wep = wep_types[0];
	if (wtypes>1) {
		player.gear->next = (struct Gear*)calloc(1, sizeof(struct Gear));
		player.gear->next->wep = wep_types[1];
		player.mwep = player.gear->next->wep;
	}
	player.rwep = player.gear->wep; player.wep = player.mwep;
}

int floorspace() {
	int floorspace = 0;
	for (int i=0; i<MAPX; i++) {
		for (int j=0; j<MAPY; j++) {
			if (map[i][j].can_walk) floorspace++;
		}
	}
	return floorspace;
}

void gen_all() {
	int fs = 0;
	while (fs < 1000) {
		fill_wall();
		//gen_map();
		struct Leaf root; root.x1 = 1; root.y1 = 1; root.xsize = MAPX-1; root.ysize = MAPY-1; root.rdepth = 0;
		gen_leaf(&root);
		fs = floorspace(); 
	}
	//tunnel();
	//place_dstair();
	gen_tmap();
	pick_valid_xy(&(player.x),&(player.y));
	gen_unit_list();
	player.turns_seen = 0;
	player.regen = 0;
	player.inv.flare = 2;
	gen_inven();
}

bool gen_new_level() {
	if (!map[player.x][player.y].down_stair) return false;
	fill_wall();
	struct Leaf root; root.x1 = 1; root.y1 = 1; root.xsize = MAPX-1; root.ysize = MAPY-1;
	gen_leaf(&root);
	place_dstair();
	TCOD_map_clear(tmap, false, false);
	gen_tmap();
	int x, y;
	pick_valid_xy(&x, &y);
	player.x = x; player.y = y;
	unit_num = (int)(0.40*MAXUNITS + rand() % MAXUNITS*0.60);
	gen_unit_list();
	depth+=1;
	player.exp+=1000;
	if (depth > 10) {
		int cursx = 0; int cursy = 0;
		char *strings[3] = {smsgs[3], smsgs[4], smsgs[5]};
		int null = 1; int *vals[3] = {&null, &null, &null};
		struct Unit gmenu; gmenu.noescape = true;
		menu(3, strings, vals, &gmenu, false, 0, 0, cursx, cursy, menu_3game);
		char *strings2[3] = {"Start new game.", "Load saved game.", "Exit game."};
		menu(3, strings2, vals, &gmenu, false, 0, 0, cursx, cursy, menu_3game);
	}
	return true;
}

void get_dxdy(TCOD_key_t key, int *dx, int *dy) {
	*dx = 0; *dy = 0;
	if (key.vk==TCODK_UP || key.vk==TCODK_KP8 || key.c=='k') {
		*dy = -1;
	} else if (key.vk==TCODK_DOWN || key.vk==TCODK_KP2 || key.c=='j') {
		*dy = 1;
	} else if (key.vk==TCODK_LEFT || key.vk==TCODK_KP4 || key.c=='h') {
		*dx = -1;
	} else if (key.vk==TCODK_RIGHT || key.vk==TCODK_KP6 || key.c=='l') {
		*dx = 1;
	} else if (key.vk==TCODK_LEFT || key.vk==TCODK_KP7 || key.c=='y') {
		*dx = -1; *dy = -1;
	} else if (key.vk==TCODK_RIGHT || key.vk==TCODK_KP9 || key.c=='u') {
		*dx = 1; *dy = -1;
	} else if (key.vk==TCODK_LEFT || key.vk==TCODK_KP1 || key.c=='b') {
		*dx = -1; *dy = 1;
	} else if (key.vk==TCODK_RIGHT || key.vk==TCODK_KP3 || key.c=='n') {
		*dx = 1; *dy = 1;
	}
}

bool target(int *x, int *y, int r) {
	int dx = 0; int dy = 0; 
	TCOD_key_t skey;
	//draw_all();
	while (!(skey.vk == TCODK_ESCAPE || skey.vk == TCODK_ENTER)) {
		get_dxdy(skey, &dx, &dy);
		*x= *x+dx; *y= *y+dy;
		int ind;
		draw_all();
		for (int i=*x-r; i<*x+r+1; i++) {
			for (int j=*y-r; j<*y+r+1; j++) {
				ind = find_unit_ind(i, j);
				if (i<MAPX && i>-1 && j<MAPY && j>-1) {
					TCOD_console_set_char_background(NULL, i, j, TCOD_light_grey, TCOD_BKGND_LIGHTEN);
				}
				int xi = i; int yj = j;
				//if (TCOD_map_is_in_fov(tmap, i, j)) {
					/*
					TCOD_console_put_char(NULL, i, j, map[i][j].c, TCOD_BKGND_LIGHTEN);
					if (check_if_in_view(ind)) {
						TCOD_console_put_char(NULL, i, j, unit_list[ind].c, TCOD_BKGND_LIGHTEN);
					} else if (ind == -2) {
						TCOD_console_put_char(NULL, i, j, player.c, TCOD_BKGND_LIGHTEN);
					}*/	
				/*} else {
					TCOD_console_put_char(NULL, i, j, 177, TCOD_BKGND_LIGHTEN);
				}*/
				
				if (TCOD_map_is_in_fov(tmap, xi, yj)) {
	TCOD_console_put_char(NULL, i, j, map[i][j].c, TCOD_BKGND_LIGHTEN);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+1, (MAPX+MSGSIZE)/2, 20, "%s [%c]", map[i][j].name, map[i][j].c);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+3, (MAPX+MSGSIZE)/2, 20, "%s", map[i][j].desc);
				}
				if (check_if_in_view(ind)) {
	struct Unit unit = unit_list[ind];
	TCOD_console_put_char(NULL, i, j, unit.c, TCOD_BKGND_LIGHTEN);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+1, (MAPX+MSGSIZE)/2, 20, "%s [%c]\n\n", unit.name, unit.c);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+3, (MAPX+MSGSIZE)/2, 20, "Does [%d, %dd%d] damage, has %d max hp, and [%d, %dd%d] defense. \n\n%s", unit.melee, unit.wep.dmgdie, unit.wep.dmgsides, unit.mhp, unit.ev, unit.b_arm.acdie, unit.b_arm.acsides, unit.desc);
				}
			}
		}
		TCOD_console_flush();
		skey = TCOD_console_wait_for_keypress(true);
	}
	if (skey.vk == TCODK_ENTER) {
		return true;
	} 
	return false;
}

bool look() {
	TCOD_console_put_char(NULL, player.x, player.y, player.c, TCOD_BKGND_LIGHTEN);
	TCOD_console_flush();
	int x = player.x; int y = player.y;
	target(&x, &y, 0);
	draw_all();
	return false;
}

void print_curs(int cursx, int cursy) {
	TCOD_console_print_rect(NULL, cursx, cursy, MAPX+HUDX, MAPY+MSGY, "> ", depth);
	TCOD_console_flush();
}

void inc_curs(TCOD_key_t skey, int *cursx, int *cursy, int m_items) {
	if (skey.vk==TCODK_DOWN || skey.vk==TCODK_KP2 || skey.c=='j') {
		if (*cursy+1<m_items) {
			*cursy+=1;
		} else {
			*cursy = 0;
		}
	} else if (skey.vk==TCODK_UP || skey.vk==TCODK_KP8 || skey.c=='k') {
		if (*cursy-1>-1) {
			*cursy-=1;
		} else {
			*cursy = m_items-1;
		}
	}
}

void upgrade_armor(struct Unit *unit) { // yikes
	TCOD_key_t skey;
	int cursx = 0;
	int cursy = 0;
	TCOD_console_clear(NULL);
	for (int i=0; i<atypes; i++) {
		TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, "%s: %dd%d +%dCR -%dE -%dR -%dM -%dS -%dP", armor_types[i].name, armor_types[i].acdie, armor_types[i].acsides, armor_types[i].crit_res, armor_types[i].ev_pen, armor_types[i].melee_pen, armor_types[i].acc_pen, armor_types[i].stealth_pen, armor_types[i].per_pen);
	}
	TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", unit->exp, armor_types[cursy].exp);
	print_curs(cursx, cursy);
	TCOD_console_flush();
	skey = TCOD_console_wait_for_keypress(true);
	while(!(skey.vk == TCODK_ESCAPE)) {
		TCOD_console_clear(NULL);
		inc_curs(skey, &cursx, &cursy, atypes);
		print_curs(cursx, cursy);
		for (int i=0; i<atypes; i++) {
			TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, "%s: %dd%d +%dCR -%dE -%dR -%dM -%dS -%dP", armor_types[i].name, armor_types[i].acdie, armor_types[i].acsides, armor_types[i].crit_res, armor_types[i].ev_pen, armor_types[i].melee_pen, armor_types[i].acc_pen, armor_types[i].stealth_pen, armor_types[i].per_pen);
		}
		TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", unit->exp, armor_types[cursy].exp);
		TCOD_console_flush();
		if (skey.vk==TCODK_ENTER && unit->exp >= armor_types[cursy].exp) {
			TCOD_console_clear(NULL);
			TCOD_console_print_rect(NULL, 2, 0, MAPX+HUDX, MAPY+MSGY, "Craft %s? (Enter to confirm)", armor_types[cursy].name);
			TCOD_console_flush();
			skey = TCOD_console_wait_for_keypress(true);
			if (skey.vk==TCODK_ENTER && armor_types[cursy].type == 'h') {
				player.h_arm=armor_types[cursy];
				unit->exp-=armor_types[cursy].exp;
			} else if (skey.vk==TCODK_ENTER && armor_types[cursy].type == 'b') {
				player.b_arm=armor_types[cursy];
				unit->exp-=armor_types[cursy].exp;
			} else if (skey.vk==TCODK_ENTER && armor_types[cursy].type == 'a') {
				player.a_arm=armor_types[cursy];
				unit->exp-=armor_types[cursy].exp;
			} else if (skey.vk==TCODK_ENTER && armor_types[cursy].type == 'l') {
				player.l_arm=armor_types[cursy];
				unit->exp-=armor_types[cursy].exp;
			}
			return;
		}
		skey = TCOD_console_wait_for_keypress(true);
	}
}

bool rest() {
	if (nunits > 0 || (player.chp >= player.mhp && player.cpow >= player.mpow) ) {
		return false;
	}
	while (nunits == 0 && (player.chp < player.mhp || player.cpow < player.mpow)) {
		run_turn();
	}
	return true;
}

bool is_in(int num, int asize, int arr[asize]) { // feel like this is reinventing the wheel but it took 10 seconds so eh
	for (int i=0; i<asize; i++) {
		if (num == arr[i]) {
			return true;
		}
	}
	return false;
}

void wep_mods(struct Weapon *wep, struct Unit *unit) {
	TCOD_key_t skey;
	skey.vk = TCODK_BACKSPACE;
	int cursx = 0; int cursy = 0;
	bool picked = false;
	while (skey.vk != TCODK_ESCAPE) {
		TCOD_console_clear(NULL);
		int j = 0;
		for (int i=0; i<mtypes; i++) {
			if 
			(mod_types[i].is_ranged == wep->is_ranged && 
			// 0 = one handed or two, 1 = one handed, 2 = 2 handed
			(mod_types[i].handedness == 0 || mod_types[i].handedness == wep->handedness) &&
			// show only mods that can be installed multiple times or aren't installed
			(mod_types[i].dupes || !is_in(mod_types[i].id+1,MAXMODS,wep->inmods)) &&
			!(mod_types[i].AP > 0 && wep->AP == 2)
			) {
				TCOD_console_print_rect(NULL, 2, j, MAPX+HUDX, MAPY+MSGY, "%s [%d, (%dd%d)] [AP:%d, critmod:%d]", mod_types[i].name, mod_types[i].acc, mod_types[i].dmgdie, mod_types[i].dmgsides, mod_types[i].AP, mod_types[i].critmod);
				if (picked && cursy == j && unit->exp >= mod_types[i].exp) {
					if (wep->projpershot > 1) { // properly account for shotguns w/ FA
						wep->dmgdie += wep->projpershot * mod_types[i].dmgdie;
					} else {
						wep->dmgdie += mod_types[i].dmgdie;
					}
					wep->dmgsides = wep->dmgsides + mod_types[i].dmgsides;
					wep->AP = wep->AP + mod_types[i].AP;
					wep->bipod = mod_types[i].bipod;
					wep->elec = mod_types[i].elec;
					wep->suppressed = mod_types[i].suppressed;
					wep->acc = wep->acc + mod_types[i].acc;
					wep->inmods[wep->nmods] = mod_types[i].id+1; // zero filled on creation
					wep->nmods = wep->nmods + 1;
					unit->exp = unit->exp - mod_types[i].exp;
					return;
				}
				j++;
			}
		}
		TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", player.exp, mod_types[cursy].exp);
		print_curs(cursx, cursy);
		TCOD_console_flush();
		skey = TCOD_console_wait_for_keypress(true);
		if (skey.vk == TCODK_ENTER) {
			picked = true;
		}
		inc_curs(skey, &cursx, &cursy, j);
	}
}

bool view_weps(bool mod, struct Unit *unit) {
	TCOD_key_t skey;
	skey.vk = TCODK_BACKSPACE;
	int cursx = 0; int cursy = 0;
	bool picked = false;
	while (skey.vk != TCODK_ESCAPE) {
		struct Gear *tmp = player.gear;
		TCOD_console_clear(NULL);
		int i = 0;
		while(tmp != NULL) {
			TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, "%s [%d, (%dd%d)]", tmp->wep.name, tmp->wep.acc, tmp->wep.dmgdie, tmp->wep.dmgsides);
			if (picked && cursy == i) {
				if (mod) {
					wep_mods(&(tmp->wep), unit);
					if (tmp->wep.is_ranged) {
						unit->rwep = tmp->wep;
					} else {
						unit->mwep = tmp->wep;
					}
				} else if (tmp->wep.is_ranged && !mod) {
					player.rwep = tmp->wep;
					if (player.wep.is_ranged) {
						player.wep = player.rwep;
					}
				} else if (!mod) {
					player.mwep = tmp->wep;
					if (!player.wep.is_ranged) {
						player.wep = player.mwep;
					}
				}				
				return true;
				
			}
			i++;
			tmp = tmp->next;
		}
		TCOD_console_print_rect(NULL, 2, i+1, MAPX+HUDX, MAPY+MSGY, "[m] Switch default attack to melee");
		TCOD_console_print_rect(NULL, 2, i+2, MAPX+HUDX, MAPY+MSGY, "[r] Switch default attack to ranged");
		inc_curs(skey, &cursx, &cursy, i);
		print_curs(cursx, cursy);
		TCOD_console_flush();
		skey = TCOD_console_wait_for_keypress(true);
		if (skey.vk == TCODK_ENTER) {
			picked = true;
		} else if (skey.c == 'm') {
			player.wep = player.mwep;
			return false;
		} else if (skey.c == 'r') {
			player.wep = player.rwep;
			return false;
		}
	}
	return false;
}

bool craft_wep(struct Unit *unit) {
	TCOD_key_t skey; skey.vk = TCODK_ENTER;
	int cursx = 0; int cursy = 0;
	while (skey.vk != TCODK_ESCAPE) {
		TCOD_console_clear(NULL);
		for (int i=0; i<wtypes; i++) {
			TCOD_console_print_rect(NULL, 2, i, MAPX+HUDX, MAPY+MSGY, "%s [%d, (%dd%d)]", wep_types[i].name, wep_types[i].acc, wep_types[i].dmgdie, wep_types[i].dmgsides);
		}
		TCOD_console_print_rect(NULL, 0, 31, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", unit->exp, wep_types[cursy].exp); //bug here
		inc_curs(skey, &cursx, &cursy, wtypes);
		print_curs(cursx, cursy);
		TCOD_console_flush();
		skey = TCOD_console_wait_for_keypress(true);
		if (skey.vk == TCODK_ENTER) {
			struct Gear *nwep = malloc(sizeof(struct Gear));
			nwep->next = player.gear;
			nwep->wep = wep_types[cursy];
			player.gear = nwep;
			unit->exp -= nwep->wep.exp;
			return true;
		}
	}
	return false;
}

bool context(TCOD_key_t key) {
	if (map[player.x][player.y].down_stair) {
		return gen_new_level();
	} else if (map[player.x][player.y].door) {
		map[player.x][player.y].is_transparent = !map[player.x][player.y].is_transparent;
		if (map[player.x][player.y].c == '#') map[player.x][player.y].c = 'o';
		else map[player.x][player.y].c = '#';
		gen_tmap();
		return true;
	} else if (map[player.x][player.y].med) {
		map[player.x][player.y].med = false;
		map[player.x][player.y].r = 55;
		map[player.x][player.y].g = 55;
		map[player.x][player.y].g = 55;
		player.chp = player.mhp*2;
		return true;
	} else if (map[player.x][player.y].power) {
		map[player.x][player.y].power = false;
		map[player.x][player.y].r = 55;
		map[player.x][player.y].g = 55;
		map[player.x][player.y].g = 55;
		player.cpow = player.mpow*2;
		return true;
	} else if (map[player.x][player.y].printer) {
		int cursx = 0; int cursy = 0;
		struct Unit equip; equip.exp = 500;
		char *strings[NUMITEMS] = {"Medigel:%d", "Stim Pack:%d", "Power Cell:%d", "Berkserker Cell:%d", "Smoke Grenades:%d", "Glue Grenades:%d", "Paint Grenades:%d", "Barrier Grenades:%d", "Frag Grenades:%d", "Rockets:%d", "Drill Rockets:%d", "Flares:%d", "Noisemakers:%d"};
		int *vals[NUMITEMS] = {&(player.inv.medi), &(player.inv.stim), &(player.inv.pcell), &(player.inv.berserk), &(player.inv.smoke), &(player.inv.glue), &(player.inv.paint), &(player.inv.barrier), &(player.inv.frag), &(player.inv.rockets), &(player.inv.drill), &(player.inv.flare), &(player.inv.noisemaker)};
		menu(NUMITEMS, strings, vals, &equip, true, 500, 0, cursx, cursy, menu_basic);
		if (equip.exp < 500) {
			map[player.x][player.y].printer = false;
			map[player.x][player.y].r = 55;
			map[player.x][player.y].g = 55;
			map[player.x][player.y].b = 55;
			return true;
		} else {
			return false;
		}
	} else if (map[player.x][player.y].forge) {
		struct Unit equip; equip.exp = 1000;
		int cursx = 0; int cursy = 0;
		char *strings[3] = {"Craft Weapons", "Upgrade Weapons", "Upgrade Suit"};
		int null = 1; int *vals[3] = {&null, &null, &null};
		menu(3, strings, vals, &equip, false, 0, 0, cursx, cursy, menu_forge);
		if (equip.exp < 1000) {
			map[player.x][player.y].forge = false; 
			map[player.x][player.y].r = 55;
			map[player.x][player.y].g = 55;
			map[player.x][player.y].b = 55;
			return true;
		} else {
			return false;
		}
	} else if (player.activeAbilities1 & A1_PLASMA_CUTTER && player.cpow >= 5) {
		key = TCOD_console_wait_for_keypress(true);
		int ndx = 0; int ndy = 0;
		get_dxdy(key, &ndx, &ndy);
		if (!map[player.x + ndx][player.y + ndy].can_walk) {
			map[player.x + ndx][player.y + ndy].can_walk = true;
			map[player.x + ndx][player.y + ndy].c = '.';
			player.cpow -= 5;
			return true;
		} 
	}
	return false;
}

bool get_move() {
	draw_all();
	TCOD_key_t key = TCOD_console_wait_for_keypress(true);
	int dx = 0; int dy = 0;
	get_dxdy(key, &dx, &dy);
	if ( (dx != 0 || dy != 0) ) {
		if (!key.lctrl) {
			if (player.spos>0) {
				return false;
			} else if (!key.shift) {
				return move_or_attack(&player, dx, dy);
			} else if (key.shift) {
				return dash(&player, dx, dy);
			}
		} else if (key.lctrl) {
			if (!key.shift) {
				if (find_unit_ind(player.x+dx, player.y+dy) > -1) {
					return punt(&player, dx, dy, 6); // Omae wa mou shindeiru
				} else if (!map[player.x+dx][player.y+dy].door && camo(&player)) {
					return move_or_attack(&player, dx, dy);
				} else if (map[player.x+dx][player.y+dy].door) {
					return weld_door(&player, dx, dy);
				}
			} else if (key.shift) {
				return leap(&player, dx, dy);
			}
		}
	} else if ( (key.vk==TCODK_KP5 || key.c=='.') ) {
		player.lt_dx = 0; player.lt_dy = 0;
		if (!key.shift && !key.lctrl) {
			return true;
		} else if (key.shift && !key.lctrl) {
			if (player.activeAbilities1 & A1_KNEELING_POSITION) {
				if (player.spos == 1) {
					player.spos = 0;
					char dmsg[MSGSIZE];
					snprintf(dmsg, MSGSIZE, "You stand up.");
					add_msg(dmsg);
					return false;
				} else {
					player.spos = 1;
					char dmsg[MSGSIZE];
					snprintf(dmsg, MSGSIZE, "You assume a kneeling shooting position.");
					add_msg(dmsg);
				}
			}
		} else if (!key.shift && key.lctrl) {
			if (player.activeAbilities1 & A1_PRONE_POSITION) {
				if (player.spos == 2) {
					player.spos = 0;
					char dmsg[MSGSIZE];
					snprintf(dmsg, MSGSIZE, "You stand up.");
					add_msg(dmsg);
					return true;
				} else {
					player.spos = 2;
					char dmsg[MSGSIZE];
					snprintf(dmsg, MSGSIZE, "You assume a prone shooting position.");
					add_msg(dmsg);
				}
				return true;
			}
		}
	} else if (key.c=='r') {
		return rest();
	} else if (key.c=='a') {
		return select_item(&player);;
	} else if (key.c=='f') {
		return shoot(&player, key);
	} else if (key.c=='v') {
		return look();
	} else if (key.c=='c') {
		int cursx = 0; int cursy = 0;
		char *strings[6] = {"Craft Weapons", "Craft Items", "Upgrade Weapons", "Upgrade Suit", "Learn Skills", "Learn Abilities"};
		int null = 1; int *vals[6] = {&null, &null, &null, &null, &null, &null};
		menu(6, strings, vals, &player, false, 0, 0, cursx, cursy, menu_char);
	} else if (key.c=='w') {
		return view_weps(false, &player);
	} else if (key.c=='x') {
		return context(key);
	} else if (key.vk==TCODK_ESCAPE) {
		int cursx = 0; int cursy = 0;
		char *strings[5] = {"Return to game.", "Save current game (not fully implemented yet).", "Load saved game (not fully implemented yet).", "Start new game.", "Exit game."};
		int null = 1; int *vals[5] = {&null, &null, &null, &null, &null};
		struct Unit gmenu; gmenu.noescape = true;
		menu(5, strings, vals, &gmenu, false, 0, 0, cursx, cursy, menu_5game);
	} else if (key.c=='/' && key.shift) {
		TCOD_console_clear(NULL);
		TCOD_console_print_rect(NULL, 2, 0, MAPX+HUDX, MAPY+MSGY, "%s", "[Esc] for main menu");
		TCOD_console_print_rect(NULL, 2, 1, MAPX+HUDX, MAPY+MSGY, "%s", "Arrow Keys/Numpad/Vi Keys for movement (Shift/LControl alters left/right arrow to diagonal)");
		TCOD_console_print_rect(NULL, 2, 2, MAPX+HUDX, MAPY+MSGY, "%s", "[.]/[5] on Numpad to wait");
		TCOD_console_print_rect(NULL, 2, 3, MAPX+HUDX, MAPY+MSGY, "%s", "[x] to perform context sensitive action (descend stairs, close/unclose doors)");
		TCOD_console_print_rect(NULL, 2, 4, MAPX+HUDX, MAPY+MSGY, "%s", "[r] to rest");
		TCOD_console_print_rect(NULL, 2, 5, MAPX+HUDX, MAPY+MSGY, "%s", "[a] to apply item");
		TCOD_console_print_rect(NULL, 2, 6, MAPX+HUDX, MAPY+MSGY, "%s", "[f] to fire currently equipped ranged weapon");
		TCOD_console_print_rect(NULL, 2, 7, MAPX+HUDX, MAPY+MSGY, "%s", "[w] to change equipped weapon or default attack");
		TCOD_console_print_rect(NULL, 2, 8, MAPX+HUDX, MAPY+MSGY, "%s", "[v] to view enviroment");
		TCOD_console_print_rect(NULL, 2, 9, MAPX+HUDX, MAPY+MSGY, "%s", "[c] to upgrade character");
		TCOD_console_print_rect(NULL, 2, 11, MAPX+HUDX, MAPY+MSGY, "%s", "Movement Keys and Enter/Escape to navigate menus");
		TCOD_console_flush();
		TCOD_console_wait_for_keypress(true);
	}
	return false;
}

void add_msg(char msg[MSGSIZE]) {
	if (msgs_s == MAXMSGS) {
		for (int i=0; i<MAXMSGS; i++) {
			memmove(msgs[i], msgs[i+1], MSGSIZE);
			memmove(msgs[msgs_s], msg, MSGSIZE);
		}
	} else {
		memmove(msgs[msgs_s], msg, MSGSIZE);
		msgs_s+=1;
	}
}

void start_game() {
	turn = 0; 
	depth = 1;
	player = splayer; //wrong scope?
	gen_all();
	msgs_s = 0; 
	print_msg(smsgs[1]);
	draw_all();
}

void print_msg(char p_msg[MSGSIZE]) {
	TCOD_console_clear(NULL);
	TCOD_console_print_rect(NULL, 0, 0, MAPX+HUDX, MAPY+MSGY, "%s", p_msg);
	TCOD_console_flush();
	TCOD_console_wait_for_keypress(true);
}

void save_units() {
	FILE *usave = fopen("units.save", "w");
	if (usave != NULL) {
		fwrite(&unit_list, sizeof(struct Unit), nunits, usave);
		printf("Successfully saved unit data.\n");
	} else {
		printf("Error opening file.\n");
	}
	fclose(usave);
	
	FILE *psave = fopen("player.save", "w");
	if (psave != NULL) {
		fwrite(&player, sizeof(struct Unit), 1, psave);
		printf("Successfully saved player data.\n");
	} else {
		printf("Error opening file.\n");
	}
	fclose(psave);
	
	FILE *msave = fopen("map.save", "w");
	if (msave != NULL) {
		fwrite(&map, sizeof(struct Tile)*MAPX*MAPY, MAPX*MAPY, msave);
		printf("Successfully saved map data.\n");
	} else {
		printf("Error opening file.\n");
	}
	fclose(msave);
	
	FILE *tmsave = fopen("tmap.save", "w");
	if (tmsave != NULL) {
		fwrite(&tmap, sizeof(TCOD_map_t), 1, tmsave);
		printf("Successfully saved tmap data.\n");
	} else {
		printf("Error opening file.\n");
	}
	fclose(tmsave);
	
	FILE *usaver = fopen("units.save", "r");
	struct Unit ul2[MAXUNITS];
	fread(&ul2, sizeof(struct Unit), nunits, usaver);
	int u; 
	for (u=0; u<MAXUNITS; u++) {
		unit_list[u] = ul2[u];
	}
	fclose(usaver);
	
	FILE *psaver = fopen("player.save", "r");
	struct Unit ps;
	fread(&ps, sizeof(struct Unit), 1, psaver);
	fclose(psaver);
	player.c = '*';
	player = ps;
	
	int i;
	for (i=0; i<nunits; i++) {
		unit_list[i] = ul2[i];
		printf("%s %s", unit_list[i].name, ul2[i].name);
		printf("1 \n");
	}
	printf("%s \n", ps.name);
	
	FILE *msaver = fopen("map.save", "r");
	struct Tile map2[MAPX][MAPY];
	fread(&map2, sizeof(struct Tile)*MAPX*MAPY, MAPX*MAPY, msaver);
	fclose(msaver);
	int x;
	int y;
	/*
	for (x=0; x<MAPX; x++) {
		for (y=0; y<MAPY; y++) {
			map[x][y] = map2[x][y];
		}
	}
	*/
	
	FILE *tmsaver = fopen("tmap.save", "r");
	TCOD_map_t tmap2;
	fread(&tmap2, sizeof(TCOD_map_t), 1, tmsave);
	fclose(tmsaver);
	tmap = tmap2;
	
}


void load_player(char *key, char *val, int *sui, int *j, int strsize) {
	if (strcmp(key,"x") == 0) {
		player.x = atoi(val);
	} else if (strcmp(key,"y") == 0) {
		player.y = atoi(val);
	} else if (strcmp(key,"lt_dx") == 0) {
		player.lt_dx = atoi(val);
	} else if (strcmp(key,"lt_dy") == 0) {
		player.lt_dy = atoi(val);
	} else if (strcmp(key,"mhp") == 0) {
		player.mhp = atoi(val);
	} else if (strcmp(key,"chp") == 0) {
		player.chp = atoi(val);
	} else if (strcmp(key,"mpow") == 0) {
		player.mpow = atoi(val);
	} else if (strcmp(key,"cpow") == 0) {
		player.cpow = atoi(val);
	} else if (strcmp(key,"ev") == 0) {
		player.ev = atoi(val);
	} else if (strcmp(key,"melee") == 0) {
		player.melee = atoi(val);
	} else if (strcmp(key,"ranged") == 0) {
		player.ranged = atoi(val);
	} else if (strcmp(key,"per") == 0) {
		player.per = atoi(val);
	} else if (strcmp(key,"stealth") == 0) {
		player.stealth = atoi(val);
	} else if (strcmp(key,"inv.medi") == 0) {
		player.inv.medi = atoi(val);
	} else if (strcmp(key,"inv.stim") == 0) {
		player.inv.stim = atoi(val);
	} else if (strcmp(key,"inv.pcell") == 0) {
		player.inv.pcell = atoi(val);
	} else if (strcmp(key,"inv.berserk") == 0) {
		player.inv.berserk = atoi(val);
	} else if (strcmp(key,"inv.smoke") == 0) {
		player.inv.smoke = atoi(val);
	} else if (strcmp(key,"inv.glue") == 0) {
		player.inv.glue = atoi(val);
	} else if (strcmp(key,"inv.paint") == 0) {
		player.inv.paint = atoi(val);
	} else if (strcmp(key,"inv.barrier") == 0) {
		player.inv.barrier = atoi(val);
	} else if (strcmp(key,"inv.frag") == 0) {
		player.inv.frag = atoi(val);
	} else if (strcmp(key,"inv.rockets") == 0) {
		player.inv.rockets = atoi(val);
	} else if (strcmp(key,"inv.drill") == 0) {
		player.inv.drill = atoi(val);
	} else if (strcmp(key,"inv.flare") == 0) {
		player.inv.flare = atoi(val);
	}
}

void load_units(char *key, char *val, int *sui, int *j, int strsize) {
	if (strcmp(key,"unit_num") == 0) {
		unit_num = atoi(val);
	}
	
	if (strcmp(key,"x") == 0) {
		unit_list[*sui].x = atoi(val);
	} else if (strcmp(key,"y") == 0) {
		unit_list[*sui].y = atoi(val);
	} else if (strcmp(key,"name") == 0) {
		memmove(unit_list[*sui].name, val, strsize);
	} else if (strcmp(key,"desc") == 0) {
		memmove(unit_list[*sui].desc, val, strsize);
	} else if (strcmp(key,"c") == 0) {
		unit_list[*sui].c = val[0];
	} else if (strcmp(key,"fac") == 0) { 
		unit_list[*sui].fac = atoi(val);
	} else if (strcmp(key,"mindepth") == 0) {
		unit_list[*sui].mindepth = atoi(val);
	} else if (strcmp(key,"maxdepth") == 0) {
		unit_list[*sui].maxdepth = atoi(val);
	} else if (strcmp(key,"spawnweight") == 0) {
		unit_list[*sui].spawnweight = atof(val);
	} else if (strcmp(key,"r") == 0) {
		unit_list[*sui].r = atof(val);
	} else if (strcmp(key,"g") == 0) {
		unit_list[*sui].g = atof(val);
	} else if (strcmp(key,"b") == 0) {
		unit_list[*sui].b = atof(val);	
	} else if (strcmp(key,"mhp") == 0) {
		unit_list[*sui].mhp = atoi(val);
	} else if (strcmp(key,"chp") == 0) {
		unit_list[*sui].chp = atoi(val);
	} else if (strcmp(key,"ev") == 0) {
		unit_list[*sui].ev = atoi(val);
	} else if (strcmp(key,"fov") == 0) {
		unit_list[*sui].fov = atoi(val);
	} else if (strcmp(key,"b_arm.acdie") == 0) {
		unit_list[*sui].b_arm.acdie = atoi(val);
	} else if (strcmp(key,"b_arm.acsides") == 0) {
		unit_list[*sui].b_arm.acsides = atoi(val);
	} else if (strcmp(key,"atk_skill") == 0) {
		unit_list[*sui].melee = atoi(val);
		unit_list[*sui].ranged = atoi(val);
	} else if (strcmp(key,"wep.is_ranged") == 0) {
		unit_list[*sui].wep.is_ranged = atoi(val);
	} else if (strcmp(key,"wep.dmgdie") == 0) {
		unit_list[*sui].wep.dmgdie = atoi(val);
	} else if (strcmp(key,"wep.dmgsides") == 0) {
		unit_list[*sui].wep.dmgsides = atoi(val);
	} else if (strcmp(key,"per") == 0) {
		unit_list[*sui].per = atoi(val);
	} else if (strcmp(key,"stealth") == 0) {
		unit_list[*sui].stealth = atoi(val);
	} else if (strcmp(key,"lit") == 0) {
		unit_list[*sui].lit = atoi(val);
		*sui = *sui + 1;
	}
}

void load_map(char *key, char *val, int *smi, int *smj, int strsize) {
	if (strcmp(key,"depth") == 0) {
		depth = atoi(val);
	} else if (strcmp(key,"turn") == 0) {
		turn = atoi(val);
	}

	if (strcmp(key,"i") == 0) {
		*smi = atoi(val);
	} else if (strcmp(key,"j") == 0) {
		*smj = atoi(val);
	} else if (strcmp(key,"c") == 0) {
		map[*smi][*smj].c = val[0];
	} else if (strcmp(key,"r") == 0) {
		map[*smi][*smj].r = atoi(val);
	} else if (strcmp(key,"g") == 0) {
		map[*smi][*smj].g = atoi(val);
	} else if (strcmp(key,"b") == 0) {
		map[*smi][*smj].b = atoi(val);
	} else if (strcmp(key,"can_walk") == 0) {
		map[*smi][*smj].can_walk = atoi(val);
	} else if (strcmp(key,"is_transparent") == 0) {
		map[*smi][*smj].is_transparent = atoi(val);
	} else if (strcmp(key,"filled") == 0) {
		map[*smi][*smj].filled = atoi(val);
	} else if (strcmp(key,"is_explored") == 0) {
		map[*smi][*smj].is_explored = atoi(val);
	} else if (strcmp(key,"lit") == 0) {
		map[*smi][*smj].tlit = atoi(val);
	} else if (strcmp(key,"door") == 0) {
		map[*smi][*smj].door = atoi(val);
	} else if (strcmp(key,"down_stair") == 0) {
		map[*smi][*smj].down_stair = atoi(val);
	} else if (strcmp(key,"terminal") == 0) {
		map[*smi][*smj].terminal = atoi(val);
	} else if (strcmp(key,"med") == 0) {
		map[*smi][*smj].med = atoi(val);	
	} else if (strcmp(key,"power") == 0) {
		map[*smi][*smj].power = atoi(val);	
	} else if (strcmp(key,"printer") == 0) {
		map[*smi][*smj].printer = atoi(val);	
	} else if (strcmp(key,"effect") == 0) {
		map[*smi][*smj].effect = atoi(val);	
	} else if (strcmp(key,"teffect") == 0) {
		map[*smi][*smj].teffect = atoi(val);	
	} else if (strcmp(key,"cover") == 0) {
		map[*smi][*smj].cover = atoi(val);
	} else if (strcmp(key,"concealment") == 0) {
		map[*smi][*smj].concealment = atoi(val);
	} else if (strcmp(key,"name") == 0) {
		memmove(map[*smi][*smj].name, val, strsize);
	} else if (strcmp(key,"desc") == 0) {
		memmove(map[*smi][*smj].desc, val, strsize);
	}
}

void printvals(char *key, char *val, int i, int j) {
	printf("i: %d key: %s val:%s\n", i, key, val);
}

void gstart_story(char *key, char *val, int *i, int *j, int bsize) {
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"title") == 0) {
		memmove(smsgs[0], buffer, bsize); 
	} else if (strcmp(key,"omsg") == 0) {
		memmove(smsgs[1], buffer, bsize); 
	} else if (strcmp(key,"dmsg") == 0) {
		memmove(smsgs[2], buffer, bsize);
	} else if (strcmp(key,"emsg1") == 0) {
		memmove(smsgs[3], buffer, bsize);
	} else if (strcmp(key,"emsg2") == 0) {
		memmove(smsgs[4], buffer, bsize);
	} else if (strcmp(key,"emsg3") == 0) {
		memmove(smsgs[5], buffer, bsize);
	} else if (strcmp(key,"vmsg1") == 0) {
		memmove(smsgs[6], buffer, bsize);
	} else if (strcmp(key,"vmsg2") == 0) {
		memmove(smsgs[7], buffer, bsize);
	} else if (strcmp(key,"vmsg3") == 0) {
		memmove(smsgs[8], buffer, bsize);
	}
}

void gstart_player(char *key, char *val, int *i, int *j, int bsize) {
	printf("key: %s val: %s\n", key, val);
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(player.name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"desc") == 0) {
		memmove(player.desc, buffer, bsize);
	} else if (strcmp(key,"c") == 0) {
		player.c = val[0];
	} else if (strcmp(key,"fac") == 0) { 
		player.fac = atoi(val);
	} else if (strcmp(key,"exp") == 0) { 
		player.exp = atoi(val);
	} else if (strcmp(key,"uhp") == 0) {
		player.uhp = atoi(val);
	} else if (strcmp(key,"mhp") == 0) {
		player.mhp = atoi(val);
	} else if (strcmp(key,"chp") == 0) {
		player.chp = atoi(val);
	} else if (strcmp(key,"mpow") == 0) {
		player.mpow = atoi(val);
	} else if (strcmp(key,"cpow") == 0) {
		player.cpow = atoi(val);
	} else if (strcmp(key,"inv.medi") == 0) {
		player.inv.medi = atoi(val);
	} else if (strcmp(key,"inv.stim") == 0) {
		player.inv.stim = atoi(val);
	} else if (strcmp(key,"inv.pcell") == 0) {
		player.inv.pcell = atoi(val);
	} else if (strcmp(key,"inv.smoke") == 0) {
		player.inv.smoke = atoi(val);
	} else if (strcmp(key,"inv.glue") == 0) {
		player.inv.glue = atoi(val);
	} else if (strcmp(key,"inv.paint") == 0) {
		player.inv.paint = atoi(val);
	} else if (strcmp(key,"inv.barrier") == 0) {
		player.inv.barrier = atoi(val);
	} else if (strcmp(key,"inv.frag") == 0) {
		player.inv.frag = atoi(val);
	} else if (strcmp(key,"inv.rockets") == 0) {
		player.inv.rockets = atoi(val);
	} else if (strcmp(key,"inv.drill") == 0) {
		player.inv.drill = atoi(val);
	} else if (strcmp(key,"ev") == 0) {
		player.ev = atoi(val);
	} else if (strcmp(key,"b_arm.name") == 0) {
		memmove(player.b_arm.name, buffer, bsize);
	} else if (strcmp(key,"b_arm.acdie") == 0) {
		player.b_arm.acdie = atoi(val);
	} else if (strcmp(key,"b_arm.acsides") == 0) {
		player.b_arm.acsides = atoi(val);
	} else if (strcmp(key,"melee") == 0) {
		player.melee = atoi(val);
	} else if (strcmp(key,"ranged") == 0) {
		player.ranged = atoi(val);
	} else if (strcmp(key,"fov") == 0) {
		player.fov = atoi(val);
	} else if (strcmp(key,"per") == 0) {
		player.per = atoi(val);
	} else if (strcmp(key,"stealth") == 0) {
		player.stealth = atoi(val);
	}
}

void read_json(char *filename, int strsize, void (*callback)(char *, char *, int *, int *, int) ) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("read_json() error: file not found\n");
		return;
	}
	int aps = 0;
	int i = 0;
	int j = 0;
	char key[strsize];
	int ki = 0;
	char val[strsize];
	int vi = 0;
	char c;
	while((c = fgetc(file)) != EOF) {
		if (c != '\"') {
			if (aps == 1) {
				key[ki] = c;
				ki++;
			} else if (aps == 3) {
				val[vi] = c;
				vi++;
			} else if (aps == 4) {
				key[ki] = '\0';
				val[vi] = '\0';
				vi = 0;
				ki = 0;
				callback(key, val, &i, &j, strsize);
				aps = 0;
				key[0] = '\0';
				val[0] = '\0';
			}
		} else if (c == '\"') { 
			aps++;
		}
	}
}

void load_game() {
	for (int i=0; i<unit_num; i++) {
		unit_list[i].x = 0;
		unit_list[i].y = 0;
	}
	gen_json_lists();
	read_json("../save/map.json", 256, load_map);
	read_json("../save/units.json", 256, load_units);
	read_json("../save/player.json", 256, load_player);
	gen_tmap();
}

void run_turn() {
	draw_all();
	ai_turn(); 
	tile_effects();
	dec_pow(&player);
	turn+=1;
}

int main() {
	utypes = 0;
	wtypes = 0;
	gen_json_lists();
	splayer = player;
	
	srand(time(0));
	TCOD_console_init_root(MAPX+HUDX, MAPY+MSGY, smsgs[0], false, TCOD_RENDERER_SDL);
	int cursx = 0; int cursy = 0;
	char *strings[3] = { "Start new game.", "Load saved game.", "Exit game."};
	int null = 1; int *vals[3] = {&null, &null, &null};
	struct Unit gmenu; gmenu.noescape = true;
	menu(3, strings, vals, &gmenu, false, 0, 0, cursx, cursy, menu_3game);
	char dmsg[MSGSIZE];
	snprintf(dmsg, MSGSIZE, "Press '?' for keybindings. Find the down stairs (>).");
	add_msg(dmsg);
	while ( !TCOD_console_is_window_closed() ) {
		draw_all();
		while (!get_move()) {}
		run_turn();
	}
}
