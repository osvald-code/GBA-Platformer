#include <string.h>
#include <tonc.h>
#include <stdlib.h>
#include "yuialt.h"
#include "yui.h"
#include "bg_test.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;


typedef struct sprite_struct{
	int id;
	int x; 
	int y;
	int facing;
	int buffer_id;
	u32 tile_id;
	u32 pal_id;
} Sprite;

typedef struct object_struct{
	int id;
	int bbox[4];
	int sprite_id;
	int y_speed;
	int x_speed;
}Object;

Sprite *sprites [2];
Object *objects [2];

typedef struct player_struct{
	int object_id;
	bool is_jumping;
}Player;


Object *init_object(int object_id, int sprite_id, int bb_x1, int bb_x2, int bb_y1, int bb_y2){
	Object *object = (Object*) malloc(sizeof(Object));
	objects[object_id] = object;
	object->id = object_id;
	object->sprite_id = sprite_id;
	object->bbox[0] = bb_x1;
	object->bbox[1] = bb_x2;
	object->bbox[2] = bb_y1;
	object->bbox[3] = bb_y2;
	object->x_speed = 0;
	object->y_speed = 0;
	return object;
}


Sprite *init_sprite(int sprite_id, int x, int y, u32 tile_id, u32 pal_id, int buffer_id, u16 arrR0,u16 arrR1){
	Sprite *sprite = (Sprite*) malloc(sizeof(Sprite));
	sprites[sprite_id] = sprite;
	sprite->id = sprite_id;
	sprite->facing = -1;
	sprite->x = x;
	sprite->y = y;
	sprite->tile_id = tile_id;
	sprite->pal_id = pal_id;	
	sprite->buffer_id = buffer_id;
	obj_set_attr(&obj_buffer[buffer_id], arrR0, arrR1, ATTR2_PALBANK(pal_id) | tile_id);
	return sprite;
}

void move_x(Object *object){
	int horz_in = 1*key_tri_horz();
	Sprite *sprite = sprites[object->sprite_id];
	//flip sprite
	if ((horz_in > 0 && sprite->facing!=1) || (horz_in < 0 && sprite->facing!=-1)) {
		((OBJ_ATTR *)&obj_buffer[sprite->buffer_id])->attr1 ^= ATTR1_HFLIP; 
		sprite->facing=horz_in;
	}
	sprite->x += horz_in;
	object->bbox[0] += horz_in;
	object->bbox[1] += horz_in;
}

void move_y(Object *object){
	//int vert_in = 1*key_tri_vert();
	Sprite *sprite = sprites[object->sprite_id];
	sprite->y -= object->y_speed; 
	object->bbox[2] -= object->y_speed;
	object->bbox[3] -= object->y_speed;
}

void move_set(Sprite *sprite)
{
	obj_set_pos(&obj_buffer[sprite->buffer_id], sprite->x, sprite->y);
}

void player_step(Player *player){
	Object *object = objects[player->object_id];
	Sprite *sprite = sprites[object->sprite_id];
	if (key_hit(KEY_B) && !player->is_jumping){
		player->is_jumping = true;
		object->y_speed+=1;
	}
	if (object->y_speed == 8){
		player->is_jumping = false;
	}
	else if (player->is_jumping){
		object->y_speed+=object->y_speed;
	}

	move_x(object);
	move_y(object);
	move_set(sprite);
}

/*
void object_step(Object *object){
			
		//key_poll();
		int horz_in = 1*key_tri_horz();
		object->sprite->x += horz_in;
		object->bbox[0] += horz_in;
		object->bbox[1] += horz_in;
		int vert_in = 1*key_tri_vert();
		object->sprite->y += vert_in;
		object->bbox[2] += vert_in;
		object->bbox[3] += vert_in;
		// move up/down
		/* if(key_hit(KEY_B)){
			object->sprite->tid+=4;
			object->sprite->obj_attr->attr2 = ATTR2_BUILD(object->sprite->tid, object->sprite->pb, 0);
		} 
		obj_set_pos(&obj_buffer[object->sprite->buffer_id], object->sprite->x , object->sprite->y);
		
}
*/
// testing a few sprite things
// D-pad: move
// SELECT: switch palette
// START: toggle mapping mode
// A: horizontal flip
// B: vertical flip
// L & R shift starting tile
void obj_test()
{
	int x= 96, y= 0;
	u32 tid= 0, pb= 0;		// tile id, pal-bank
//https://gbadev.net/tonc/regobj.html#sec-oam-entry
	OBJ_ATTR *metr= &obj_buffer[0];
	obj_set_attr(metr,
		ATTR0_SQUARE,				// Square, regular sprite
		ATTR1_SIZE_16x16,					// 64x64p,
		ATTR2_PALBANK(pb) | tid);		// palbank 0, tile 0

	// position sprite (redundant here; the _real_ position
	// is set further down
	obj_set_pos(metr, x, y);
	u32 sprite_index = 16;
	while(1)
	{
		vid_vsync();
		key_poll();

		/*if(key_is_down(KEY_DOWN)){
			y += 1;
			if (y%10==0){
				sprite_index = sprite_index==0?16:0;
			}
			metr->attr2 = ATTR2_BUILD(sprite_index, pb, 0);
		}else
		{
			metr->attr2 = ATTR2_BUILD(0, pb, 0);
		}
		*/
		// move left/right
		x += 1*key_tri_horz();

		// move up/down
		y += 1*key_tri_vert();

		// increment/decrement starting tile with R/L
		tid += bit_tribool(key_hit(-1), KI_R, KI_L);

		// flip
		if(key_hit(KEY_A)){
			metr->attr2 = ATTR2_BUILD(16, pb, 0);
			y += 5;
		}	// horizontally
			//metr->attr1 ^= ATTR1_HFLIP;
		//if(key_is_down(KEY_B))	// vertically
		//	metr->attr2 = ATTR2_BUILD(64, pb, 0);
		if(key_hit(KEY_B)){
			metr->attr2 = ATTR2_BUILD(32, pb, 0);
			y += 5;
		}
		
		// make it glow (via palette swapping)
		pb= key_is_down(KEY_SELECT) ? 1 : 0;

		// toggle mapping mode
		if(key_hit(KEY_START))
			REG_DISPCNT ^= DCNT_OBJ_1D;

		// Hey look, it's one of them build macros!
		//metr->attr2= ATTR2_BUILD(tid, pb, 0);
		obj_set_pos(metr, x, y);
		REG_BG0HOFS= -80;
		REG_BG0VOFS= -64;
		oam_copy(oam_mem, obj_buffer, 1);	// only need to update one
	}
}

bool is_collision(Object *obj_a, Object *obj_b){

	return (obj_a->bbox[1] > obj_b->bbox[0]) && (obj_a->bbox[2] < obj_b->bbox[3]);
}

Player* load_player(){
	Player* player = (Player*) malloc(sizeof(Player));
	player->object_id = 0;
	Object* object = init_object(0,0,0,8,0,16);
	sprites[0] = init_sprite(0, 0,0, 0,0, 0,ATTR0_SQUARE,ATTR1_SIZE_16x16);  
	player->is_jumping = false;
	return player;
}

void load_level_objects(){

}


void load_vram(){
	//obj1
	memcpy32(&tile_mem[4][0], yuiTiles, yuiTilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[0], yuiPal, yuiPalLen / sizeof(u16));

	//obj2
	memcpy32(&tile_mem[4][20], yuialtTiles, yuialtTilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[16], yuialtPal, yuialtPalLen / sizeof(u16));

	// Load palette
	memcpy16(pal_bg_mem, bg_testPal, bg_testPalLen / sizeof(u16));
	// Load tiles into CBB 0
	memcpy32(&tile_mem[0][0], bg_testTiles, bg_testTilesLen / sizeof(u32));
	// Load map into SBB 30
	memcpy32(&se_mem[30][0], bg_testMap, bg_testMapLen / sizeof(u32));	
}

// Define a red 1x1 tile for the hitbox
const u16 hitbox_tile[8] = {
    0x1F, 0x1F, 0x1F, 0x1F,  // Red tile
    0x1F, 0x1F, 0x1F, 0x1F
};

void draw_hitbox(int x, int y) {
    for (int i = 0; i < 3; i++) {  // 3x3 hitbox
        for (int j = 0; j < 3; j++) {
			int index = (y + j) * 32 + (x + i);
            ((u16*) 0x06000000)[index] = 0;  // 0 is the tile index for the hitbox tile
        }
    }
}

int main()
{
	load_vram();
	REG_DISPCNT= DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0 | DCNT_BG0;
	memcpy((void*)0x06008000, (const void*)hitbox_tile, sizeof(hitbox_tile));
	draw_hitbox(5, 5);
	//tte_init_chr4c_b4_default(0, BG_CBB(2)|BG_SBB(31));
   // tte_write("#{P:72,64}");        // Goto (72, 64).
    //tte_write("Hello World!");      // Print "Hello world!"*/
	//tte_init_con();
		// set up BG0 for a 4bpp 64x32t map, using
	//   using charblock 0 and screenblock 31
	
	
	//REG_BG0CNT= BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_REG_64x64;

	//obj_test();
	//REG_BG0HOFS= -100;
	//REG_BG0VOFS= -64;
	
	
	oam_init(obj_buffer, 128);
	Player *plr = load_player();
	Object *plr2 = init_object(96+16,96+32,0,0,
		init_sprite(96+16,0, 20,1, 1, ATTR0_SQUARE,ATTR1_SIZE_16x16)
	);
	//((OBJ_ATTR *)&obj_buffer[plr2->sprite->buffer_id])->attr1 ^= ATTR1_HFLIP; 
	
	//Object* floor = init_object(1,-1,100,64,100+16,64+8);
	//obj_set_pos(&obj_buffer[plr2->sprite->buffer_id], plr2->sprite->x , plr2->sprite->y);
	int i =0 ; 
	char temp[128];
	while(1)
	{
		//VBlankIntrWait();
		vid_vsync();
		key_poll(); 
		player_step(plr);
		/*if(is_collision(plr,plr2)){
			plr->sprite->tile_id+=4;
			((OBJ_ATTR *)&obj_buffer[plr->sprite->buffer_id])->attr2 = ATTR2_BUILD(plr->sprite->tile_id, plr->sprite->pal_id, 0);
			plr->sprite->x-=10;
			plr->bbox[0]-=10;
			plr->bbox[1]-=10;
			obj_set_pos(&obj_buffer[plr->sprite->buffer_id], plr->sprite->x , plr->sprite->y);
		}*/ 
		/*if(plr->sprite->y-8 >= 100){
			plr->speed = 0;
		}
		*/
	
		/* if (is_collision(objects[plr->object_id],floor)){
			objects[plr->object_id]->y_speed = 0;
		}else */ 
		
		/*if (!(bool)plr->is_jumping && objects[plr->object_id]->y_speed>-8){
			objects[plr->object_id]->y_speed-= 1;
		}

		if (objects[plr->object_id]->bbox[3]>=160){
			int bboxcross = objects[plr->object_id]->bbox[3];
			objects[plr->object_id]->y_speed=0;
			objects[plr->object_id]->y_speed=0;
			sprites[(objects[plr->object_id])->sprite_id]->y -= 160-bboxcross; 
			objects[plr->object_id]->bbox[2] -= 160-bboxcross;
			objects[plr->object_id]->bbox[3] -= 160-bboxcross;
		}
		

		i++;
		//object_step(plr2);
		*/
		oam_copy(oam_mem, obj_buffer,2);
	};

	return 0;
}
