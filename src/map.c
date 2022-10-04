#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/libtcod.h"
#include "system.h"
#include "units.h"
#include "main.h"

struct Tile printer; struct Tile med; struct Tile power; struct Tile forge; struct Tile plant; struct Tile locker; struct Tile tabl;
struct Tile crop1; struct Tile crop2; struct Tile crop3; struct Tile wall; struct Tile sfloor; struct Tile door;

struct MapData map_data;

void init_tiles() {
	printer.can_walk = 1;
	printer.is_transparent = 1;
	printer.wall = 0;
	printer.filled = 1;
	printer.printer = true;
	printer.c = 182;
	printer.r = 255;
	printer.g = 255;
	printer.b = 255;
	printer.cover = 3;
	printer.concealment = 5;
	
	forge.can_walk = 1;
	forge.is_transparent = 1;
	forge.wall = 0;
	forge.filled = 1;
	forge.forge = true;
	forge.c = '%';
	forge.r = 155;
	forge.g = 155;
	forge.b = 155;
	forge.cover = 3;
	forge.concealment = 5;
	
	med.can_walk = 1;
	med.is_transparent = 1;
	med.wall = 0;
	med.filled = 1;
	med.lit = 0;//1;
	med.med = true;
	med.c = 234;
	med.r = 255;
	med.g = 130;
	med.b = 130;
	med.cover = 3;
	med.concealment = 5;
	
	power.can_walk = 1;
	power.is_transparent = 1;
	power.wall = 0;
	power.filled = 1;
	power.power = true;
	power.c = 156;
	power.r = 255;
	power.g = 255;
	power.b = 0;
	power.cover = 3;
	power.concealment = 5;
	
	plant.can_walk = 1;
	plant.c = '&';
	plant.is_transparent = 0;
	plant.wall = 0;
	plant.filled = 1;
	plant.r = 0;
	plant.g = 128;
	plant.b = 0;
	plant.cover = 2;
	plant.concealment = 5;
	memmove(plant.name, "Peace Lily", MAXNSIZE);
	
	locker.can_walk = 1;
	locker.c = '[';
	locker.is_transparent = 0;
	locker.wall = 0;
	locker.filled = 1;
	locker.locker = true;
	locker.r = 155;
	locker.g = 155;
	locker.b = 155;
	locker.cover = 2;
	locker.concealment = 5;
	memmove(locker.desc, "Indoor locker.", MAXNSIZE);
	
	//"table" is reserved
	tabl.can_walk = 1;
	tabl.c = '=';
	tabl.is_transparent = 0;
	tabl.wall = 0;
	tabl.filled = 1;
	tabl.r = 200;
	tabl.g = 155;
	tabl.b = 155;
	tabl.cover = 2;
	tabl.concealment = 8;
	
	crop1.c = ':';
	crop1.r = 200;
	crop1.g = 0;
	crop1.b = 155;
	crop1.is_transparent = 0;
	crop1.cover = 0;
	crop1.concealment = 5;
	
	crop2.c = '}';
	crop2.r = 255;
	crop2.g = 255;
	crop2.b = 0;
	crop2.is_transparent = 0;
	crop2.cover = 0;
	crop2.concealment = 5;
						
	crop3.c = '%';
	crop3.r = 155;
	crop3.g = 0;
	crop3.b = 0;
	crop3.is_transparent = 0;
	crop3.cover = 0;
	crop3.concealment = 5;
	
	wall.r = 170;
	wall.g = 170;
	wall.b = 255;
	wall.can_walk = 0;
	wall.is_transparent = 0;
	wall.filled = 0;
	wall.is_explored = 0;
	wall.wall = true;
	wall.lit = false;
	wall.down_stair = false;
	wall.terminal = false;
	wall.med = false;
	wall.power = false;
	wall.printer = false;
	wall.forge = false;
	wall.c = '+';
	wall.effect = 0;
	wall.teffect = 0;
	wall.cover = 0;
	wall.concealment = 0;
	memmove(wall.name, "Station wall", MAXNSIZE);
	memmove(wall.desc, "It doesn't look like you can cut through this.", DESCSIZE);
	
	sfloor.r = 170;
	sfloor.g = 170;
	sfloor.b = 255;
	sfloor.can_walk = 1;
	sfloor.is_transparent = 1;
	sfloor.filled = 1;
	sfloor.is_explored = 0;
	sfloor.wall = false;
	sfloor.lit = false;
	sfloor.down_stair = false;
	sfloor.terminal = false;
	sfloor.med = false;
	sfloor.power = false;
	sfloor.printer = false;
	sfloor.c = '.';
	sfloor.effect = 0;
	sfloor.teffect = 0;
	sfloor.cover = 0;
	sfloor.concealment = 0;
	memmove(sfloor.name, "Station floor", MAXNSIZE);
	memmove(sfloor.desc, "It doesn't look like you can cut through this.", DESCSIZE);
	
	door.is_transparent = 0;
	door.can_walk = 1;
	door.door = 1;
	door.c = '#';
	door.r = 200;
	door.g = 200;
	door.b = 200;
	
}

void fill_wall() {
	init_tiles();
	for (int i=0;i<MAPX;i++) {
		for (int j=0;j<MAPY;j++) {
			map[i][j] = wall;
			//map[i][j].c = map_data.tile_types[0].c;
			printf("%s \n", map_data.tile_types[0].c);
		}
	}
}

bool fill_room(struct Leaf *leaf, int rsize_x, int rsize_y) { // make sep functions for diff room types?
	bool error = false;
	int x1 = leaf->x1 + 1; 
	int x2 = leaf->x1+rsize_x - 1;
	int y1 = leaf->y1 + 1;
	int y2 = leaf->y1+rsize_y - 1;
	for (int i=x1; i<=x2; i++) {
		for (int j=y1; j<=y2; j++) {
			if (map[i][j].filled) {	
				map[i][j].c = 'x';
				error = true;
			} else {
				//char r = leaf->rdepth + '0';
				map[i][j] = sfloor;
			}
			//if (leaf->rtype == 1) map[i][j].lit = true; // medbay would have backup generator
		}
	}
	if (leaf->rtype == 1 && leaf->rtypes[0] < 1) { // medbay, needs more stuff
		map[rand()%(rsize_x - 1) + x1][rand()%(rsize_y - 1) + y1] = med;
		leaf->rtypes[0] = leaf->rtypes[0] + 1;
	} else if (leaf->rtype == 2) { // workshop, needs more stuff (add item damage that is repaired here (for energy)?)
		map[rand()%(rsize_x - 1) + x1][rand()%(rsize_y - 1) + y1] = forge;
		
		// what is below might be the worst thing i've ever written.
		bool which[2] = {rand()%2, rand()%2};
		for (int a=x1*which[0] + y1*!which[0]; a<=x2*which[0] + y2*!which[0]; a++) {
	map[a*which[0]+!which[0]*(x1 + which[1]*(rsize_x-2))][a*!which[0]+which[0]*(y1+which[1]*(rsize_y-2))] = locker;
		}
	} else if (leaf->rtype == 3) { // aeroponics lab, needs a bit of tweaking
			struct Tile crops[3] = {crop1, crop2, crop3};
			for (int k = x1+1; k < x2; k+=1+(rsize_x>rsize_y)) {
				for (int l = y1+1; l < y2; l+=1+(rsize_y>=rsize_x)) {
					map[k][l] = crops[rand()%3];
				}
			}
	} else if (leaf->rtype == 4) { // room with 3D printers, should flesh out more
		for (int z=0; z<rand()%3; z++)	map[rand()%(rsize_x - 1) + x1][rand()%(rsize_y - 1) + y1] = printer;
	} else if (leaf->rtype == 5) { // meeting room, need to make break rooms, parks - whole level maybe, bars, etc
		for (int k = x1+2; k <= x2-2; k++) {
			for (int l = y1+2; l <= y2-2; l++) {
				map[k][l] = tabl;
			}
		}
		int x[4] = {x1, x1, x2, x2}; 
		int y[4] = {y1, y2, y1, y2};
		for (int m=0; m<4; m++) map[x[m]][y[m]] = plant;
	}
	return error;
}

/*void fill_edges() {
	for (int i=0; i<MAPX; i++) {
		map[i][0].can_walk = 0;
		map[i][0].is_transparent = 0;
		map[i][0].filled = 0;
		map[i][0].is_explored = 0;
		map[i][0].c = '+';
		map[i][MAPY-1].can_walk = 0;
		map[i][MAPY-1].is_transparent = 0;
		map[i][MAPY-1].filled = 0;
		map[i][MAPY-1].is_explored = 0;
		map[i][MAPY-1].c = '+';
	}
	for (int j=0; j<MAPY; j++) {
		map[0][j].can_walk = 0;
		map[0][j].is_transparent = 0;
		map[0][j].filled = 0;
		map[0][j].is_explored = 0;
		map[0][j].c = '+';
		map[MAPX-1][j].can_walk = 0;
		map[MAPX-1][j].is_transparent = 0;
		map[MAPX-1][j].filled = 0;
		map[MAPX-1][j].is_explored = 0;
		map[MAPX-1][j].c = '+';
	}
}*/

void gen_leaf(struct Leaf *leaf) {
	leaf->rtype = 0;
	leaf->rtypes[0] = 0;
	leaf->rdepth++;
	struct Leaf left;
	struct Leaf right;
	left.rdepth = leaf->rdepth;
	right.rdepth = leaf->rdepth;
	int split = rand()%2;
	int xl = 0; int yl = 0;
	if (split == 0) {
		xl = leaf->xsize/2;
		yl = leaf->ysize;
		left.x1 = leaf->x1;
		left.y1 = leaf->y1;
		right.x1 = leaf->x1+xl;
		right.y1 = leaf->y1;

	} else if (split == 1) {
		xl = leaf->xsize; 
		yl = leaf->ysize/2;
		left.x1 = leaf->x1;
		left.y1 = leaf->y1;
		right.x1 = leaf->x1;
		right.y1 = leaf->y1+yl;
	} else {
		printf("Error: Node not split properly\n");
	}
	left.xsize = xl; 
	left.ysize = yl; 
	right.xsize = xl;
	right.ysize = yl;
	leaf->left = &left;
	leaf->right = &right;
	int rsize_x = rand()%9+5;
	int rsize_y = rand()%9+5;
	if (rsize_x > xl) { rsize_x = xl; }
	if (rsize_y > yl) { rsize_y = yl; }
	int x1 = leaf->left->x1 + 1; 
	int y1 = leaf->left->y1 + 1;
	int x2 = leaf->right->x1 + 1;
	int y2 = leaf->right->y1 + 1;
	if (xl < 15 || yl < 15) {
		leaf->left->rtype = rand()%5+1;
		leaf->right->rtype = rand()%5+1;
		if (fill_room(leaf->left, rsize_x, rsize_y) && split == 0) {
			printf("Error: Left roomfill filled already filled square.\n");
		} else if (fill_room(leaf->right, rsize_x, rsize_y) && split == 0) {
			printf("Error: Right roomfill filled already filled square.\n");
		}
	} else {
		gen_leaf(leaf->left);
		gen_leaf(leaf->right);
	}
	if (x1 == x2) {
		int j;
		for (j = y1; j<y2; j++) {
			if (map[x1+rsize_x/2-1][j].wall || map[x1+rsize_x/2-1][j].locker) {
				map[x1+rsize_x/2-1][j] = sfloor;
				if 
				( 
				((map[x1+rsize_x/2][j+1].can_walk || map[x1+rsize_x/2-2][j+1].can_walk) ||
				(map[x1+rsize_x/2][j-1].can_walk|| map[x1+rsize_x/2-2][j-1].can_walk)) &&
				!map[x1+rsize_x/2][j].can_walk && !map[x1+rsize_x/2-2][j].can_walk
				) {
					map[x1+rsize_x/2-1][j] = door;
					ndoors++;
					doorsxy[ndoors][0] = x1+rsize_x/2-1;
					doorsxy[ndoors][1] = j;
				}
			}
		}
	} else if (y1 == y2) {
		int i;
		for (i = x1; i<x2; i++) {
			if (map[i][y1+rsize_y/2-1].wall || map[i][y1+rsize_y/2-1].locker) {
				map[i][y1+rsize_y/2-1] = sfloor;
				if 
				( 
				((map[i+1][y1+rsize_y/2].can_walk || map[i+1][y1+rsize_y/2-2].can_walk) || 
				(map[i-1][y1+rsize_y/2].can_walk || map[i-1][y1+rsize_y/2-2].can_walk)) &&
				!map[i][y1+rsize_y/2].is_transparent && !map[i][y1+rsize_y/2-2].is_transparent
				) {
					map[i][y1+rsize_y/2-1] = door;
					ndoors++;
					doorsxy[ndoors][0] = i;
					doorsxy[ndoors][1] = y1+rsize_y/2-1;
				}
			}
		}
	} else {
		printf("Error: no path drawn between nodes.\n");
	}
}

/*void bsp(int x1, int x2, int y1, int y2, int n) { // this function is utter garbage never use it
	int d = rand() % 2;
	if (n>0) {
		if (d == 0) {
			//int r = rand() % (x2+1-x1) + x1;
			printf("Horizontal Split\n");
			//printf("x: %d %d %d \ny: %d %d\n", x1, r, x2,y1,y2);
			printf("Recursion begin\n");
			if (x2-x1 > 3) {
				bsp(x1, x2/2, y1, y2, n-1);
				bsp(x2/2+1, x2, y1, y2, n-1);
			}
			printf("Recursion end\n");
		} else {
			//int r = rand() % (y2+1-y1) + y1;
			printf("Vertical Split\n");
			//printf("x: %d %d \ny: %d %d %d \n", x1,x2,y1, r, y2);
			printf("Recursion begin\n");
			if (y2-y1 > 3) {
				bsp(x1, x2, y1, y2/2, n-1);
				bsp(x1, x2, y2/2+1, y2, n-1);
			}
			printf("Recursion end\n");
		}
	}
	if (n==0) {
		printf("%d %d %d %d\n", x1, x2, y1, y2);
		for (int i=x1; i<x2; i++) {
			for (int j=y1; j<y2; j++) {
				if (map[i][j].filled) {
					printf("Error\n");
				}
				map[i][j].can_walk = 1;
				map[i][j].is_transparent = 1;
				map[i][j].c = '.';
				map[i][j].filled = true;
			}
		}
	}
}
*/
void fill_rect(int x, int y) {
	int n = rand() % MAXROOMSIZE;
	if (n<MINROOMSIZE) {
		n = MINROOMSIZE;
	}
	if (x+n < MAPX && y+n < MAPY) {
		for (int i=x;i<x+n;i++){
			for (int j=y;j<y+n;j++) {
				if (!map[i][j].filled && (x < i) && (i < x+n)	&& (y < j) && (j < y+n)) {
					map[i][j] = sfloor;
				}
			}
		}
	}
}

void dig_path(int x1, int x2, int y1, int y2) {
	int i;
	fill_rect(x1, y1);
	if (x1<x2) {
		for (i=x1; i<x2; i++) {
			map[i][y1] = sfloor;
		}
	} else {
		for (i=x1; i>x2; i--) {
			map[i][y1] = sfloor;
		}
	}
	fill_rect(x2, y1);
	int j;
	if (y1<y2) {
		for (j=y1; j<y2; j++) {
			map[x2][j] = sfloor;
		}
	} else {
		for (j=y1; j>y2; j--) {
			map[x2][j] = sfloor;
		}
	}
	fill_rect(x2, y2);
}

void pick_valid_xy(int *x, int *y) {
	bool valid_xy = false;
	int tx, ty;
	while (valid_xy == false) {
		tx = rand() % MAPX; ty = rand() % MAPY; // not sure this is the best way to do this
		if (map[tx][ty].can_walk && find_unit_ind(tx, ty) == -1) {
			*x = tx; *y = ty;
			if (map[tx][ty].can_walk == false) {
				printf("Error: Invalid xy chosen\n");
			}
			valid_xy = true;
		}
	}
}

void place_dstair() { // places both dstair and upgrade terminal, should rename
	int x, y;
	for (int i=0; i<100; i++) {
		pick_valid_xy(&x, &y);
		if (map[x][y].c == '.') {
			break;
		}
	}
	map[x][y].down_stair = true;
	map[x][y].c = '>';
	map[x][y].r = 200;
	map[x][y].g = 200;
	map[x][y].b = 200;
	int x2, y2;
	for (int i=0; i<100; i++) {
		pick_valid_xy(&x2, &y2);
		if (map[x2][y2].c == '.') {
			break;
		}
	}
	map[x2][y2].terminal = true;
	map[x2][y2].c = 245;
	map[x2][y2].r = 200;
	map[x2][y2].g = 200;
	map[x2][y2].b = 200;
}

void gen_map() {
	fill_wall();
	int x1 = rand() % MAPX; int y1 = rand() % MAPY;
	int x2 = rand() % MAPX; int y2 = rand() % MAPY;
	for (int n=0; n<ROOMNUM; n++){
		dig_path(x1, x2, y1, y2);
		x1 = x2; y1 = y2;
		x2 = rand() % MAPX; y2 = rand() % MAPY;
		dig_path(x1, x2, y1, y2);
	}
	place_dstair();
}

void tunnel() {
	fill_wall();
	int x1 = rand() % MAPX; int y1 = rand() % MAPY;
	int dx = rand()%2; int dy = rand()%2-1;
	int length = rand()%20;
	int rsize = rand()%7+3;
	int x = 0; int y = 0;
	for (int i=0; i<length; i++) {
		printf("i:%d", i);
		x = x1+dx*i; y = y1+dy*i;
		if (x > -1 && x < MAPX && y > -1 && y < MAPY) {
			map[x][y] = sfloor;
		}
	}
	for (int i=x-rsize/2; i<x+rsize/2; i++) {
		for (int j=y-rsize/2; j<y+rsize/2; j++) {
			if (i > -1 && i < MAPX && j > -1 && j < MAPY) {
				map[i][j] = sfloor;
			}
		}
	}
	//int x2 = rand() % MAPX; int y2 = rand() % MAPY;
	place_dstair();
}

void gen_tmap() {
	tmap = TCOD_map_new(MAPX, MAPY);
	for (int i=0; i<MAPX; i++) {
		for (int j=0; j<MAPY; j++) {
			bool walkable = map[i][j].can_walk; // &&find_unit_ind(i, j) == -1;
			TCOD_map_set_properties(tmap, i, j, map[i][j].is_transparent, walkable);
		}
	}
}

void tile_effects() {
	int i, j;
	for(i=0; i<MAPX; i++) {
		for(j=0; j<MAPY; j++) {
			if (map[i][j].teffect > 1) {
				map[i][j].teffect--;
			} else if (map[i][j].teffect == 1) {
				map[i][j].teffect--;
				if (map[i][j].effect == 1) {
					map[i][j].is_transparent = true;
					gen_tmap();
				} else if (map[i][j].effect == 2) {
					map[i][j].can_walk = true;
					map[i][j].c = '.';
				}
			}
		}
	}

}

void draw_map() {
	int i, j;
	TCOD_map_compute_fov(tmap, player.x, player.y, check_fov(player), true, FOV_PERMISSIVE_8);
	for(i=0; i<MAPX; i++) {
		for(j=0; j<MAPY; j++) {
			TCOD_color_t ucolor={map[i][j].r, map[i][j].g, map[i][j].b};
			if ((map[i][j].down_stair || map[i][j].terminal || map[i][j].med || map[i][j].power || map[i][j].printer) && player.activeAbilities1 & A1_POWER_MAP) {
				TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, TCOD_grey, TCOD_black);
			}
			if (TCOD_map_is_in_fov(tmap, i, j)) {
				if (map[i][j].lit) {
					TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, ucolor, TCOD_white);
				} else {
					TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, ucolor, TCOD_black);
				}
				map[i][j].is_explored = true;
			} else if (map[i][j].is_explored && !map[i][j].lit) {
				TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, ucolor, TCOD_dark_grey);
			} else if (map[i][j].is_explored && map[i][j].lit) {
				TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, ucolor, TCOD_light_grey);
			}
		}
	}
	int l, k;
	TCOD_map_compute_fov(tmap, player.x, player.y, 100, true, FOV_PERMISSIVE_8);
	for(l=0; l<MAPX; l++) {
		for(k=0; k<MAPY; k++) {
			TCOD_color_t ucolor={map[l][k].r, map[l][k].g, map[l][k].b};
			if (TCOD_map_is_in_fov(tmap, l, k) && (map[l][k].tlit || map[l][k].lit)) {
				TCOD_console_put_char_ex(NULL, l, k, map[l][k].c, ucolor, TCOD_white);
				map[l][k].is_explored = true;
			}
			map[l][k].tlit = 0;
		}
	}
	TCOD_map_compute_fov(tmap, player.x, player.y, check_fov(player), true, FOV_PERMISSIVE_8);
}

void draw_map_om() {
	TCOD_map_compute_fov(tmap, player.x, player.y, 6, true, FOV_PERMISSIVE_8);
	int i, j;
	for(i=0; i<MAPX; i++) {
		for(j=0; j<MAPY; j++) {
			TCOD_color_t ucolor={map[i][j].r, map[i][j].g, map[i][j].b};
			TCOD_console_put_char_ex(NULL, i, j, map[i][j].c, ucolor, TCOD_black);
		}
	}

}

void draw_msgs() {
	for (int i=0; i<msgs_s; i++) {
		TCOD_console_print_rect(NULL, 0, MAPY+i+1, (MAPX+MSGSIZE)/2, 20, "%s", msgs[i]);
	} 
	// below lies a memorial to the hours i spent trying to figure out c arrays
	/*for (int i=0; i<TCOD_list_size(msgs); i++) {
		char *msg = (char *)TCOD_list_get(msgs, i);
		TCOD_console_print_rect(NULL, 0, MAPY+1+i, MAPX+HUDX, 20, "%s", *msg);
	}*/
	/*int i = -1;
	while (strcmp(msgs[i++], "ENDMSGS") == false) {
		TCOD_console_print_rect(NULL, 0, MAPY+1+i, MAPX+HUDX, 20, "%s", msgs[i]);
	}
	for (int i=0; i<MAXMSGS; i++) {
		if (strcmp(msgs[i], "ENDMSGS") != 0) {
			TCOD_console_print_rect(NULL, 0, MAPY+1+i, MAPX+HUDX, 20, "%s", msgs[i]);
		}
	}*/
}

void draw_info() {
	for(int i=0; i<MAPX+HUDX; i++) {
		TCOD_console_put_char_ex(NULL, i, MAPY, '_', TCOD_grey, TCOD_black);
	}
	for(int j=0; j<MAPY+1; j++) {
		TCOD_console_put_char_ex(NULL, MAPX, j, '|', TCOD_grey, TCOD_black);
	}
	for(int k=MAPY+1; k<MAPY+MSGY; k++) {
		TCOD_console_put_char_ex(NULL, (MAPX+HUDX)/2, k, '|', TCOD_grey, TCOD_black);
	}
	struct Armor armor = calc_armor(player);
	TCOD_console_print_rect(NULL, MAPX+1, 0, HUDX, 20, "Depth: %d", depth);
	TCOD_console_print_rect(NULL, MAPX+1, 2, HUDX, 20, "Stats:");
	TCOD_console_print_rect(NULL, MAPX+1, 3, HUDX, 20, "HP: %d / %d", player.chp, player.mhp);
	TCOD_console_print_rect(NULL, MAPX+1, 4, HUDX, 20, "Power: %d / %d", player.cpow, player.mpow);
	TCOD_console_print_rect(NULL, MAPX+1, 6, HUDX, 20, "Equipment:");
	TCOD_console_print_rect(NULL, MAPX+1, 7, HUDX, 20, "Armor: (%d) [(M:%d R:%d CR:%d), (%d-%d) + %d]", check_stealth(player, player), player.ev-armor.ev_pen - player.spos*5, player.ev-armor.ev_pen + player.spos*5 + check_concealment(player), check_crit_res(player), armor.acdie, armor.acsides, check_cover(player));
	TCOD_console_print_rect(NULL, MAPX+1, 8, HUDX, 20, "Melee: %s [%d, (%dd%d)]", player.mwep.name, player.mwep.acc + player.melee - armor.melee_pen - player.mwep.acc_pen, player.mwep.dmgdie, player.mwep.dmgsides + (player.berserk>0)*player.mwep.dmgsides);
	int racc;
	if (player.rwep.bipod && player.spos > 0) {
		racc = player.ranged + player.rwep.acc + player.spos*5*2;
	} else {
		racc = player.ranged + player.rwep.acc - player.rwep.acc_pen - armor.acc_pen + player.spos*5;
	}
	TCOD_console_print_rect(NULL, MAPX+1, 9, HUDX, 20, "Ranged: %s [%d, (%dd%d)]", player.rwep.name, racc, player.rwep.dmgdie, player.rwep.dmgsides);
	if (player.wep.is_ranged) {
		TCOD_console_print_rect(NULL, MAPX+1, 10, HUDX, 20, "Ranged weapon set to default");
	} else {
		TCOD_console_print_rect(NULL, MAPX+1, 10, HUDX, 20, "Melee weapon set to default");
	}
	TCOD_console_print_rect(NULL, MAPX+1, 12, HUDX, 20, "Inventory:");
	TCOD_console_print_rect(NULL, MAPX+1, 13, HUDX, 20, "Medigel: %d", player.inv.medi);
	TCOD_console_print_rect(NULL, MAPX+1, 14, HUDX, 20, "Stimpacks: %d", player.inv.stim);
	TCOD_console_print_rect(NULL, MAPX+1, 15, HUDX, 20, "Power Cell: %d", player.inv.pcell);
	TCOD_console_print_rect(NULL, MAPX+1, 16, HUDX, 20, "Berserker Cell: %d", player.inv.pcell);
	TCOD_console_print_rect(NULL, MAPX+1, 17, HUDX, 20, "Smoke Grenades: %d", player.inv.smoke);
	TCOD_console_print_rect(NULL, MAPX+1, 18, HUDX, 20, "Glue Grenades: %d", player.inv.glue);
	TCOD_console_print_rect(NULL, MAPX+1, 19, HUDX, 20, "Paint Grenades: %d", player.inv.paint);
	TCOD_console_print_rect(NULL, MAPX+1, 20, HUDX, 20, "Barrier Grenades: %d", player.inv.barrier);
	TCOD_console_print_rect(NULL, MAPX+1, 21, HUDX, 20, "Frag Grenades: %d", player.inv.frag);
	TCOD_console_print_rect(NULL, MAPX+1, 22, HUDX, 20, "Rockets: %d", player.inv.rockets);
	TCOD_console_print_rect(NULL, MAPX+1, 23, HUDX, 20, "Drill Rockets: %d", player.inv.drill);
	TCOD_console_print_rect(NULL, MAPX+1, 24, HUDX, 20, "Flares: %d", player.inv.flare);
	TCOD_console_print_rect(NULL, MAPX+1, 25, HUDX, 20, "Noisemakers: %d", player.inv.noisemaker);
}
