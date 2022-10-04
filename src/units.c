#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../include/libtcod.h"
#include "main.h"
#include "units.h"
#include "items.h"
#include "ai.h"
#include "skills.h"

struct Armor calc_armor(struct Unit unit) {
	struct Armor armor_total;
	armor_total.acdie = unit.h_arm.acdie + unit.b_arm.acdie + unit.a_arm.acdie + unit.l_arm.acdie;
	armor_total.acsides = unit.h_arm.acsides + unit.b_arm.acsides + unit.a_arm.acsides + unit.l_arm.acsides;
	armor_total.crit_res = unit.h_arm.crit_res + unit.b_arm.crit_res + unit.a_arm.crit_res + unit.l_arm.crit_res;
	armor_total.ev_pen = unit.h_arm.ev_pen + unit.b_arm.ev_pen + unit.a_arm.ev_pen + unit.l_arm.ev_pen;
	armor_total.melee_pen = unit.h_arm.melee_pen + unit.b_arm.melee_pen + unit.a_arm.melee_pen + unit.l_arm.melee_pen;
	armor_total.acc_pen = unit.h_arm.acc_pen + unit.b_arm.acc_pen + unit.a_arm.acc_pen + unit.l_arm.acc_pen;
	armor_total.stealth_pen = unit.h_arm.stealth_pen + unit.b_arm.stealth_pen + unit.a_arm.stealth_pen + unit.l_arm.stealth_pen;
	armor_total.per_pen = unit.h_arm.per_pen + unit.b_arm.per_pen + unit.a_arm.per_pen + unit.l_arm.per_pen;
	if (unit.activeAbilities1 & A1_CAMERA_SYSTEM) {
		armor_total.ev_pen = unit.b_arm.ev_pen + unit.a_arm.ev_pen + unit.l_arm.ev_pen;
		armor_total.per_pen = 0;
	}
	return armor_total;
}

void observe() {
	if (player.activeAbilities1 & A1_STEALTH_SUBROUTINE) { // only player can use without rewriting
		for (int i=0; i<nunits; i++) {
			if (!unit_list[units_in_view[i]].aware && unit_list[units_in_view[i]].turns_seen < 10) {
				unit_list[units_in_view[i]].turns_seen++;
			}
		}
	}
}

void dec_pow(struct Unit *unit) { // this just sort of became a "stuff that runs every turn function"
	if (unit->camo) unit->camo = false;
	if (unit->lt_dx>0 || unit->lt_dy>0) {
		unit->running = true;
	} else {
		unit->running = false;
	}
	if (unit->berserk > 1) {
		unit->berserk = unit->berserk - 1;
	} else if (unit->berserk == 1) {
		unit->berserk = 0;
	}
	if (unit->sturns > 1) {
		unit->sturns = unit->sturns - 1;
	} else if (unit->sturns == 1) {
		unit->sturns = 0;
		unit->stuck = false;
	}
	if (unit->commwait > 1) {
		unit->commwait--;
	} else if (unit->commwait == 1) {
		unit->commwait = 0;
	}
	enemies_in_view(&player);
	observe();
	regen(unit);
	unit->noevade = false;
	unit->turnspent = false;
}

bool weld_door(struct Unit *unit, int dx, int dy) {
	if (unit->activeAbilities1 & A1_WELDING_SYSTEM && unit->cpow >= 5) {
		map[player.x+dx][player.y+dy].can_walk = !map[player.x+dx][player.y+dy].can_walk;
		unit->cpow-=5;
		if (map[player.x+dx][player.y+dy].c == '#') {
			map[player.x+dx][player.y+dy].c = '%';
		} else {
			map[player.x+dx][player.y+dy].c = '#';
		}
		gen_tmap();
		return true;
	}
	return false;
}

void gen_allowed_units(struct Unit allowed_units[MAXUNITS], int *dunits, int *sweight_total) {
	for (int j=0; j<utypes; j++) {
		if (depth >= unit_types[j].mindepth && depth <= unit_types[j].maxdepth) {
			allowed_units[*dunits] = unit_types[j];
			*dunits = *dunits + 1;
		}
	}
	for (int k=0; k<*dunits; k++) {
		 *sweight_total+=allowed_units[k].spawnweight;
	}
}

void gen_unit_list() {
	struct Unit allowed_units[MAXUNITS];
	int dunits = 0;
	int sweight_total = 0;
	gen_allowed_units(allowed_units, &dunits, &sweight_total);
	int i;
	unit_num = (int)(0.40*MAXUNITS + rand() % MAXUNITS*0.60);
	for (i=0; i<unit_num; i++) {
		int x; int y;
		int spawnroll = rand()%sweight_total;
		int sweight_current = 0;
		int l = 0;
		while (l<dunits) {
			sweight_current += allowed_units[l].spawnweight;
			if (spawnroll<sweight_current) {
				unit_list[i] = allowed_units[l];
				for (int i=0; i<10; i++) {
					pick_valid_xy(&x, &y);
					if ( abs(player.x - x) > player.fov && abs(player.y - y) > player.fov ) {
						break;
					}
				}
				unit_list[i].x = x;
				unit_list[i].y = y;
				if (map[unit_list[i].x][unit_list[i].y].can_walk == false) {
					printf("Error: Unit placed in wall\n");
				}
				unit_list[i].conc_pen = 0;
				unit_list[i].turns_seen = 0;
				l = dunits;
			}
			l++;
		}
	}
}

void regen(struct Unit *unit) {
	if (unit->regen > 1) {
		unit->chp+=1;
		unit->regen -= 1;
	} else if (unit->regen == 1) {
		unit->chp+=1;
		unit->regen -= 1;
		unit->mhp = unit->uhp;
		if (unit->chp >= unit->uhp) {
			unit->chp = unit->uhp;
		}
	} else if (turn % 10 == 0) {
		if (unit->chp < unit->mhp && nunits == 0) {
			unit->chp+=1;
		}
		if (unit->cpow < unit->mpow) {
			unit->cpow+=1;
		}
	}
}

bool camo(struct Unit *unit) {
	if (unit->activeAbilities1 & A1_ACTIVE_CAMO && unit->cpow>5) {
		unit->camo = true;
		unit->cpow -= 5;
		return true;
	} else {
		unit->camo = false;
		return false;
	}
}

int check_stealth(struct Unit unit, struct Unit observer) {
	struct Armor armor = calc_armor(unit);
	int obpen = 0;
	int unitper = check_perception(unit);
	if (unit.fac == 0) {
		obpen = observer.turns_seen;
		if (obpen > unitper/2) obpen = unitper/2;
	}
	int stealth = unit.stealth - armor.stealth_pen + check_concealment(unit)*2 + obpen + unit.camo*20;
	if (map[unit.x][unit.y].lit) stealth = 0;
	return stealth;
}

int check_perception(struct Unit unit) {
	struct Armor armor = calc_armor(unit);
	return unit.per - armor.per_pen - unit.camo*10;
}

int check_fov(struct Unit unit) {
	return unit.fov + (((unit.activeAbilities1 & A1_RECON_SUBROUTINE)>0)*unit.per/5);
}

int check_cover(struct Unit unit) {
	int x = unit.x; int y = unit.y;
	int cover = map[x][y].cover;
	return cover;
}

int check_concealment(struct Unit unit) {
	int x = unit.x; int y = unit.y;
	int concealment = map[x][y].concealment;
	return concealment;
}

int check_crit_res(struct Unit unit) {
	struct Armor armor = calc_armor(unit);
	return armor.crit_res + ((unit.activeAbilities1 & A1_MED2) > 0)*5 + ((unit.activeAbilities1 & A1_CAMERA_SYSTEM) > 0)*unit.per/4;
}

void die(struct Unit *unit, bool m) {
	if (unit->ownedAbilities1 & A1_MED2 && !(unit->activeAbilities1 & A1_MED2)) {
		unit->mhp += (int)unit->mhp*0.25;
		unit->chp = unit->mhp;
		unit->activeAbilities1 |= A1_MED2;
		if (unit->fac == 0) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "Your suit isolates your brain from your body.");
			add_msg(dmsg);
		} else {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "The %s's suit isolates its brain from its body.", unit->name);
			add_msg(dmsg);
		}
		return;
	}
	if (unit->fac == 0) {
		print_msg(smsgs[2]);
		int cursx = 0; int cursy = 0;
		int null = 1;
		char *strings[3] = { "Start new game.", "Load saved game.", "Exit game."};
		int *vals[3] = {&null, &null, &null};
		struct Unit gmenu; gmenu.noescape = true;
		menu(3, strings, vals, &gmenu, false, 0, 0, cursx, cursy, menu_3game);
	} else { 
		int ind = find_unit_ind(unit->x, unit->y);
		for (int i=0; i<unit_num; i++) {
			if (i==ind) {
				if (m) {
					char dmsg[MSGSIZE];
					snprintf(dmsg, MSGSIZE, "You kill the %s.", unit->name);
					add_msg(dmsg);
				}
				for (int j=i; j<unit_num-1; j++) {
					unit_list[j] = unit_list[j+1];
				}
				unit_num--;
				return;
			}
		}
	}
}

void take_dmg(struct Unit *attacker, struct Unit *defender, struct Weapon wep) {
	attacker->turnspent = true;
	if (attacker->fac == defender->fac) {
		printf("Error: No friendly fire allowed, this is a bug. \n");
		return; 
	}
	if (attacker->activeAbilities1 & A1_PREDICTION_SUBROUTINE && defender->conc_pen < check_perception(*attacker)) { // this will need to be changed for enemies to use
		defender->conc_pen++;
	}
	if (attacker->fam && defender->berserk < 1) {
		defender->ducking = 2;
	}
	struct Armor atk_armor = calc_armor(*attacker);
	struct Armor def_armor = calc_armor(*defender);
	int hit;
	int dodge; int flatdodge;
	int apos = 5*attacker->spos;
	int dpos = 5*defender->spos;
	int d1 = rand()%21;
	int d2 = rand()%21;
	if (wep.is_ranged) {
		if (wep.bipod && apos > 0) {
			hit = rand()%21 + attacker->ranged + wep.acc + apos*2;
		} else {
			hit = rand()%21 + attacker->ranged + wep.acc - wep.acc_pen - atk_armor.acc_pen + apos;
		}
		flatdodge = defender->ev - def_armor.ev_pen + dpos + check_concealment(*defender);
	} else {
		if (attacker->activeAbilities1 & A1_IMPETUOUS_OFFENSE) {
			hit = rand()%(21 + attacker->melee*2 - atk_armor.melee_pen);
		} else {
			hit = rand()%21 + attacker->melee - atk_armor.melee_pen;
		}
		flatdodge = defender->ev - def_armor.ev_pen - dpos;
	}
	if (defender->running && wep.is_ranged) {
		flatdodge+=5;
	}
	flatdodge = flatdodge - defender->conc_pen;
	if (defender->activeAbilities1 & A1_FACILE_DODGING) {
		dodge = (d1 > d2 ? d1 : d2) + flatdodge;
	} else {
		dodge = d1 + flatdodge;
	}
	//if (flatdodge < 0) dodge = 0;
	if ( ((dodge > hit && !(attacker->activeAbilities1 & A1_HEADSHOT_PROTOCOL)) || (dodge>(hit/2) && attacker->activeAbilities1 & A1_HEADSHOT_PROTOCOL)) && !defender->noevade) {
		if (attacker->fac == 0) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "You miss the %s. (%d vs %d)", defender->name, hit, dodge);
			add_msg(dmsg);
		} else if (defender->fac == 0) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "The %s misses you. (%d vs %d)", attacker->name, hit, dodge);
			add_msg(dmsg);
		}
		if (defender->activeAbilities1 & A1_RIPOSTE) { // maybe just pistols, bayonets, and melee weapons?
			defender->riposting = true;
		}
		return;
	}
	dodge = d1 + flatdodge;
	if (attacker->activeAbilities2 & A2_SILENCER && !defender->silenced) {
		defender->silenced = true;
		char dmsg[MSGSIZE];
		snprintf(dmsg, MSGSIZE, "The %s is silenced.", defender->name);
		add_msg(dmsg);
	}
	if (attacker->immobilizer && !wep.is_ranged && !defender->stuck) { //change to adjacent check
		defender->stuck = true;
		defender->sturns = 2;
		char dmsg[MSGSIZE];
		snprintf(dmsg, MSGSIZE, "The %s immobilizes you.", attacker->name);
		add_msg(dmsg);
		return;
	}
	if (wep.tagger == true) { 
		defender->tagged = true; 
		if (attacker->fac == 0) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "You land a tracker on the %s. (%d - %d)", defender->name, hit, dodge);
			add_msg(dmsg);
		} else if (defender->fac == 0) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "The %s lands a tracker on you. (%d - %d)", attacker->name, hit, dodge);
			add_msg(dmsg);
		}
		if (wep.dmgdie < 1) {
			return;
		}
	}
	int critdie = (hit - dodge)/(10 - wep.critmod + check_crit_res(*defender));
	if (attacker->activeAbilities1 & A1_HEADSHOT_PROTOCOL) critdie = critdie*2;
	int dmgroll = 0;
	for (int i=0; i<wep.dmgdie+critdie; i++) {
		dmgroll += rand() % (wep.dmgsides+1);
	}
	if (defender->activeAbilities1 & A1_FACILE_DODGING) {
		int dmgroll2 = 0;
		for (int i=0; i<wep.dmgdie+critdie; i++) {
			dmgroll2 += rand() % (wep.dmgsides+1);
		}
		if (dmgroll2 > dmgroll) dmgroll = dmgroll2;
	}
	int preAP_acroll = 0;
	int acroll = rand()%(def_armor.acsides+1) + def_armor.acdie;
	/*
	for (int i=0; i<=def_armor.acdie; i++) {
		acroll += rand()%(def_armor.acsides+1);
	}
	*/
	if (wep.is_ranged) acroll += check_cover(*defender);
	acroll = preAP_acroll - (preAP_acroll*wep.AP)/2; //check this
	int dmg = dmgroll - acroll;
	if (!defender->aware && wep.dmgdie > 0 && (!wep.suppressed || !wep.is_ranged)) {
		int check = roll_die(20, defender->per + dmg, check_stealth(*attacker, *defender));
		if (check>0 || !wep.is_ranged) {
			defender->aware = true;
			defender->dest = true;
			defender->destx = attacker->x; defender->desty = attacker->y;
		}
		if (attacker->activeAbilities1 & A1_ASSASSINATE) {
			hit += check_stealth(*attacker, *defender);
		}
	}
	if (dmg < 0) dmg = 0; // Paranoia
	if (dmg > 0 && critdie > 0) { //knockback not triggering
		if (attacker->activeAbilities1 & A1_CRIPPLING_SHOT) defender->crippled = true;
		if (!wep.is_ranged && attacker->activeAbilities2 & A2_KNOCKBACK) punt(attacker, defender->x-attacker->x, defender->y-attacker->y, 2);
		if (wep.elec) {
			int status = rand()%3;
			char sname[3][32] = {"cripples" , "immobilizes", "blinds"};
			if (status == 0) defender->crippled = true;
			else if (status == 1) defender->noevade = true; //bug, might not work
			else if (status == 2) defender->blind = true;
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "Your electrified attack %s the %s.", sname[status], defender->name);
			add_msg(dmsg);
		} 
		if (attacker->activeAbilities1 & A1_EXECUTE && \
		!wep.is_ranged && defender->chp < attacker->rwep.dmgdie*attacker->rwep.dmgsides && \
		attacker->rwep.handedness < 2) {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "You execute the %s.", defender->name);
			add_msg(dmsg);
			die(defender, false);
			return;
		}
		
	}
	defender->chp -= dmg;
	if (!wep.is_ranged && defender->activeAbilities1 & A1_PUNISH) attacker->noevade = true;
	if (attacker->fac == 0) {
		char dmsg[MSGSIZE];
		if (attacker->activeAbilities1 & A1_HEADSHOT_PROTOCOL) {
			snprintf(dmsg, MSGSIZE, "You headshot the %s for %d dmg. (%d vs %d, %d - %d)", defender->name, dmg, hit/2, dodge, dmgroll, acroll);
		} else {
			snprintf(dmsg, MSGSIZE, "You deal %d damage to the %s. (%d vs %d, %d - %d)", dmg, defender->name, hit, dodge, dmgroll, acroll);
		}
		add_msg(dmsg);
	} else if (defender->fac == 0) {
		char dmsg[MSGSIZE];
		if (attacker->activeAbilities1 & A1_HEADSHOT_PROTOCOL) {
			snprintf(dmsg, MSGSIZE, "The %s headhsots you for %d dmg. (%d vs %d, %d - %d)", defender->name, dmg, hit/2, dodge, dmgroll, acroll);
		} else {
			snprintf(dmsg, MSGSIZE, "The %s hits you for %d damage. (%d vs %d, %d - %d)", attacker->name, dmg, hit, dodge, dmgroll, acroll);
		}
		add_msg(dmsg);
	}
	if (defender->chp <= 0) die(defender, true);
}

void take_indirect_dmg(struct Unit *defender, struct Explosive wep) { // should be rolled into take_dmg
	int dmgroll = 0; 
	for (int i=0; i<wep.dmgdie; i++) {
		dmgroll += rand() % (wep.dmgsides+1);
	}
	int acroll = 0;
	for (int i=0; i<defender->b_arm.acdie; i++) {
		acroll += rand() % (defender->b_arm.acsides+1);
	}
	int dmg = dmgroll - acroll;
	if (dmg < 0) { dmg = 0; }
	defender->chp -= dmg;
	char dmsg[MSGSIZE];
	snprintf(dmsg, MSGSIZE, "The explosion hits the %s for %d damage. (%d - %d)", defender->name, dmg, dmgroll, acroll);
	add_msg(dmsg);
	if (defender->chp <= 0) {
		die(defender, true);
	} 
}

void explode(struct Explosive bomb, int x, int y) {
	if (bomb.barrier && !map[x][y].door && !map[x][y].down_stair) {
		map[x][y].is_transparent = false;
		map[x][y].can_walk = false;
		map[x][y].c = '+';
		map[x][y].effect = 2;
		map[x][y].teffect = 10;
		TCOD_map_set_properties(tmap, x, y, map[x][y].is_transparent, map[x][y].can_walk);
	}
	for (int i=x-bomb.radius; i<x+bomb.radius+1; i++) {
		for (int j=y-bomb.radius; j<y+bomb.radius+1; j++) {
			if ( (0<i && i<MAPX) && (0<j && j<MAPY) ) {				
				int r = 100 - 25*MAX(abs(i-x),abs(j-y));
				if (bomb.smoke && map[i][j].is_transparent) {
					map[i][j].is_transparent = false;
					map[i][j].effect = 1;
					map[i][j].teffect = 10;
				}
				if (bomb.map_alt && !map[i][j].down_stair) {
					map[i][j] = sfloor;
				}
				if (bomb.flare) {
					map[i][j].lit = true;
				}
				TCOD_map_set_properties(tmap, i, j, map[i][j].is_transparent, map[i][j].can_walk);
				int ind = find_unit_ind(i, j);
				if (ind == -2) {
					if (r>rand()%101) take_indirect_dmg(&player, bomb);	
				} else if (ind != -1) {
					if (r>rand()%101) take_indirect_dmg(&unit_list[ind], bomb);
					if (bomb.glue) {
						unit_list[ind].stuck = true;
						unit_list[ind].sturns = 10;
					}
					if (bomb.paint) { 
						unit_list[ind].blind = true;
						unit_list[ind].bturns = 10;
					}
					if (bomb.flare) { 
						unit_list[ind].lit = true;
					}
				}
			}
		}
	}
	gen_tmap();
}

void enemies_in_view(struct Unit *unit) { // uh this is a disaster
	int fov = check_fov(*unit);
	TCOD_map_compute_fov(tmap, unit->x, unit->y, check_fov(*unit), true, FOV_PERMISSIVE_8);
	nunits = 0;
	for(int i=unit->x-fov; i<unit->x+fov; i++) {
		for(int j=unit->y-fov; j<unit->y+fov; j++) {
			int ind = find_unit_ind(i,j);
			if (ind >= 0) {
				if (unit->fac != unit_list[ind].fac && TCOD_map_is_in_fov(tmap, i, j)) {
					units_in_view[nunits] = ind;
					nunits+=1;
				}
			}
		}
	}
	TCOD_map_compute_fov(tmap, player.x, player.y, 100, true, FOV_PERMISSIVE_8);
	for (int k=0; k<unit_num; k++) {
		if (TCOD_map_is_in_fov(tmap, unit_list[k].x, unit_list[k].y) && unit_list[k].lit) {
			bool seen = false;
			for (int z=0; z < nunits; z++) {
				if (units_in_view[z] == k) seen = true;
			}
			if (!seen) {
				units_in_view[nunits] = k;
				nunits+=1;
			}
		}
	}
	TCOD_map_compute_fov(tmap, unit->x, unit->y, check_fov(*unit), true, FOV_PERMISSIVE_8);
}

bool check_if_in_view(int x) {
	for (int i=0; i<nunits; i++) {
		if (units_in_view[i] == x) return true;
	}
	return false;
}

void shoot_info(int t_ind) { // this and look should use the same function to display unit info
	draw_all();
	TCOD_console_put_char(NULL, unit_list[units_in_view[t_ind]].x, unit_list[units_in_view[t_ind]].y, unit_list[units_in_view[t_ind]].c, TCOD_BKGND_LIGHTEN);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+1, (MAPX+MSGSIZE)/2, 20, "%s [%c]", unit_list[units_in_view[t_ind]].name, unit_list[units_in_view[t_ind]].c);
	TCOD_console_print_rect(NULL, (MAPX+HUDX)/2+1, MAPY+3, (MAPX+MSGSIZE)/2, 20, "%s", unit_list[units_in_view[t_ind]].desc);
	TCOD_console_flush();
}

bool shoot(struct Unit *unit, TCOD_key_t skey) {
	if (unit->rwep.set_req && (unit->rwep.setupx != unit->x || unit->rwep.setupy != unit->y) ) {
		add_msg("You ready your weapon.");
		unit->rwep.setupx = player.x;
		unit->rwep.setupy = player.y;
		return true;
	}
	if (nunits == 0) {
		add_msg("No valid targets.");
		return false;
	}
	int t_ind = 0;
	for (int i=0; i<=nunits; i++) {
		if (units_in_view[i] == units_in_view[last_hit]) {
			t_ind = last_hit;
		}
		if (i == nunits) {
			t_ind = 0;
		}
	}
	shoot_info(t_ind);
	TCOD_key_t key = TCOD_console_wait_for_keypress(true);
	int dx = 0; int dy = 0;	
	while (key.c != 'f' && key.vk != TCODK_ESCAPE) { // moves through list of units in view
		get_dxdy(key, &dx, &dy);
		if (abs(dx) + abs(dy) > 1) { // diagonals act as left/right
			dy = 0;
		}
			if (t_ind + dx + dy > nunits-1) {
				t_ind = 0;
			} else if (t_ind + dx + dy < 0) {
				t_ind = nunits-1;
			} else {
				t_ind += dx + dy;
			}
		shoot_info(t_ind);
		key = TCOD_console_wait_for_keypress(true);
		
	}
	if (key.c=='f' && !skey.shift && nunits>0) {
		last_hit = t_ind;
		if (unit->activeAbilities1 & A1_HAMMER_SHOT) {
			struct Unit hunit = *unit;
			hunit.ranged = hunit.ranged/2;
			take_dmg(&hunit, &unit_list[units_in_view[t_ind]], unit->rwep);
			take_dmg(&hunit, &unit_list[units_in_view[t_ind]], unit->rwep);
		} else {
			take_dmg(unit, &unit_list[units_in_view[t_ind]], unit->rwep);
		}
		return true;
	} else if (skey.shift && unit->activeAbilities1 & A1_COVERING_FIRE) {
		struct Unit hunit = *unit;
		hunit.ranged = hunit.ranged/4;
		hunit.fam = true;
		take_dmg(&hunit, &unit_list[units_in_view[t_ind]], unit->rwep);
		return true;
	}
	return false;
}

int find_unit_ind(int x, int y) { // returns index in unit array, -1 if not in, -2 if player
	for (int i=0; i<unit_num; i++) {
		if (unit_list[i].x == x && unit_list[i].y == y) {
			return i;
		}
	}
	if (player.x == x && player.y == y) {
		return -2;
	}
	return -1;
}

void check_zoc(struct Unit *unit, struct Unit *check) {
	if (check->activeAbilities1 & A1_ZONE_OF_CONTROL && unit->fac > 0) {
		if (abs(check->x - unit->x) < 2 && abs(check->y - unit->y) < 2) {
			if (unit->lt_dx != check->lt_dx || unit->lt_dy != check->lt_dy) {
				take_dmg(check, unit, check->mwep);
			}
		}
	}
}

void check_flank(struct Unit *unit, int dx, int dy) {
	if (unit->activeAbilities1 & A1_FLANKING) {
		for (int i=0; i<unit_num; i++) {
			if (abs(unit->x - unit_list[i].x) < 2 && abs(unit->y - unit_list[i].y) < 2) {
				if (abs(unit->x+dx - unit_list[i].x) < 2 && abs(unit->y+dy - unit_list[i].y) < 2) {
					if (unit_list[i].fac != unit->fac) {
						take_dmg(unit, &unit_list[i], unit->wep);
					}
				}
			}
			if (abs(unit->x - player.x) < 2 && abs(unit->y - player.y) < 2) {
				if (abs(unit->x+dx - player.x) < 2 && abs(unit->y+dy - player.y) < 2) {
					if (player.fac != unit->fac) {
						take_dmg(unit, &player, unit->wep);
					}
				}
			}
		}
	}
}

bool move_or_attack(struct Unit *unit, int dx, int dy) {
	if (unit->lt_dx == dx && unit->lt_dy == dy) {
		unit->spturn+=1;
	} else {
		unit->spturn=0;
	}
	if (map[unit->x][unit->y].locker && map[unit->x+dx][unit->y+dy].locker) return false;
	int ind;
	if ( (unit->activeAbilities1 & A1_OVERDRIVE_GEARS && unit->spturn > 4) || unit->oc > 0) {
		int i1; int i2;
		i1 = find_unit_ind(unit->x+dx, unit->y+dy);
		dx = dx*2; dy = dy*2;
		i2 = find_unit_ind(unit->x+dx, unit->y+dy);
		if (unit->oc > 0) { unit->oc--; }
		if ((i1 == i2) && i1 == -1) {
			ind = -1;
		} else {
			ind = i1;
		}
	} else {
		ind = find_unit_ind(unit->x+dx, unit->y+dy);
	}
	if (map[unit->x+dx][unit->y+dy].can_walk && ind == -1) {
		if (unit->stuck) {
			if (unit->fac == 0) {
				char dmsg[MSGSIZE];
				snprintf(dmsg, MSGSIZE, "You are immobilized");
				add_msg(dmsg);
				return false;
			}
			return false;
		}
		check_flank(unit, dx, dy);
		if (!unit->crippled || rand()%2 > 0) {
			unit->x+=dx; unit->y+=dy;
		} else {
			char dmsg[MSGSIZE];
			snprintf(dmsg, MSGSIZE, "The %s stumbles.", unit->name);
			add_msg(dmsg);
		}
		if ( (unit->activeAbilities1 & A1_OVERDRIVE_GEARS && unit->spturn > 4) || unit->oc > 0) { 
			unit->lt_dx=dx/2; unit->lt_dy=dy/2; 
		} else {
			unit->lt_dx=dx; unit->lt_dy=dy;
		} 
		check_zoc(unit, &player);
		unit->rwep.setupx = -1;
		unit->rwep.setupy = -1;
		return true;
	}
	if (map[unit->x+dx][unit->y+dy].can_walk && ind != -1) {
		if (unit->rwep.set_req && (unit->rwep.setupx != unit->x || unit->rwep.setupy != unit->y) ) {
		add_msg("You ready your weapon.");
		unit->rwep.setupx = player.x;
		unit->rwep.setupy = player.y;
		return true;
		}
		if (unit->fac != unit_list[ind].fac && ind != -2) {
			if (unit->wep.is_ranged && unit->activeAbilities1 & A1_HAMMER_SHOT) {
				struct Unit hunit = *unit;
				hunit.ranged = hunit.ranged/2;
				take_dmg(&hunit, &unit_list[ind], unit->wep);
				take_dmg(&hunit, &unit_list[ind], unit->wep);
			} else {
				struct Weapon wep = unit->wep;
				if (unit->berserk > 0 && !unit->wep.is_ranged) wep.dmgsides = wep.dmgsides*2;
				take_dmg(unit, &unit_list[ind], wep);
			}
		} else if (ind == -2) {
			take_dmg(unit, &player, unit->wep);
			if (player.riposting) {
				take_dmg(&player, unit, unit->wep);
				player.riposting = false;
			}
		}
		unit->lt_dx=0; unit->lt_dy=0;
		return true;
	}
	return false;
}

int roll_die(int sides, int atk, int def) {
	int roll = rand()%(sides+1) + atk;
	int target = rand()%(sides+1) + def;
	if (roll >= target) {
		return roll-target;
	} else {
		return -1;
	}
}

void ai_turn() {
	for(int i=0; i<unit_num; i++) {
		dec_pow(&unit_list[i]);
	}
	for (int j=0; j<10; j++) { // kludge fix
		for(int i=0; i<unit_num; i++) {
			if (!unit_list[i].turnspent) ai_unit_turn(&unit_list[i]);;
		}
	}
}

void draw_unit(struct Unit unit) {
	TCOD_color_t ucolor = {unit.r, unit.g, unit.b};
	if (unit.fac==0) {
		if (unit.chp*4 < unit.mhp) {
			TCOD_color_t pcolor = {255, 0, 0};
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, pcolor, TCOD_black);
		} else if (unit.chp*2 < unit.mhp && unit.chp*4 > unit.mhp) {
			TCOD_color_t pcolor = {255, 255, 0};
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, pcolor, TCOD_black);
		} else if (unit.chp*2 > unit.mhp) {
			TCOD_color_t pcolor = {255, 255, 255};
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, pcolor, TCOD_black);
		}
		return;
	} else if (unit.aware) {
		if (unit.noevade) {
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_light_blue);
		} else if (unit.berserk > 0) {
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_dark_red);
		} else if (unit.regen > 0) {
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_green);
		} else {
			TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_black);
		}
	} else if (unit.hunting) {
		TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_light_grey);
	} else {
		TCOD_console_put_char_ex(NULL, unit.x, unit.y, unit.c, ucolor, TCOD_dark_grey);
	}
}

void draw_units() {
	if (player.activeAbilities1 & A1_DOOR_HACKING) {
		for (int x=player.x-1; x<player.x+2; x++) {
			for (int y=player.y-1; y<player.y+2; y++) {
				if (map[x][y].door) {
					map[x][y].is_transparent = 1;
				}
			}
		}
	}
	gen_tmap();
	TCOD_map_compute_fov(tmap, player.x, player.y, check_fov(player), true, FOV_PERMISSIVE_8);
	if (player.activeAbilities1 & A1_DOOR_HACKING) {
		for (int x=player.x-1; x<player.x+1; x++) {
			for (int y=player.y-1; y<player.y+1; y++) {
				if (map[x][y].is_transparent && map[x][y].door) {
					map[x][y].is_transparent = 0;
				}
			}
		}
	}
	int i;
	for (i=0;i<unit_num;i++) {
		if (TCOD_map_is_in_fov(tmap, unit_list[i].x, unit_list[i].y) || unit_list[i].tagged) {
			draw_unit(unit_list[i]);
		} else if (player.activeAbilities1 & A1_MICROPHONE_SYSTEM) {
			int proll = roll_die(20, player.per, unit_list[i].stealth);
			bool inprange = (proll > abs(player.x - unit_list[i].x) && proll > abs(player.y - unit_list[i].y));
			if (inprange) {
				if (proll > 15) {
					draw_unit(unit_list[i]);
				} else if (proll > 0) {
					TCOD_console_put_char_ex(NULL, unit_list[i].x, unit_list[i].y, '*', TCOD_light_grey, TCOD_black);
				}
			}
		}
	}
	gen_tmap();
	TCOD_map_compute_fov(tmap, player.x, player.y, 100, true, FOV_PERMISSIVE_8);
	for (int k=0; k<unit_num; k++) {
		if (TCOD_map_is_in_fov(tmap, unit_list[k].x, unit_list[k].y) && unit_list[k].lit) {
			draw_unit(unit_list[k]);
			for (int l=unit_list[k].x-unit_list[k].fov+1; l<unit_list[k].x+unit_list[k].fov; l++) {
				for (int m=unit_list[k].y-unit_list[k].fov+1; m<unit_list[k].y+unit_list[k].fov; m++) {
					if (map[l][m].is_transparent && find_unit_ind(l, m) == -1) {
						map[l][m].tlit = 1;
					}
				}
			}
		} 
	}
	//TCOD_console_put_char_ex(NULL, player.x+1, player.y-1, '*', TCOD_orange, TCOD_black);
}

void draw_units_om() {
	for (int i=0;i<unit_num;i++) {
		draw_unit(unit_list[i]);
	}
}
