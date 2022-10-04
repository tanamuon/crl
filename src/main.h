#ifndef MAIN_H
#define MAIN_H

#include "../include/libtcod.h"
#include "system.h"
#include "units.h"
#include "map.h"

extern int last_hit;
extern int utypes;
extern struct Unit *unit_types;
extern int wtypes;
extern struct Weapon *wep_types;
extern int mtypes;
extern struct Weapon *mod_types;
extern int atypes;
extern struct Armor *armor_types;
extern struct Unit unit_list[MAXUNITS];
extern struct Unit player;
extern int unit_num;
extern struct Tile map[MAPX][MAPY];
extern TCOD_map_t tmap;
extern int turn;
extern int depth;
extern char msgs[MAXMSGS][MSGSIZE];
extern char smsgs[STORYMSGS][MSGSIZE];
extern int msgs_s;
extern int units_in_view[MAXUNITS]; // this gives the unit_list index of all units in player fov
extern int nunits;
extern struct Unit splayer;
extern int ndoors;
extern int doorsxy[MAPX*MAPY][2];


extern void run_turn();
extern void menu(int n, char **strings, int **val, struct Unit *unit, bool xpreq, int xpflat, int xpfac, int cx, int cy, bool callback(int *, int *, struct Unit *, TCOD_key_t, int, int));
extern bool menu_3game(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy);
extern void upgrade_armor(struct Unit *unit);
extern bool view_weps(bool mod, struct Unit *unit);
extern bool craft_wep(struct Unit *unit);
extern void save_units();
extern void draw_all();
extern void add_msg(char msg[MSGSIZE]);
extern void welcome_msg();
extern void start_game();
extern bool rest();
extern bool char_menu(bool printer);
extern bool game_menu(bool start);
extern int victory_menu();
extern void draw_curs(int curs);
extern void draw_gmenu();
extern void get_dxdy(TCOD_key_t key, int *dx, int *dy);
extern void print_msg(char p_msg[MSGSIZE]);
extern void print_curs(int cursx, int cursy);
extern void inc_curs(TCOD_key_t skey, int *cursx, int *cursy, int m_items);
extern bool target(int *x, int *y, int r);

#endif
