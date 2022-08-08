#ifndef ROOM_H
#define ROOM_H

#include "render.h"

void init_floor (scene* render_scene);
void import_walls (scene* render_scene, char* path);
void import_objs (scene* render_scene, char* path);
v4* add_wall (v4* data); //Note: this ONLY adds wall collision

v4* get_wall_data ();
int get_num_walls ();
int lines_colliding (v2* result, v4* line_1, v4* line_2);

#endif