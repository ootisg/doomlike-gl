#ifndef GAME_H
#define GAME_H

#include "render.h"

struct game_object {
	
	//The ID of this object's model
	int render_obj_id;
	
	//Animation info
	int is_animated;
	int num_frames;
	float frame_time;
	float start_time;
	
	//Position information
	float x;
	float y;
	
};

typedef struct game_object game_object;

//Set render_obj_id to -1 for a new render_obj
game_object* init_game_object (void* loc, scene* s, int num_vertices, int render_obj_id);

void billboard (game_object* obj, scene* s, float width, float height);

#endif