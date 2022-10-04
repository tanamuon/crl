#include "../include/libtcod.h"
#include "main.h"
#include "skills.h"

extern int find_unit_ind(int x, int y);

void learn_ability() {
	TCOD_key_t key;
	key.vk = TCODK_BACKSPACE;
	int cursx = 0; int cursy = 0;
	TCOD_console_set_color_control(TCOD_COLCTRL_1,TCOD_white,TCOD_black);
	TCOD_console_set_color_control(TCOD_COLCTRL_2,TCOD_grey,TCOD_black);
	int exp_req = player.abilities * 500 + 500;
	int snum = 35;
	char skills[70][1024] =
	{
	"Vertical Jump Pistons", "Ctrl + Shift + Direction allows you to jump over an adjacent enemy if there is an empty space behind them. Costs 5 energy.", 
	"Horizontal Jump Pistons", "Shift + Direction allows you to dash up to 6 spaces or in front of the nearest obstacle, whichever is shorter. Costs 5 energy.", 
	"Elbow Pistons", "Ctrl + Direction allows punches an enemy up to 6 spaces away or into the nearest obstacle. If they hit an obstacle damage done is equal to (spaces flung)d(melee skill). Gauntlet style weapon stats are added to damage roll if equipped. Costs 5 energy.", 
	"Overdrive Gears", "Traveling in the same direction for 4 or more turns will cause you to move 2 spaces every step instead of 1.", 
	"Execute", "A melee critical hit delivered while having a 1 handed ranged weapon equipped will instantly kill your opponent if the max damage roll for your ranged weapon is greater than the enemies remaining HP.", 
	"Hammer Shot", "Ranged attacks fire 2 shots instead of 1, at 1/2 ranged skill each.", 
	"Map Hacking", "The location of all interactable map tiles on a floor are always known.", 
	"Zone of Control", "Automatically melee attack any enemy that moves into a square adjacent to you, unless you moved from the same square the previous turn.", 
	"Flanking", "Automatically attack an enemy if you move from one square that is adjacent to them to another.", 
	"Punish", "You are guaranteed to land an attack on an enemy that hit you last turn. This effect is indicated by a blue background.", 
	"Crippling Shot", "Any unit you crit has a 50% chance to fail to act next turn.", 
	"Covering Fire", "Holding Shift while firing with [f] will cause target to become unable to attack or move for that turn and the following. Ranged skill is reduced to 1/4th for that attack.", 
	"Riposte", "Dodging an attack will allow you to automatically counterattack.", 
	"Respirocyte Injectors", "Increase your max HP by 50%.", 
	"Failsafe Circulatory System", "If you recieve fatal damage you will be restored to full HP, and gain 25% additional max HP and 5 crit resistance. Can only occur once.", 
	"Headshot Protocol", "The amount you need to roll to hit an opponent is doubled but the number of crit die is calculated normally and then doubled.", 
	"Kneeling Position", "Shift + Wait will use a turn to enter a kneeling shooting position, which adds 5 to ranged to-hit and ranged evasion but subtracts 5 from melee evasion. Shift + Wait again will leave the position without using a turn.", 
	"Prone Position", "CTRL + Wait will use a turn to enter a prone shooting position, which adds 10 to ranged to-hit and ranged evasion but subtracts 10 from melee evasion. CTRL + Wait again will use a turn to leave the position.", 
	"Active Camo System", "CTRL + Direction will add 20 to Stealth rolls for the next turn, but subtract 10 from Perception.", 
	"Mic System", "Roll Perception checks against unseen enemys. If absolute distance < (perception roll - stealth) enemies are marked with a [*]. If (perception roll - stealth) > 15 [*] enemy type becomes known.",  // probably reqrite this
	"Night Vision System", "FOV radius increases by 50%.", 
	"Target Recognition Subroutine", "Can see units up to Perception/5 spaces outside your FOV.", 
	"Door Hack Subroutine", "If you are adjacent to a closed door you can see enemies through it as if it was open. No change to enemy vision.", 
	"Camera System", "Add Perception/4 to crit resistance. Also Eliminate Perception penalties from all armor and Evasion penalties from helmets.", 
	"Target Prediction Subroutine", "The number of turns you have spent attacking an enemy is subtracted from their dodge rolls, up to a maximum of your Perception.", 
	"Welding System", "Pressing CTRL+Direction of adjacent door will weld it shut, or unweld if already welded. Costs 5 energy.", 
	"Assassination Protocol", "Add your stealth score to your to-hit roll when attacking an unaware enemy", 
	"Plasma Cutter", "[x] followed by Direction of adjacent wall will remove a square of wall.", 
	"Observation Protocol", "Gain a bonus to stealth rolls against enemies equal to the number of turns they have beem in view while unaware of your presence, to a max of Perception/2.", 
	"Impetuous Offense", "Melee to-hit roll becomes a d(20+2*skill-penalty) rather than d20+skill-penalty.", 
	"Facile Dodging", "You take the larger of 2 dodge rolls to determine whether an attack lands and attackers take the larger of 2 damage rolls. The first dodge roll is still used to determine whether an attack crits.",  // this is probably still imba
	"Silencer", "Any enemy that you attack becomes unable to call for help or radio its allies.", 
	"Radio Jamming", "Enemies with radios are unable to use them.",
	"Knockback", "Critical hits knock the enemy back a space."
	};
	while (key.vk != TCODK_ESCAPE) {
		TCOD_console_clear(NULL);
		inc_curs(key, &cursx, &cursy, snum-1);
		print_curs(cursx, cursy);
		for (int i=0; i<snum; i++) {
			int flag;
			if (cursy>31*2) flag=player.ownedAbilities1;
			else flag=player.ownedAbilities2;
			TCOD_console_print(NULL, 2, i, "%c%s%c", flag & 1<<i ? TCOD_COLCTRL_2 : TCOD_COLCTRL_1, skills[i*2], TCOD_COLCTRL_STOP);
		}
		TCOD_console_print_rect(NULL, 50, 0, MAPX+HUDX, MAPY+MSGY, "%s", skills[cursy*2+1]);
		TCOD_console_print_rect(NULL, 0, snum+1, MAPX+HUDX, MAPY+MSGY, "Exp:%d\nExp Required:%d", player.exp, exp_req);
		TCOD_console_flush();
		key = TCOD_console_wait_for_keypress(true);
		int flagSet;
		int mod = 0;
		if (cursy<31*2) {
			flagSet=player.ownedAbilities1;
			mod = cursy;
		} else {
			flagSet=player.ownedAbilities2;
			mod = cursy - 31;
		}
		if (key.vk==TCODK_ENTER && player.exp >= exp_req && !(flagSet & 1<<mod) ) { // 1<<n = 2^n
			if (cursy<31*2)	player.ownedAbilities1 |= 1<<mod;
			else player.ownedAbilities2 |= 1<<mod;
			if (1<<cursy != A1_MED2) player.activeAbilities1 |= 1<<mod;
			if (1<<cursy == A1_MED1) {
				player.mhp += (int)player.mhp*0.5;
			} else if (1<<cursy == A1_NIGHT_VISION) {
				player.fov += (int)(player.fov*0.5);
			}
			player.exp -= exp_req;
			player.abilities += 1;
			return;
		}
	}
}

bool leap(struct Unit *unit, int dx, int dy) {
	int odx = dx; int ody = dy;
	int pcost = 5;
	if (dx != 0 || dy != 0) {
		dx = dx * 2;
		dy = dy * 2;
		if (find_unit_ind(player.x+dx, player.y+dy) == -1 && map[player.x+dx][player.y+dy].can_walk && unit->cpow>=pcost) {
			unit->cpow-=pcost;
			unit->x+=dx;
			unit->y+=dy;
			unit->lt_dx = odx;
			unit->lt_dy = ody;
			return true;
		} 
	}
	return false;
}

bool dash_or_smash(struct Unit *unit, int dx, int dy) {
	if (find_unit_ind(player.x+dx, player.y+dy)==-1) {
		return dash(unit, dx, dy);
	} else {
		return punt(unit, dx, dy, 6);
	}
}

bool dash(struct Unit *unit, int dx, int dy) {
	int odx = dx; int ody = dy;
	int pcost = 5;
	int range = 6;
	if (( dx != 0 || dy != 0) && unit->activeAbilities1 & A1_DASH && unit->cpow>=pcost) {
		for (int i=0; i<range; i++) {
			if (find_unit_ind(player.x+dx, player.y+dy) == -1 && map[player.x+dx][player.y+dy].can_walk && unit->cpow>=pcost) {
				unit->x+=dx;
				unit->y+=dy;
			} else {
				unit->cpow-=pcost;
				unit->lt_dx = odx;
				unit->lt_dy = ody;
				return true;
			}
		}
		unit->cpow-=pcost;
		unit->lt_dx = odx;
		unit->lt_dy = ody;
		return true;
	}
	return false;
}

bool punt(struct Unit *unit, int dx, int dy, int range) { // just get rid of this
	int pcost = 0;
	if (range>3) pcost = 5;
	int ind = find_unit_ind(player.x+dx, player.y+dy);
	if (( dx != 0 || dy != 0) && unit->cpow>=pcost && ind > -1) {
		unit_list[ind].turnspent = true;
		for (int i=0; i<range; i++) {
			if (map[unit_list[ind].x+dx][unit_list[ind].y+dy].can_walk && find_unit_ind(unit_list[ind].x+dx, unit_list[ind].y+dy) == -1) {
				unit_list[ind].x+=dx;
				unit_list[ind].y+=dy;
			} else if (find_unit_ind(unit_list[ind].x+dx, unit_list[ind].y+dy) > -1 || !map[unit_list[ind].x+dx][unit_list[ind].y+dy].can_walk) {
				unit->cpow-=pcost;
				struct Weapon fist; fist.dmgdie = i; fist.dmgsides = player.melee; fist.AP = 0;
				if (unit->mwep.nodisarm) {
					fist.dmgsides = fist.dmgsides + unit->mwep.dmgsides;
					fist.dmgdie = fist.dmgdie + unit->mwep.dmgdie;
				}
				unit_list[ind].noevade = true;
				unit_list[ind].stuck = true;
				unit_list[ind].sturns = 2;
				take_dmg(unit, &unit_list[ind], fist);
				int obstacle = find_unit_ind(unit_list[ind].x+dx, unit_list[ind].y+dy);
				if (obstacle != -1) {
					take_dmg(unit, &unit_list[obstacle], fist);
				}
				return true;
			}
		}
		unit->cpow-=pcost;
		return true;
	}
	return false;
}
