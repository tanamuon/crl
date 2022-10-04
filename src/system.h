#ifndef SYSTEM_H
#define SYSTEM_H

// this file was a mistake

#define MAPX 80
#define MAPY 50
#define MAXROOMSIZE 7
#define MINROOMSIZE 3
#define ROOMNUM 20
#define MSGY 20
#define HUDX 40
#define MSGSIZE 128
#define MAXMSGS 20
#define MAXUNITS 30
#define MAXNSIZE 32
#define DESCSIZE 1024
#define STORYMSGS 256
#define MAXMODS 12
#define NUMITEMS 13

#define SIZE(X) sizeof(X)/(sizeof(X[0]))

#define A1_LEAP				1<<1  
#define A1_PUNT				1<<2
#define A1_EXECUTE			1<<3
#define A1_POWER_MAP			1<<4
#define A1_OVERDRIVE_GEARS		1<<5
#define A1_DASH				1<<6
#define A1_HAMMER_SHOT			1<<7
#define A1_ZONE_OF_CONTROL		1<<8
#define A1_FLANKING			1<<9
#define A1_PUNISH			1<<10
#define A1_CRIPPLING_SHOT		1<<11
#define A1_COVERING_FIRE		1<<12
#define A1_RIPOSTE			1<<13
#define A1_MED1				1<<14
#define A1_MED2				1<<15
#define A1_HEADSHOT_PROTOCOL		1<<16
#define A1_KNEELING_POSITION		1<<17
#define A1_PRONE_POSITION		1<<18
#define A1_ACTIVE_CAMO			1<<19
#define A1_MICROPHONE_SYSTEM		1<<20
#define A1_NIGHT_VISION			1<<21
#define A1_RECON_SUBROUTINE		1<<22
#define A1_DOOR_HACKING			1<<23
#define A1_CAMERA_SYSTEM		1<<24
#define A1_PREDICTION_SUBROUTINE	1<<25
#define A1_WELDING_SYSTEM		1<<26
#define A1_ASSASSINATE			1<<27
#define A1_PLASMA_CUTTER		1<<28
#define A1_STEALTH_SUBROUTINE		1<<29
#define A1_IMPETUOUS_OFFENSE		1<<30
#define A1_FACILE_DODGING		1<<31

#define A2_SILENCER			1<<1
#define A2_JAMMING			1<<2
#define A2_KNOCKBACK			1<<3

#define	S1_SILENCED			1<<1
#define	S1_NORETAL			1<<2
#define	S1_RIPOSTING			1<<3
#define	S1_CAMO				1<<4
#define	S1_FAM				1<<5
#define	S1_HUNTING			1<<6
#define	S1_FLANKING			1<<7
#define	S1_STUCK			1<<8
#define	S1_TAGGED			1<<9
#define	S1_AWARE			1<<10
#define	S1_DEST				1<<11
#define	S1_BLIND			1<<12
#define	S1_IMMOBILIZER			1<<13
#define	S1_NOEVADE			1<<14
#define	S1_RUNNING			1<<15
#define	S1_CRIPPLED			1<<16
#define	S1_COMMS			1<<17
#define	S1_NOESCAPE			1<<18
#define	S1_LIT				1<<19
#define	S1_TURNSPENT			1<<20

enum dmgtype {KINETIC_EDGED, KINETIC_BLUNT, THERMAL, ELECTRICAL, CHEMICAL, BIOLOGICAL, NUCLEAR};
enum dmgeffect {REGENERATING, ANTI_SHOCK, BLEED, ON_FIRE, ACID, SOPOR, INFECTED, IRRADIATED};
enum Etype {GRENADE, ROCKET, SMOKE, BARRIER, GLUE, PAINT, FLARE};

#endif
