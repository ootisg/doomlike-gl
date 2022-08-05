#ifndef ROOM_H
#define ROOM_H

#include "render.h"

void init_floor (scene* render_scene);
void import_walls (scene* render_scene, char* path);

v4* get_wall_data ();
int get_num_walls ();
int lines_colliding (v2* result, v4* line_1, v4* line_2);

#endif