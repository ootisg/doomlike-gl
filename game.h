#ifndef GAME_H
#define GAME_H

#include "render.h"

struct game_object {
	
	//The ID of this object's model
	int render_obj_id;
	
	//Animation info
	int is_animated;
	int num_frames;
	int repeat_anim;
	float frame_time;
	float start_time;
	
	//Position information
	float x;
	float y;
	
};

typedef struct game_object game_object;

//Game logic, call every frame
void game_logic_step (scene* s);

//Set render_obj_id to -1 for a new render_obj
game_object* init_game_object (void* loc, scene* s, material* mat, int num_vertices, int render_obj_id);

void billboard (game_object* obj, scene* s, float width, float height);

//Object-specific inits
game_object* init_enemy (scene* s, float x, float y);
game_object* init_barrel (scene* s, float x, float y);
game_object* init_item (scene* s, material* mat, void* callback, float x, float y);
game_object* init_decoration (scene* s, material* mat, float x, float y);
game_object* init_door (scene* s, int door_type, float x1, float y1, float x2, float y2);

//Item callbacks
void bomb_callback ();
void speed_callback ();
void health_callback ();
void boots_callback ();
void dmg_callback ();

#endif