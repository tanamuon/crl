#include "ai.h"
#include "system.h"
#include "units.h"
#include "items.h"
#include "skills.h"
#include "main.h"
#include "../include/libtcod.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

bool move_towards(struct Unit *mover, int x, int y) {
	TCOD_map_compute_fov(tmap, mover->x, mover->y, mover->fov, true, FOV_PERMISSIVE_8);
	TCOD_path_t path = TCOD_path_new_using_map(tmap, 1.0f);
	TCOD_path_compute(path, mover->x, mover->y, x, y);
	if (TCOD_path_size(path) == 0) {
		printf("destination reached\n");
		mover->dest = false;
		return false;
		if (map[mover->x][mover->y].door) printf("door reached\n");
	} else if (!TCOD_path_is_empty(path)) { // ai pathfinding improvements go here
		//dumb hack to pick which enemy keeps its original destination in the event of a pileup
		if (rand() % 2 > 0) {
			mover->dest = false;
			return false;
		}
	}
	int derpx = 0, derpy = 0;
	if(!TCOD_path_walk(path, &derpx, &derpy, true) && TCOD_map_is_in_fov(tmap, player.x, player.y)) { // BOY this is not a good function
		printf("path walk failed\n");
	}
	if (derpx == 0 && derpy == 0) return false;
	int dx = derpx - mover->x; 
	int dy = derpy - mover->y;
	mover->turnspent = true;
	move_or_attack(mover, dx, dy);
	return true;
}

bool check_adjacent(int x1, int y1, int x2, int y2) {
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
			if (x1+i == x2 && y1+j == y2) return true;
		}
	}
	return false;
}

int * check_surround(struct Unit defender, struct Unit attacker) {
	static int surround[3] = {0, 0, 0};
	int xa = 0; int ya = 0;
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
			xa = defender.x + i;
			ya = defender.y + j;
			if (map[xa][ya].can_walk && find_unit_ind(xa, ya) == -1 && check_adjacent(xa, ya, attacker.x, attacker.y)) {
				surround[0] = 1;
				surround[1] = xa;
				surround[2] = ya;
				return surround;
			}
		}
	}
	return surround;
}

int max(int a, int b) {
	if (a < b) {
		return b;
	}
	return a;
}

int get_nearest_door(int x, int y, int *di, int dx, int dy) {
	int ldist = MAPY;
	int dist = MAPY;
	for (int i=0; i<ndoors; i++) {
		dist = max( abs(x - doorsxy[i][0]), abs(y - doorsxy[i][1]) );
		if ( dist < ldist && doorsxy[i][0]*dx >= 0 && doorsxy[i][1]*dy >= 0) {
			ldist = dist;
			*di = i;
		}
	}
	return ldist;
}

bool dof_limit_check(struct Unit defender, struct Unit attacker, int *x, int *y) {
	int dof = 0;
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
			if (map[defender.x+i][defender.y+j].can_walk && find_unit_ind(defender.x+i, defender.y+j) == -1  && check_adjacent(attacker.x, attacker.y, defender.x+i, defender.y+j) ) {
				dof++;
				for (int m=0; m<unit_num; m++) {
					if (unit_list[m].x != attacker.x || unit_list[m].y != attacker.y) {
						if (check_adjacent(attacker.x, attacker.y, unit_list[m].x, unit_list[m].y) ) {
							if (!check_adjacent(defender.x, defender.y, unit_list[m].x, unit_list[m].y) ) {
								printf("can make space for ally\n");			
								*x = defender.x+i;
								*y = defender.y+j;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void return_valid_moves(int *n, int vmoves[*n][2], int x, int y) {
	printf("n at beginning:%d",*n);
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
		printf("i:%d, j:%d\n",i,j);
			if (map[x+i][y+j].can_walk && !(i == 0 && j == 0) ) {
				vmoves[*n][0] = x+i;
				vmoves[*n][1] = y+j;
				*n = *n + 1;
			}
		}
	}
	return;
}


bool ai_switch_and_heal(struct Unit *unit) {
	int moves = 0;
	int movelist[moves][2];
	return_valid_moves(&moves, movelist, unit->x, unit->y);
	if (moves == 2 && check_adjacent(player.x, player.y, unit->x, unit->y) && unit->inv.stim > 0) {
		int ind = find_unit_ind(movelist[0][0],movelist[0][1]);
		if (ind < 0) {
			ind = find_unit_ind(movelist[1][0],movelist[1][1]);
		}
		if (ind > -1 && unit_list[ind].regen < 1) {					
			int ax = unit_list[ind].x; 
			int ay = unit_list[ind].y;
			unit_list[ind].x = unit->x;
			unit_list[ind].y = unit->y;
			unit->x = ax;
			unit->y = ay;
			char dmsg1[MSGSIZE];
			snprintf(dmsg1, MSGSIZE, "The %s switches places with the %s.", unit->name, unit_list[ind].name);
			add_msg(dmsg1);
			if (unit->inv.stim > 0) {
				char dmsg2[MSGSIZE];
				snprintf(dmsg2, MSGSIZE, "The %s begins healing its wounds.", unit->name);
				add_msg(dmsg2);
				use_stim(unit);
				unit->inv.stim = unit->inv.stim - 1;
			}
			unit->turnspent = true;
			unit_list[ind].turnspent = true;
			return true;
		}
	}
	return false;
}

bool ai_go_berserk(struct Unit *unit) {
	if (unit->inv.berserk > 0 && unit->berserk < 1 && !unit->wep.is_ranged && check_adjacent(player.x, player.y, unit->x, unit->y)) {
		unit->berserk = 15;
		char dmsg[MSGSIZE];
		snprintf(dmsg, MSGSIZE, "The %s goes berserk.", unit->name);
		add_msg(dmsg);
		unit->turnspent = true;
		return true;
	}
	return false;
}

bool surround_check(int x, int y) {
	for (int i=-1; i<=1; i++) {
		for (int j=-1; j<=1; j++) {
			if (map[x+i][y+j].can_walk && find_unit_ind(x+i, y+j) == -1) {
				return false;
			}
		}
	}
	return true;
}

bool ai_aware_melee(struct Unit *unit) {
	int di = 0; int dj = 0;
	int ei = 0; int ej = 0;
	int udoor = get_nearest_door(unit->x, unit->y, &di, 0, 0);
	int pdoor = get_nearest_door(player.x, player.y, &dj, 0, 0);

	//put this into a lowhp function maybe
	if (unit->chp*4 < unit->mhp) { // this will hopefully nerf hallways a bit
		if (ai_switch_and_heal(unit)) return true;
		if (ai_go_berserk(unit)) return true;
	}
	
	if (!check_adjacent(unit->x, unit->y, player.x, player.y)) {
		int oldx = unit->x; int oldy = unit->y;
		move_towards(unit, player.x, player.y);
		int * surround = check_surround(player, *unit);
		if (oldx == unit->x && oldy == unit->y && surround[0]==1 ) {
			move_towards(unit, surround[1], surround[2]);
		}
	} else {
		for (int i=-1; i<1; i++) {
			for (int j=-1; j<1; j++) {
				if (find_unit_ind(player.x+i, player.y+j) == -1 && map[player.x+i][player.y+j].can_walk) {
					printf("sf ");
				}
			}
		}
		printf("\n");
		if (unit->berserk>0 || !surround_check(player.x, player.y)) {
			printf("player is surrounded, not moving\n");
			take_dmg(unit, &player, unit->wep);
		} else if (pdoor - udoor == 0) {
				move_towards(unit, doorsxy[di][0], doorsxy[di][1]);
				printf("player can be intercepted, moving towards door\n");
			// there should be a ranged enemy that blocks doors
		} else if (dof_limit_check(player, *unit, &ei, &ej)) {
			printf("hideously overcomplicated function not useless\n");
			return move_towards(unit, ei, ej);
		} else {
			return move_towards(unit, player.x, player.y);
		}
	}
	return false;
}

bool ai_blind_check(struct Unit *unit) {
	if (unit->blind) {
		TCOD_map_compute_fov(tmap, unit->x, unit->y, unit->fov, true, FOV_PERMISSIVE_8);
		if (unit->bturns == 1) {
			unit->blind = false;
			unit->bturns = 0;
		} else if (unit->bturns > 1) {
			unit->bturns--;
		}
		unit->aware = false;
		int x = rand()%3 -1;
		int y = rand()%3 -1;
		move_towards(unit, unit->x+x, unit->y+y);
		return true;
	}
	return false;
}

bool ai_stuck_check(struct Unit *unit) {
	if (unit->stuck) {
		if (unit->sturns == 1) {
			unit->stuck = false;
			unit->sturns = 0;
		} else if (unit->sturns > 1) {
			unit->sturns--;
		}
		return true;
	}
	return false;
}

bool ai_player_in_view(struct Unit *unit) {
	unit->hunting = true;
	unit->plastx = player.x;
	unit->plasty = player.y;
	if (unit->ducking > 0) {
		unit->ducking--;
		if (rand()%4 < 3 && unit->wep.is_ranged) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "The %s ducks for cover.", unit->name);
			add_msg(dmsg);
			return true;
		}
	}
	if (unit->wep.is_ranged) {
		take_dmg(unit, &player, unit->wep);
		return true;
	} else { 
		return ai_aware_melee(unit);
	}
	return false;
}

bool ai_player_not_in_view(struct Unit *unit) {
	if (unit->hunting) {
		if (!move_towards(unit, unit->plastx, unit->plasty)) {
			unit->hunting = false;
			int door = 0;
			int dx = unit->x - unit->plastx;
			int dy = unit->y - unit->plasty;
			get_nearest_door(unit->x, unit->y, &door, dx, dy);
			unit->dest = true;
			unit->destx = doorsxy[door][0];
			unit->desty = doorsxy[door][1];
			printf("lost track of player, guessing door number:%d\n", door);
			// ai should make a guess here
		} else {
			return true;
		}
	} else if (unit->dest) {
		return move_towards(unit, unit->destx, unit->desty);
	} else {
		unit->dest = true;
		pick_valid_xy(&unit->destx, &unit->desty);
		return move_towards(unit, unit->destx, unit->desty);
	}
	return false;
}

void ai_bracket_surround_check(struct Unit *unit) { // this causes a segfault and i don't know why
	int n = 0;
	int vmoves[n][2];
	printf("pre rvm\n");
	return_valid_moves(&n, vmoves, player.x, player.y);
	printf("post rvm, n:%d\n",n);
	if (n == 2) {
		int p = rand()%2;
		printf("p:%d\n", p);
		unit->destx = vmoves[p][0];
		unit->desty = vmoves[p][1];
		unit->flanking = true;
	}
}

bool ai_raise_alarm(struct Unit *unit) {
	if (unit->commwait > 0) return false;
	bool alerted = false;
	for (int i=0; i<unit_num; i++) {
		if (TCOD_map_is_in_fov(tmap, unit_list[i].x, unit_list[i].y) && !unit_list[i].aware) {
			if (!alerted) {
				char dmsg1[MSGSIZE];
				snprintf(dmsg1, MSGSIZE, "The %s alerts its allies to your presence.", unit->name);
				add_msg(dmsg1);
			}
			unit_list[i].aware = unit->aware;
			unit_list[i].hunting = unit->hunting;
			unit_list[i].plastx = unit->plastx;
			unit_list[i].plasty = unit->plasty;
			alerted = true;
			char dmsg2[MSGSIZE];
			snprintf(dmsg2, MSGSIZE, "The %s is alerted to your presence.", unit_list[i].name);
			add_msg(dmsg2);
			unit->commwait += 5;
		}
	}
	return alerted;
}

bool ai_raise_comms(struct Unit *unit) {
	if (!unit->comms || unit->commwait > 0 || player.activeAbilities2 & A2_JAMMING) return false;
	bool alerted = false;
	for (int i=0; i<unit_num; i++) {
		if (!unit_list[i].hunting && unit_list[i].comms) {
			if (!alerted) {
				char dmsg1[MSGSIZE];
				snprintf(dmsg1, MSGSIZE, "The %s radios its allies to your presence.", unit->name);
				add_msg(dmsg1);
			}
			unit_list[i].hunting = unit->hunting;
			unit_list[i].plastx = unit->plastx;
			unit_list[i].plasty = unit->plasty;
			alerted = true;
			unit->commwait += 10;
		}
	}
	return alerted;
}

void ai_unit_turn(struct Unit *unit) {
	int fov = check_fov(*unit);
	TCOD_map_compute_fov(tmap, unit->x, unit->y, fov, true, FOV_PERMISSIVE_8);
	if (TCOD_map_is_in_fov(tmap, player.x, player.y)) {
		if (!map[player.x][player.y].locker && !unit->aware) {
			unit->aware = roll_die(20, unit->per, check_stealth(player, *unit)) > 0;
		}
	} else if (!TCOD_map_is_in_fov(tmap, player.x, player.y)) {
		unit->aware = false;
	}
	if (ai_blind_check(unit)) return; 
	if (ai_stuck_check(unit)) return; // ranged enemies should still be able to attack
	if (unit->flanking) {
		if (!move_towards(unit, unit->destx, unit->desty)) {
			unit->flanking = false;
			printf("not flanking anymore\n");
		}
		printf("attempting flank\n");
	}/*
	if (unit->aware && !unit->flanking) {
		//ai_bracket_surround_check(unit);
		//printf("surround check worked\n");
		int moves = 0;
		int movelist[2][2];
		return_valid_moves(&moves, movelist, unit->x, unit->y);
		if (moves == 2) {
			printf("player is surrounded\n");
			unit->flanking = true;
			int p = rand()%2;
			unit->destx = movelist[p][0];
			unit->desty = movelist[p][1];
		}
		if (!(unit->statusFlags1 & S1_SILENCED)) {
			if (ai_raise_alarm(unit)) {
				return;
			} else if (ai_raise_comms(unit)) {
				return;
			}
		}
		ai_player_in_view(unit);
	} else {
		ai_player_not_in_view(unit);
	}*/
	if (unit->aware && !unit->flanking) ai_player_in_view(unit);
	else ai_player_not_in_view(unit);
}
