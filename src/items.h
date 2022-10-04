#ifndef ITEMS_H
#define ITEMS_H

//struct Unit *unit;

extern bool select_item(struct Unit *unit);
extern bool use_medi(struct Unit *unit);
extern bool use_stim(struct Unit *unit);
extern bool use_pcell(struct Unit *unit);
extern bool use_smoke(struct Unit *unit);
extern bool use_glue(struct Unit *unit);
extern bool use_paint(struct Unit *unit);
extern bool use_barrier(struct Unit *unit);
extern bool use_grenade(struct Unit *unit);
extern bool use_drill(struct Unit *unit); 
extern bool use_rocket(struct Unit *unit);
extern bool use_flare(struct Unit *unit);

#endif
