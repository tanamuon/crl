#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/libtcod.h"
#include "main.h"
#include "jsmn.h"

int ui = -1;
int wi = -1;
int mi = -1;
int ai = -1;
int ti = -1;
int sui = -1;
int smi = -1;
int smj = 0;

void read_file(char *fp, char *fc) {
	FILE *file;
	char c;
	int rfindex = 0;
	file = fopen(fp, "r");
	while((c = fgetc(file)) != EOF) {
		fc[rfindex] = c;
		rfindex++;
	}
	fc[rfindex] = '\0';
	fclose(file);
}

void JSONparse(char JSON_STRING[100000], void (*callback)(char *, char *)) {
	char key[2048];
	char val[2048];
	jsmn_parser p;
	jsmntok_t t[512];
	jsmn_init(&p);
	int r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, SIZE(t));
	for (int i=1; i<abs(r); i++) {
		jsmntok_t jkey = t[i];
		jsmntok_t jval = t[i+1];
		int klen = jkey.end - jkey.start;
		int vlen = jval.end - jval.start;
		for (int j=0; j<klen; j++) {
			key[j] = JSON_STRING[jkey.start + j];
		}
		for (int k=0; k<vlen; k++) {
			val[k] = JSON_STRING[jval.start + k];
		}
		key[klen] = '\0';
		val[vlen] = '\0';
		callback(key, val);
		i++;
	}
	
}

void mJSONparse(char JSON_STRING[100000], void (*callback)(char *, char *)) {
	char key[2048];
	char val[2048];
	jsmn_parser p;
	jsmntok_t t[512];
	jsmn_init(&p);
	int r = MAPX*MAPY;
	for (int i=1; i<abs(r); i++) {
		jsmntok_t jkey = t[i];
		jsmntok_t jval = t[i+1];
		int klen = jkey.end - jkey.start;
		int vlen = jval.end - jval.start;
		for (int j=0; j<klen; j++) {
			key[j] = JSON_STRING[jkey.start + j];
		}
		for (int k=0; k<vlen; k++) {
			val[k] = JSON_STRING[jval.start + k];
		}
		key[klen] = '\0';
		val[vlen] = '\0';
		callback(key, val);
		i++;
	}
	
}
 
void tParse(char *key, char *val) {
	if (strcmp(key,"id") == 0) {
		ti++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"id") == 0) {
		map_data.tile_types[ti].id = val[0];
	} else if (strcmp(key,"name") == 0) {
		memmove(map_data.tile_types[ti].name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"desc") == 0) {
		memmove(map_data.tile_types[ti].desc, buffer, bsize);
	} else if (strcmp(key,"c") == 0) {
		map_data.tile_types[ti].c = val[0];
	}
}

void uParse(char *key, char *val) {
	if (strcmp(key,"name") == 0) {
		ui++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(unit_types[ui].name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"desc") == 0) {
		memmove(unit_types[ui].desc, buffer, bsize);
	} else if (strcmp(key,"c") == 0) {
		unit_types[ui].c = val[0];
	} else if (strcmp(key,"x") == 0) { 
		unit_types[ui].x = atoi(val);
	} else if (strcmp(key,"y") == 0) { 
		unit_types[ui].y = atoi(val);
	} else if (strcmp(key,"fac") == 0) { 
		unit_types[ui].fac = atoi(val);
	} else if (strcmp(key,"mindepth") == 0) {
		unit_types[ui].mindepth = atoi(val);
	} else if (strcmp(key,"maxdepth") == 0) {
		unit_types[ui].maxdepth = atoi(val);
	} else if (strcmp(key,"spawnweight") == 0) {
		unit_types[ui].spawnweight = atof(val);
	} else if (strcmp(key,"r") == 0) {
		unit_types[ui].r = atof(val);
	} else if (strcmp(key,"g") == 0) {
		unit_types[ui].g = atof(val);
	} else if (strcmp(key,"b") == 0) {
		unit_types[ui].b = atof(val);	
	} else if (strcmp(key,"mhp") == 0) {
		unit_types[ui].mhp = atoi(val);
	} else if (strcmp(key,"chp") == 0) {
		unit_types[ui].chp = atoi(val);
	} else if (strcmp(key,"ev") == 0) {
		unit_types[ui].ev = atoi(val);
	} else if (strcmp(key,"fov") == 0) {
		unit_types[ui].fov = atoi(val);
	} else if (strcmp(key,"b_arm.acdie") == 0) {
		unit_types[ui].b_arm.acdie = atoi(val);
	} else if (strcmp(key,"b_arm.acsides") == 0) {
		unit_types[ui].b_arm.acsides = atoi(val);
	} else if (strcmp(key,"atk_skill") == 0) {
		unit_types[ui].melee = atoi(val);
		unit_types[ui].ranged = atoi(val);
	} else if (strcmp(key,"wep.is_ranged") == 0) {
		unit_types[ui].wep.is_ranged = atoi(val);
	} else if (strcmp(key,"wep.dmgdie") == 0) {
		unit_types[ui].wep.dmgdie = atoi(val);
	} else if (strcmp(key,"wep.dmgsides") == 0) {
		unit_types[ui].wep.dmgsides = atoi(val);
	} else if (strcmp(key,"per") == 0) {
		unit_types[ui].per = atoi(val);
	} else if (strcmp(key,"stealth") == 0) {
		unit_types[ui].stealth = atoi(val);
	} else if (strcmp(key,"lit") == 0) {
		unit_types[ui].lit = atoi(val);
	} else if (strcmp(key,"immobilizer") == 0) {
		unit_types[ui].immobilizer = atoi(val);
	} else if (strcmp(key,"comms") == 0) {
		unit_types[ui].comms = atoi(val);
	} else if (strcmp(key,"inv.stim") == 0) {
		unit_types[ui].inv.stim = atoi(val);
	} else if (strcmp(key,"inv.berserk") == 0) {
		unit_types[ui].inv.berserk = atoi(val);
	} else if (strcmp(key,"ability1") == 0) {
		int v = 1<<atoi(val); // for some reason |= atoi(val) didn't work
		unit_types[ui].ownedAbilities1 |= v;
		unit_types[ui].activeAbilities1 |= v;
	} else if (strcmp(key,"ability2") == 0) {
		int v = 1<<atoi(val);
		unit_types[ui].ownedAbilities2 |= v;
		unit_types[ui].activeAbilities2 |= v;
	}
}

void SuParse(char *key, char *val) {
	if (strcmp(key,"name") == 0) {
		sui++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(unit_list[sui].name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"desc") == 0) {
		memmove(unit_list[sui].desc, buffer, bsize);
	} else if (strcmp(key,"c") == 0) {
		unit_list[sui].c = val[0];
	} else if (strcmp(key,"fac") == 0) { 
		unit_list[sui].fac = atoi(val);
	} else if (strcmp(key,"mindepth") == 0) {
		unit_list[sui].mindepth = atoi(val);
	} else if (strcmp(key,"maxdepth") == 0) {
		unit_list[sui].maxdepth = atoi(val);
	} else if (strcmp(key,"spawnweight") == 0) {
		unit_list[sui].spawnweight = atof(val);
	} else if (strcmp(key,"r") == 0) {
		unit_list[sui].r = atof(val);
	} else if (strcmp(key,"g") == 0) {
		unit_list[sui].g = atof(val);
	} else if (strcmp(key,"b") == 0) {
		unit_list[sui].b = atof(val);	
	} else if (strcmp(key,"mhp") == 0) {
		unit_list[sui].mhp = atoi(val);
	} else if (strcmp(key,"chp") == 0) {
		unit_list[sui].chp = atoi(val);
	} else if (strcmp(key,"ev") == 0) {
		unit_list[sui].ev = atoi(val);
	} else if (strcmp(key,"fov") == 0) {
		unit_list[sui].fov = atoi(val);
	} else if (strcmp(key,"b_arm.acdie") == 0) {
		unit_list[sui].b_arm.acdie = atoi(val);
	} else if (strcmp(key,"b_arm.acsides") == 0) {
		unit_list[sui].b_arm.acsides = atoi(val);
	} else if (strcmp(key,"atk_skill") == 0) {
		unit_list[sui].melee = atoi(val);
		unit_list[sui].ranged = atoi(val);
	} else if (strcmp(key,"wep.is_ranged") == 0) {
		unit_list[sui].wep.is_ranged = atoi(val);
	} else if (strcmp(key,"wep.dmgdie") == 0) {
		unit_list[sui].wep.dmgdie = atoi(val);
	} else if (strcmp(key,"wep.dmgsides") == 0) {
		unit_list[sui].wep.dmgsides = atoi(val);
	} else if (strcmp(key,"per") == 0) {
		unit_list[sui].per = atoi(val);
	} else if (strcmp(key,"stealth") == 0) {
		unit_list[sui].stealth = atoi(val);
	} else if (strcmp(key,"lit") == 0) {
		unit_list[sui].lit = atoi(val);
	}
}

void wParse(char *key, char *val) {
	if (strcmp(key,"name") == 0) {
		wi++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(wep_types[wi].name, buffer, bsize);
	} else if (strcmp(key,"desc") == 0) {
		memmove(wep_types[wi].desc, buffer, bsize);
	} else if (strcmp(key,"is_ranged") == 0) {
		wep_types[wi].is_ranged = atoi(val);
	} else if (strcmp(key,"projpershot") == 0) {
		wep_types[wi].projpershot = atoi(val);
	} else if (strcmp(key,"acc") == 0) {
		wep_types[wi].acc = atoi(val);
	} else if (strcmp(key,"acc_pen") == 0) {
		wep_types[wi].acc_pen = atoi(val);
	} else if (strcmp(key,"dmgdie") == 0) {
		wep_types[wi].dmgdie = atoi(val);
	} else if (strcmp(key,"dmgsides") == 0) {
		wep_types[wi].dmgsides = atoi(val);
	} else if (strcmp(key,"exp") == 0) {
		wep_types[wi].exp = atoi(val);
	} else if (strcmp(key,"tagger") == 0) {
		wep_types[wi].tagger = atoi(val);
	} else if (strcmp(key,"bipod") == 0) {
		wep_types[wi].bipod = atoi(val);
	} else if (strcmp(key,"set_req") == 0) {
		wep_types[wi].set_req = atoi(val);
	} else if (strcmp(key,"AP") == 0) {
		wep_types[wi].AP = atoi(val);
	} else if (strcmp(key,"elec") == 0) {
		wep_types[wi].elec = atoi(val);
	} else if (strcmp(key,"critmod") == 0) {
		wep_types[wi].critmod = atoi(val);
	} else if (strcmp(key,"handedness") == 0) {
		wep_types[wi].handedness = atoi(val);
	} else if (strcmp(key,"nodisarm") == 0) {
		wep_types[wi].nodisarm = atoi(val);
	} else if (strcmp(key,"knockback") == 0) {
		wep_types[wi].knockback = atoi(val);
	}
}

void mParse(char *key, char *val) {
	if (strcmp(key,"name") == 0) {
		mi++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(mod_types[mi].name, buffer, bsize);
	} else if (strcmp(key,"desc") == 0) {
		memmove(mod_types[mi].desc, buffer, bsize);
	} else if (strcmp(key,"is_ranged") == 0) {
		mod_types[mi].is_ranged = atoi(val);
	} else if (strcmp(key,"acc") == 0) {
		mod_types[mi].acc = atoi(val);
	} else if (strcmp(key,"acc_pen") == 0) {
		wep_types[mi].acc_pen = atoi(val);
	} else if (strcmp(key,"dmgdie") == 0) {
		mod_types[mi].dmgdie = atoi(val);
	} else if (strcmp(key,"dmgsides") == 0) {
		mod_types[mi].dmgsides = atoi(val);
	} else if (strcmp(key,"exp") == 0) {
		mod_types[mi].exp = atoi(val);
	} else if (strcmp(key,"tagger") == 0) {
		mod_types[mi].tagger = atoi(val);
	} else if (strcmp(key,"bipod") == 0) {
		mod_types[mi].bipod = atoi(val);
	} else if (strcmp(key,"elec") == 0) {
		mod_types[mi].elec = atoi(val);
	} else if (strcmp(key,"critmod") == 0) {
		mod_types[mi].critmod = atoi(val);
	} else if (strcmp(key,"handedness") == 0) {
		mod_types[mi].handedness = atoi(val);
	} else if (strcmp(key,"dupes") == 0) {
		mod_types[mi].dupes = atoi(val);
	} else if (strcmp(key,"AP") == 0) {
		mod_types[mi].AP = atoi(val);
	} else if (strcmp(key,"suppressed") == 0) {
		mod_types[mi].suppressed = atoi(val);
	}
	mod_types[mi].id = mi;
}

void pParse(char *key, char *val) {
	int bsize = strlen(val)+1;
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
	} else if (strcmp(key,"inv.berserk") == 0) {
		player.inv.berserk = atoi(val);
	} else if (strcmp(key,"inv.noisemaker") == 0) {
		player.inv.noisemaker = atoi(val);
	} else if (strcmp(key,"inv.flare") == 0) {
		player.inv.flare = atoi(val);
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

/*void sParse(char *key, char *val) {
	int bsize = strlen(val)+1;
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
}*/

void aParse(char *key, char *val) {
	if (strcmp(key,"name") == 0) {
		ai++;
	}
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"name") == 0) {
		memmove(armor_types[ai].name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"type") == 0) {
		armor_types[ai].type = *val;
	} else if (strcmp(key,"desc") == 0) {
		memmove(armor_types[ai].desc, buffer, bsize);
	} else if (strcmp(key,"acdie") == 0) {
		armor_types[ai].acdie = atoi(val);
	} else if (strcmp(key,"acsides") == 0) {
		armor_types[ai].acsides = atoi(val);
	} else if (strcmp(key,"crit_res") == 0) {
		armor_types[ai].crit_res = atoi(val);
	} else if (strcmp(key,"ev_pen") == 0) {
		armor_types[ai].ev_pen = atoi(val);
	} else if (strcmp(key,"melee_pen") == 0) {
		armor_types[ai].melee_pen = atoi(val);
	} else if (strcmp(key,"acc_pen") == 0) {
		armor_types[ai].acc_pen = atoi(val);
	} else if (strcmp(key,"stealth_pen") == 0) {
		armor_types[ai].stealth_pen = atoi(val);
	} else if (strcmp(key,"per_pen") == 0) {
		armor_types[ai].per_pen = atoi(val);
	} else if (strcmp(key,"exp") == 0) {
		armor_types[ai].exp = atoi(val);
	}
}

void uCount(char *key, char *val) {
	if (strcmp(key, "name") == 0) {
		utypes++;
	}
}

void wCount(char *key, char *val) {
	if (strcmp(key, "name") == 0) {
		wtypes++;
	}
}

void mCount(char *key, char *val) {
	if (strcmp(key, "name") == 0) {
		mtypes++;
	}
}

void aCount(char *key, char *val) {
	if (strcmp(key, "name") == 0) {
		atypes++;
	}
}

void tCount(char *key, char *val) {
	if (strcmp(key, "name") == 0) {
		map_data.tnum++;
	}
}

extern void read_json(char *filename, int strsize, void (*callback)(char *, char *, int *, int *, int) );
extern void gstart_story(char *key, char *val, int *i, int *j, int bsize);
extern void gstart_player(char *key, char *val, int *i, int *j, int bsize);

void gen_json_lists() {
	char JSON_STRING1[10000];
	read_file("../json/units.json", JSON_STRING1);
	JSONparse(JSON_STRING1, uCount);
	unit_types = calloc(utypes, sizeof(struct Unit));
	JSONparse(JSON_STRING1, uParse);
	
	char JSON_STRING2[10000];
	read_file("../json/weapons.json", JSON_STRING2);
	JSONparse(JSON_STRING2, wCount);
	wep_types = calloc(wtypes, sizeof(struct Weapon));
	JSONparse(JSON_STRING2, wParse);
	
	char JSON_STRING3[10000];
	read_file("../json/player.json", JSON_STRING3);
	JSONparse(JSON_STRING3, pParse);
	
	/*char JSON_STRING4[10000];
	read_file("../json/story.json", JSON_STRING4);
	JSONparse(JSON_STRING4, sParse);*/
	
	//read_json("../json/player.json", 1024, gstart_player);
	read_json("../json/story.json", 1024, gstart_story);
	
	char JSON_STRING5[10000];
	read_file("../json/mods.json", JSON_STRING5);
	JSONparse(JSON_STRING5, mCount);
	mod_types = calloc(mtypes, sizeof(struct Weapon));
	JSONparse(JSON_STRING5, mParse);
	
	char JSON_STRING6[10000];
	read_file("../json/armor.json", JSON_STRING6);
	JSONparse(JSON_STRING6, aCount);
	armor_types = calloc(atypes, sizeof(struct Armor));
	JSONparse(JSON_STRING6, aParse);
	
	char JSON_STRING7[10000];
	read_file("../json/tiles.json", JSON_STRING7);
	JSONparse(JSON_STRING7, tCount);
	map_data.tile_types = calloc(map_data.tnum, sizeof(struct Tile));
	//JSONparse(JSON_STRING7, tParse);
}

void SmParse(char *key, char *val) {
	if (key[0] =='c') {
		smi++;
	}
	if (smj == MAPY - 1) {
		smj = 0;
	}
	map[smi][smj].c = '&';
	int bsize = strlen(val)+1;
	char buffer[bsize];
	val[bsize-1] = '\0';
	memmove(buffer, val, bsize);
	if (strcmp(key,"c") == 0) {
		map[smi][smj].c = '&';
	} else if (strcmp(key,"r") == 0) {
		map[smi][smj].r = atoi(val);
	} else if (strcmp(key,"g") == 0) {
		map[smi][smj].g = atoi(val);
	} else if (strcmp(key,"b") == 0) {
		map[smi][smj].b = atoi(val);
	} else if (strcmp(key,"can_walk") == 0) {
		map[smi][smj].can_walk = atoi(val);
	} else if (strcmp(key,"is_transparent") == 0) {
		map[smi][smj].is_transparent = atoi(val);
	} else if (strcmp(key,"filled") == 0) {
		map[smi][smj].filled = atoi(val);
	} else if (strcmp(key,"is_explored") == 0) {
		map[smi][smj].is_explored = atoi(val);
	} else if (strcmp(key,"lit") == 0) {
		map[smi][smj].tlit = atoi(val);
	} else if (strcmp(key,"door") == 0) {
		map[smi][smj].door = atoi(val);
	} else if (strcmp(key,"down_stair") == 0) {
		map[smi][smj].down_stair = atoi(val);
	} else if (strcmp(key,"terminal") == 0) {
		map[smi][smj].terminal = atoi(val);
	} else if (strcmp(key,"med") == 0) {
		map[smi][smj].med = atoi(val);	
	} else if (strcmp(key,"power") == 0) {
		map[smi][smj].power = atoi(val);	
	} else if (strcmp(key,"printer") == 0) {
		map[smi][smj].printer = atoi(val);	
	} else if (strcmp(key,"effect") == 0) {
		map[smi][smj].effect = atoi(val);	
	} else if (strcmp(key,"teffect") == 0) {
		map[smi][smj].teffect = atoi(val);	
	} else if (strcmp(key,"cover") == 0) {
		map[smi][smj].cover = atoi(val);
	} else if (strcmp(key,"concealment") == 0) {
		map[smi][smj].concealment = atoi(val);
	} else if (strcmp(key,"name") == 0) {
		memmove(map[smi][smj].name, buffer, bsize); // memmove apparently doesn't null terminate in all cases
	} else if (strcmp(key,"desc") == 0) {
		memmove(map[smi][smj].desc, buffer, bsize);
	}
}

/*void load_game(int num) {
	//char JSON_STRING1[100000000];
	//read_file("../save/units.json", JSON_STRING1);
	//JSONparse(JSON_STRING1, SuParse);
	
	char JSON_STRING2[100000];
	read_file("../save/map.json", JSON_STRING2);
	mJSONparse(JSON_STRING2, SmParse);
}*/
