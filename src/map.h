#ifndef MAP_H
#define MAP_H

struct Unit;

struct Tile {
	// these should be a vector
	int c;
	int r;
	int g;
	
	int b; // what even is this?
	
	// these should be an int
	bool can_walk;
	bool is_transparent;
	bool filled;
	bool is_explored;
	bool wall;
	bool lit;
	bool tlit;
	bool door;
	bool down_stair;
	bool terminal;
	bool med;
	bool power;
	bool printer;
	bool forge;
	bool locker;
	int effect;
	int teffect;
	
	// should be a vector
	int cover;
	int concealment;
	
	int id;
	char name[MAXNSIZE];
	char desc[DESCSIZE];
};

struct Leaf {
	int rtype;
	int rtypes[5];
	struct Leaf *left;
	struct Leaf *right;
	int rdepth;
	int xsize;
	int ysize;
	int x1;
	int y1;
};

struct MapData {
	int max_x;
	int max_y;
	int tnum;
	struct Tile *tile_types;
};

extern struct MapData map_data;

extern struct Tile sfloor;
extern struct Tile printer;
extern struct Tile med;
extern struct Tile power;
extern struct Tile plant;
extern struct Tile locker;
extern struct Tile table;
extern struct Tile crop1;
extern struct Tile crop2;
extern struct Tile crop3;
extern struct Tile wall;
extern struct Tile sfloor;
extern struct Tile door;
extern struct Tile forge;

extern void tunnel();
extern void fill_wall();
extern void fill_edges();
extern bool fill_room(struct Leaf *leaf, int rsize_x, int rsize_y);
extern void gen_leaf(struct Leaf *leaf);
extern void bsp(int x1, int x2, int y1, int y2, int n);
extern void fill_rect(int x, int y);
extern void dig_path(int x1, int x2, int y1, int y2);
extern void pick_valid_xy(int *x, int *y);
extern void place_dstair();
extern void gen_map();
extern void gen_tmap();
extern void tile_effects();
extern void draw_map();
extern void draw_map_om();
extern void draw_msgs();
extern void draw_info();

#endif
