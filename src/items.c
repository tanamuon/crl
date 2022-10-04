#include "../include/libtcod.h"
#include "main.h"
#include "items.h"
#include <stdio.h>

bool use_explosive(int x, int y, enum Etype e, int * num);
bool use_noisemaker(struct Unit *unit);

bool menu_item(int *valcopy, int *val, struct Unit *unit, TCOD_key_t skey, int cursx, int cursy) {
	if (skey.c=='m' || (skey.vk==TCODK_ENTER && cursy == 0) ) {
		return use_medi(unit);
	} else if (skey.c=='s' || (skey.vk==TCODK_ENTER && cursy == 1) ) {
		return use_stim(unit);
	} else if (skey.c=='c' || (skey.vk==TCODK_ENTER && cursy == 2) ) {
		return use_pcell(unit);
	} else if (skey.c=='v' || (skey.vk==TCODK_ENTER && cursy == 3) ) {
		if (unit->inv.berserk > 0) {
			unit->berserk = 15;
			unit->inv.berserk--;
			return true;
		}
	} else if (skey.c=='w' || (skey.vk==TCODK_ENTER && cursy == 4) ) {
		if (&(unit->inv.smoke) > 0) return use_explosive(unit->x, unit->y, SMOKE, &(unit->inv.smoke));
	} else if (skey.c=='q' || (skey.vk==TCODK_ENTER && cursy == 5) ) {
		if (&(unit->inv.glue) > 0) return use_explosive(unit->x, unit->y, GLUE, &(unit->inv.glue));
	} else if (skey.c=='f' || (skey.vk==TCODK_ENTER && cursy == 6) ) {
		if (&(unit->inv.paint) > 0) return use_explosive(unit->x, unit->y, PAINT, &(unit->inv.paint));
	} else if (skey.c=='b' || (skey.vk==TCODK_ENTER && cursy == 7) ) {
		if (&(unit->inv.barrier) > 0) return use_explosive(unit->x, unit->y, BARRIER, &(unit->inv.barrier));
	} else if (skey.c=='g' || (skey.vk==TCODK_ENTER && cursy == 8) ) {
		if (&(unit->inv.frag) > 0) return use_explosive(unit->x, unit->y, GRENADE, &(unit->inv.frag));
	} else if (skey.c=='r' || (skey.vk==TCODK_ENTER && cursy == 9) ) {
		if (&(unit->inv.rockets) > 0) return use_explosive(unit->x, unit->y, ROCKET, &(unit->inv.rockets));
	} else if (skey.c=='d' || (skey.vk==TCODK_ENTER && cursy == 10) ) {
		return use_drill(unit);
	} else if (skey.c=='x' || (skey.vk==TCODK_ENTER && cursy == 11) ) {
		if (&(unit->inv.flare) > 0) return use_explosive(unit->x, unit->y, FLARE, &(unit->inv.flare));
	} else if (skey.c=='n' || (skey.vk==TCODK_ENTER && cursy == 12) ) {
		if (&(unit->inv.noisemaker) > 0) return use_noisemaker(unit);
	}
	return false;
}

bool select_item(struct Unit *unit) {
	int cursx = 0; int cursy = 0;
	char *strings[NUMITEMS] = {"[m] Use Medigel", "[s] Use Stimpack", "[c] Use Power Cell", "[v] Use Berserker Cell", "[w] Throw Smoke Grenade", "[q] Throw Glue Grenade", "[f] Throw Paint Grenade", "[b] Throw Barrier Grenade", "[g] Throw Frag Grenade", "[r] Fire Rocket", "[d] Fire Drill Rocket", "[x] Throw Flare", "[n] Throw Noisemaker"};
	int *vals[NUMITEMS] = {&(player.inv.medi), &(player.inv.stim), &(player.inv.pcell), &(player.inv.berserk), &(player.inv.smoke), &(player.inv.glue), &(player.inv.paint), &(player.inv.barrier), &(player.inv.frag), &(player.inv.rockets), &(player.inv.drill), &(player.inv.flare), &(player.inv.noisemaker)};
	int inv_sum1 = 0;
	for (int i=0; i<NUMITEMS; i++) inv_sum1 += *vals[i]; //kludge
	menu(NUMITEMS, strings, vals, unit, false, 0, 0, cursx, cursy, menu_item);
	int inv_sum2 = 0;
	for (int i=0; i<NUMITEMS; i++) inv_sum2 += *vals[i]; // more kludge
	if (inv_sum1 > inv_sum2) {
		return true;
	} else {
		return false;
	}
}

bool use_medi(struct Unit *unit) {
	if (unit->inv.medi > 0 && unit->chp < unit->mhp) {
		if (unit->chp + (int)unit->mhp*0.5 < unit->mhp) {
			unit->chp += (int)unit->mhp*0.5;
		} else {
			unit->chp = unit->mhp;
		}
		unit->inv.medi--;
		return true;
	} 
	return false;
}

bool use_stim(struct Unit *unit) {
	if (unit->inv.stim > 0) {
		unit->regen = 10;
		unit->mhp = unit->mhp * 2;
		unit->inv.stim--;
		return true;
	} 
	return false;
}

bool use_pcell(struct Unit *unit) {
	if (unit->inv.pcell > 0) {
		//unit->oc = 10;
		unit->cpow = unit->mpow;
		unit->inv.pcell--;
		return true;
	} 
	return false;
}

bool use_noisemaker(struct Unit *unit) {
	int tx = unit->x; int ty = unit->y;
	if(target(&tx, &ty, 0)){
		for (int i=0; i<unit_num; i++) {
			if (!unit_list[i].aware && !unit_list[i].hunting) {
				unit_list[i].hunting = true;
				unit_list[i].plastx = tx;
				unit_list[i].plasty = ty;
			}
		}
		unit->inv.noisemaker--;
	}
	return true;
}

bool use_drill(struct Unit *unit) {
	struct Explosive drill;
	drill.map_alt = true; drill.flare = false; drill.smoke = false; drill.glue = false; drill.paint = false; drill.barrier = false; drill.dmgdie = 0; drill.dmgsides = 0; drill.radius = 0;
	if (unit->inv.drill > 0) {
		draw_all();
		TCOD_key_t key = TCOD_console_wait_for_keypress(true);
		int dx = 0, dy = 0;
		get_dxdy(key, &dx, &dy);
		int i = 1;
		int imax = 10;
		while (i<imax && player.x+i*dx > 0 && player.x+i*dx < MAPX && player.y+i*dy > 0 && player.y+i*dy < MAPY) {
			explode(drill, player.x+i*dx, player.y+i*dy);
			i++;
		}
		unit->inv.drill--;
		return true;
	}
	return false;
}

bool use_explosive(int x, int y, enum Etype e, int * num) {
	struct Explosive bomb;
	if (e == ROCKET) {
		bomb.dmgdie = 20; bomb.dmgsides = 5; bomb.radius = 3; bomb.map_alt = true;
	} else if (e == GRENADE) {
		bomb.dmgdie = 10; bomb.dmgsides = 5; bomb.radius = 2;
	} else if (e == SMOKE) {
		bomb.dmgdie = 0; bomb.dmgsides = 0; bomb.radius = 4; bomb.smoke = true;
	} else if (e == BARRIER) {
		bomb.dmgdie = 0; bomb.dmgsides = 0; bomb.radius = 0; bomb.barrier = true;
	} else if (e == GLUE) {
		bomb.dmgdie = 0; bomb.dmgsides = 0; bomb.radius = 2; bomb.glue = true;
	} else if (e == PAINT) {
		bomb.dmgdie = 0; bomb.dmgsides = 0; bomb.radius = 2; bomb.paint = true;
	} else if (e == FLARE) {
		bomb.dmgdie = 0; bomb.dmgsides = 0; bomb.radius = 2; bomb.smoke = false; bomb.flare = true;
	}
	if(target(&x, &y, bomb.radius)) {
		explode(bomb, x, y);
		*num = *num - 1;
	}
	return true;
}
