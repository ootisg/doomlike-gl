#include "game.h"

#include "render.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "buffers_gl.h"
#include "room.h"
#include "render_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GLFW/glfw3.h>

#define MAX_ENEMIES 128
#define MAX_ITEMS 128
#define MAX_BARRELS 128
#define MAX_DECORATIONS 128
#define MAX_DOORS 128

material* enemy_material = NULL;
material* enemy_dead_material = NULL;
material* barrel_material = NULL;
material* door_material = NULL;

game_object* enemy_list = NULL;
float* enemies_health;
int num_enemies;

game_object* item_list = NULL;
void** item_collision_funcs = NULL;
int num_items = 0;

game_object* barrel_list = NULL;
int num_barrels = 0;

game_object* decoration_list = NULL;
int num_decorations = 0;

game_object* door_list = NULL;
float* door_timers = NULL;
v4** door_collisions = NULL;
int num_doors = 0;

int bomb_count = 0;
int speed_count = 0;
float health = 100.0f;
int has_dmg = 0;

void bomb_callback () {
	bomb_count++;
}

void speed_callback () {
	speed_count++;
}

void health_callback () {
	health += 50.0f;
	if (health >= 100.0f) {
		health = 100.0f;
	}
}

void boots_callback () {
	set_player_speed (4);
}

void dmg_callback () {
	has_dmg = 1;
}

void printfloats (float* loc, int amt) {
	int i;
	for (i = 0; i < amt; i++) {
		printf ("INDEX %d: %f\n", i, loc[i]);
	}
}

float obj_verticess[] = {
    // positions          // normals           // texture coords
	 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f
};

//Game logic
void game_logic_step (scene* s) {
	
	//Handle speedup usage
	if (speeding () && speed_count > 0) {
		set_speedup_timer (7);
		speed_count--;
	}
	
	//Handle bomb count logic
	int using_bomb = 0;
	if (bombing () && bomb_count > 0) {
		using_bomb = 1;
		bomb_count--;
	}
	
	//ENEMY LOGIC. No optimizations here, since the # of enemies is always < 20
	int i;
	for (i = 0; i < num_enemies; i++) {
		
		//Get the current enemy
		game_object* curr = &(enemy_list[i]);
		
		//Only run enemy logic if enemy isn't dead
		if (enemies_health[i] > 0.0f) {
			
			//Get the current camera (camera position = player position)
			camera* cam = get_active_camera ();
			
			//Check for line-of-sight to the player
			v4 sight_line;
			initv4 (&sight_line, curr->x, curr->y, cam->pos.x, cam->pos.z);
			int has_los = 1;
			int j;
			for (j = 0; j < get_num_walls (); j++) {
				v4* curr_wall = &(get_wall_data ()[j]);
				v2 res;
				if (lines_colliding (&res, curr_wall, &sight_line)) { //Argument order matters due to incorrect handling of special case in lines_colliding
					has_los = 0;
					break;
				}
			}
			//Move toward player if further than .5 units and has LOS
			if (has_los) {
				
				//Kill enemies if player used bomb
				if (using_bomb) {
					enemies_health[i] = 0.0f;
					s->materials[curr->render_obj_id] = *enemy_dead_material;
					curr->start_time = glfwGetTime ();
					curr->repeat_anim = 0;
				}
				
				//Enemy AI
				float dist = sqrt ((curr->x - cam->pos.x) * (curr->x - cam->pos.x) + (curr->y - cam->pos.z) * (curr->y - cam->pos.z));
				if (dist >= 0.5) {
					//Move towards player
					float enemy_speed = 3.0;
					v2 dir_vec;
					initv2 (&dir_vec, curr->x - cam->pos.x, curr->y - cam->pos.z);
					vector_normalize2 (&dir_vec, &dir_vec);
					vector_scale2 (&dir_vec, &dir_vec, -enemy_speed * frame_delta_time ());
					curr->x += dir_vec.x;
					curr->y += dir_vec.y;
					//Move away from other enemies (prevents grouping)
					float force_coeff = .003;
					int k;
					for (k = 0; k < num_enemies; k++) {
						if (k != i) {
							game_object* other = &(enemy_list[k]);
							float enemy_dist = sqrt ((curr->x - other->x) * (curr->x - other->x) + (curr->y - other->y) * (curr->y - other->y));
							if (enemy_dist < 3) {
								v2 force_vec;
								initv2 (&force_vec, curr->x - other->x, curr->y - other->y);
								vector_normalize2 (&force_vec, &force_vec);
								vector_scale2 (&force_vec, &force_vec, (1 / (enemy_dist * enemy_dist)) * force_coeff);
								curr->x += force_vec.x;
								curr->y += force_vec.y;
							}
						}
					}
				} else {
					printf ("PLAYER HIT!\n");
				}
			}
			
			//Detect if shot at
			if (shooting ()) {
				
				//Hitbox detection (sort of)
				v2 colliding_pt;
				v2 shoot_vec, shoot_dir;
				initv2 (&shoot_dir, cam->dir.x, cam->dir.z);
				vector_scale2 (&shoot_vec, &shoot_dir, -100);
				v4 shoot_line;
				initv4 (&shoot_line, cam->pos.x, cam->pos.z, cam->pos.x + shoot_vec.x, cam->pos.z + shoot_vec.y);
				float hbox_size = .5;
				v4 hblines[2];
				initv4 (&(hblines[0]), curr->x - hbox_size, curr->y - hbox_size, curr->x + hbox_size, curr->y + hbox_size);
				initv4 (&(hblines[1]), curr->x + hbox_size, curr->y - hbox_size, curr->x - hbox_size, curr->y + hbox_size);
				int j;
				int hbox_collided = 0;
				for (j = 0; j < 2; j++) {
					if (lines_colliding (&colliding_pt, &shoot_line, &(hblines[j]))) {
						hbox_collided = 1;
					}
				}
				
				//Check for walls between player and enemy
				if (hbox_collided) {
					float max_dist = 100;
					for (j = 0; j < get_num_walls (); j++) {
						v4* curr_wall = &(get_wall_data ()[j]);
						if (lines_colliding (&colliding_pt, curr_wall, &shoot_line)) {
							v2 collision_vec;
							v2 cam_pos;
							initv2 (&cam_pos, cam->pos.x, cam->pos.z);
							vector_diff2 (&collision_vec, &cam_pos, &(colliding_pt));
							float dist = sqrt (collision_vec.x * collision_vec.x + collision_vec.y * collision_vec.y);
							if (dist < max_dist) {
								max_dist = dist;
							}
						}
					}
					float dist_to_enemy = sqrt ((curr->x - cam->pos.x) * (curr->x - cam->pos.x) + (curr->y - cam->pos.z) * (curr->y - cam->pos.z));
					if (max_dist > dist_to_enemy) {
						//Enemy was hit
						//Damage enemy
						printf ("HIT %d\n", i);
						enemies_health[i] -= has_dmg ? 34.0f : 20.0f;
						//Handle enemy death
						if (enemies_health[i] <= 0.0f) {
							s->materials[curr->render_obj_id] = *enemy_dead_material;
							curr->start_time = glfwGetTime ();
							curr->repeat_anim = 0;
						}
					}
				}
			}
		}
		
		//Billboard the enemy to face the player
		billboard (curr, s, 2, 2);
		
	}
	for (i = 0; i < num_barrels; i++) {
		billboard (&(barrel_list[i]), s, 2, 2);
	}
	for (i = 0; i < num_decorations; i++) {
		billboard (&(decoration_list[i]), s, 2, 2);
	}
	for (i = 0; i < num_items; i++) {
		
		//Get the current item
		game_object* curr = &(item_list[i]);
		
		//Get the current camera (camera position = player position)
		camera* cam = get_active_camera ();
		
		//Check for collision
		float dist = sqrt ((cam->pos.x - curr->x) * (cam->pos.x - curr->x) + (cam->pos.z - curr->y) * (cam->pos.z - curr->y));
		if (dist < 1) {
			//TODO callback
			void (*callback_func)(void) = item_collision_funcs[i];
			callback_func ();
			curr->x = -1;
			curr->y = -1;
		}
		
		//Billboard the items to face the player
		billboard (curr, s, 2, 2);
		
	}
	for (i = 0; i < num_doors; i++) {
		
		//Activate doors when nearby
		camera* cam = get_active_camera ();
		game_object* curr = &(door_list[i]);
		double dist = sqrt ((cam->pos.x - curr->x) * (cam->pos.x - curr->x) + (cam->pos.z - curr->y) * (cam->pos.z - curr->y));
		if (dist < 1 && door_timers[i] == -1) {
			door_timers[i] = glfwGetTime ();
		}
		
		//Animate doors going away
		if (door_timers[i] != -1) {
			float elapsed = glfwGetTime () - door_timers[i];
			if (elapsed > 2.01) {
				initv4 ((door_collisions[i]), 0, 0, -1, -1);
				//Door is done
			} else {
				float y_offs = elapsed;
				int rid = door_list[i].render_obj_id;
				matrix_trans4 (&(s->models[rid]), 0, -y_offs, 0);
			}
		}
		
	}
}

//Object-specific functions
game_object* init_enemy (scene* s, float x, float y) {
	
	//Init enemy material/list if not initialized
	if (enemy_list == NULL) {
		enemy_list = malloc (sizeof (game_object) * MAX_ENEMIES);
		enemies_health = malloc (sizeof (int) * MAX_ENEMIES);
	}
	if (enemy_material == NULL) {
		enemy_material = malloc (sizeof (material));
		enemy_dead_material = malloc (sizeof (material));
		init_material (enemy_material, "resources/demon_idle.png", "resources/theme_5_specular.png");
		init_material (enemy_dead_material, "resources/demon_death.png", "resources/theme_5_specular.png");
	}
	
	//Allocate enemy
	enemies_health[num_enemies] = 100.0;
	game_object* tobj = &(enemy_list[num_enemies++]);
	//Initialize enemy
	init_game_object (tobj, s, enemy_material, 4, -1);
	tobj->x = x;
	tobj->y = y;
	tobj->is_animated = 1;
	tobj->repeat_anim = 1;
	tobj->num_frames = 6;
	tobj->frame_time = .1;
	return tobj;
	
}

game_object* init_barrel (scene* s, float x, float y) {
	
	//Init barrel list if not initialized
	if (barrel_list == NULL) {
		barrel_list = malloc (sizeof (game_object) * MAX_BARRELS);
	}
	if (barrel_material == NULL) {
		barrel_material = malloc (sizeof (material));
		init_material (barrel_material, "resources/barrel.png", "resources/theme_5_specular.png");
	}
	
	//Allocate barrel
	game_object* tobj = &(barrel_list[num_barrels++]);
	//Initialize barrel
	init_game_object (tobj, s, barrel_material, 4, -1);
	tobj->x = x;
	tobj->y = y;
	return tobj;
	
}

game_object* init_item (scene* s, material* mat, void* callback, float x, float y) {
	
	//Init item list if not initialized
	if (item_list == NULL) {
		item_list = malloc (sizeof (game_object) * MAX_ITEMS);
	}
	if (item_collision_funcs == NULL) {
		item_collision_funcs = malloc (sizeof (void*) * MAX_ITEMS);
	}
	
	//Set the callback
	item_collision_funcs[num_items] = callback;
	//Allocate item
	game_object* tobj = &(item_list[num_items++]);
	//Initialize item
	init_game_object (tobj, s, mat, 4, -1);
	tobj->x = x;
	tobj->y = y;
	return tobj;
	
}

game_object* init_decoration (scene* s, material* mat, float x, float y) {
	
	//Init decoration list if not initialized
	if (decoration_list == NULL) {
		decoration_list = malloc (sizeof (game_object) * MAX_DECORATIONS);
	}
	
	//Allocate decoration
	game_object* tobj = &(decoration_list[num_decorations++]);
	//Initialize decoration
	init_game_object (tobj, s, mat, 4, -1);
	tobj->x = x;
	tobj->y = y;
	return tobj;
	
}

//TODO init_door
game_object* init_door (scene* s, int door_type, float x1, float y1, float x2, float y2) {
	
	//Init door material/list if not initialized
	if (door_list == NULL) {
		door_list = malloc (sizeof (game_object) * MAX_DOORS);
		door_timers = malloc (sizeof (float) * MAX_DOORS);
		door_collisions = malloc (sizeof (float) * MAX_DOORS);
	}
	if (door_material == NULL) {
		door_material = malloc (sizeof (material));
		init_material (door_material, "resources/door.png", "resources/theme_5_specular.png");
	}
	
	//Allocate door
	door_timers[num_doors] = -1;
	v4 collision_data;
	initv4 (&collision_data, x1, y1, x2, y2);
	door_collisions[num_doors] = add_wall (&collision_data);
	game_object* tobj = &(door_list[num_doors++]);
	//Initialize door
	init_game_object (tobj, s, door_material, 4, -1);
	tobj->x = (x1 + x2) / 2;
	tobj->y = (y1 + y2) / 2;
	
	//Fill vertex data
	float* buffer = malloc (sizeof (float) * 8 * 4);
	camera* cam = get_active_camera ();
	v3 offs, dir, perp, normal, obj_pos, up;
	initv3 (&up, 0, 1, 0);
	initv3 (&dir, x2 - x1, 0, y2 - y1);
	vector_normalize3 (&dir, &dir);
	vector_cross3 (&perp, &up, &dir);
	initv3 (&normal, perp.x, perp.y, perp.z);
	int i;
	for (i = 0; i < 2; i++) {
		//Vert 1 and 3
		buffer[i * 16 + 0] = i == 0 ? x1 : x2;
		buffer[i * 16 + 1] = 0;
		buffer[i * 16 + 2] = i == 0 ? y1 : y2;
		buffer[i * 16 + 3] = normal.x;
		buffer[i * 16 + 4] = normal.y;
		buffer[i * 16 + 5] = normal.z;
		buffer[i * 16 + 6] = i;
		buffer[i * 16 + 7] = 0;
		//Vert 2 and 4
		buffer[i * 16 + 8] = i == 0 ? x1 : x2;
		buffer[i * 16 + 9] = 2;
		buffer[i * 16 + 10] = i == 0 ? y1 : y2;
		buffer[i * 16 + 11] = normal.x;
		buffer[i * 16 + 12] = normal.y;
		buffer[i * 16 + 13] = normal.z;
		buffer[i * 16 + 14] = i;
		buffer[i * 16 + 15] = 1;
	}
	int rid = tobj->render_obj_id;
	GLuint vao_id = s->vaos[rid];
	glBindBuffer (GL_ARRAY_BUFFER, s->vbos[rid]);
	glBufferData (GL_ARRAY_BUFFER, sizeof (float) * 8 * 4, buffer, GL_STATIC_DRAW);
	free (buffer);
	return tobj;
	
}

//Game object functions
game_object* init_game_object (void* loc, scene* s, material* mat, int num_vertices, int render_obj_id) {
	
	//Cast
	game_object* obj = (game_object*)loc;
	
	//Alloc a render object
	int rid;
	if (render_obj_id == -1) {
		rid = obj->render_obj_id = s->num_objs++;
	} else {
		rid = obj->render_obj_id = render_obj_id;
	}
	//Bind the program
	s->programs[rid] = make_program_from_files ("obj_vertex_shader.glsl", NULL, "obj_frag_shader.glsl");
	//Bind a vertex buffer (initially all 0) using a VAO
	glGenVertexArrays (1, &(s->vaos[rid]));
	glBindVertexArray (s->vaos[rid]);
	float* vertex_buffer = malloc (sizeof (float) * 8 * num_vertices);
	int i;
	for (i = 0; i < 8 * num_vertices; i++) {
		vertex_buffer[i] = 0;
	}
	VBO* vertex_vbo = VBO_init (malloc (sizeof (VBO)), obj_verticess, sizeof (float) * 8 * num_vertices, GL_ARRAY_BUFFER);
	s->vbos[rid] = vertex_vbo->buffer_id;
	free (vertex_buffer);
	s->mesh_sizes[rid] = num_vertices;
	s->obj_names[rid] = "game_object";
	s->materials[rid] = *mat;
	//Attrib ptrs
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof (float)));
	glEnableVertexAttribArray (1);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof (float)));
	glEnableVertexAttribArray (2);
	//Initialize the model matrix to the identity matrix
	matrix_trans4 (&(s->models[rid]), 0, 0, 0);
	
	//Default parameters
	obj->is_animated = 0;
	obj->num_frames = 1;
	obj->frame_time = 1;
	obj->start_time = glfwGetTime ();
	obj->x = 0;
	obj->y = 0;
	
	//Return the object
	return obj;
	
}

void billboard (game_object* obj, scene* s, float width, float height) {
	
	//Calculate which frame of animation to use/texcoords
	float tex_x = 0;
	float tex_y = 0;
	float tex_width = 1;
	float tex_height = 1;
	if (obj->is_animated) {
		tex_width = 1.0 / obj->num_frames;
		float dt = glfwGetTime () - obj->start_time;
		int frames_elapsed = dt / obj->frame_time;
		int curr_frame = obj->repeat_anim ? frames_elapsed % obj->num_frames : (frames_elapsed > obj->num_frames - 1 ? obj->num_frames - 1 : frames_elapsed);
		tex_x = tex_width * curr_frame;
	}
	
	//Calculate vertex positions and normal
	float* buffer = malloc (sizeof (float) * 8 * 4);
	camera* cam = get_active_camera ();
	v3 offs, perp, normal, obj_pos;
	vector_cross3 (&perp, &(cam->up), &(cam->dir));
	initv3 (&normal, perp.x, perp.y, perp.z);
	initv3 (&obj_pos, obj->x, 0, obj->y);
	vector_scale3 (&perp, &perp, width / 2);
	int i;
	for (i = 0; i < 2; i++) {
		//Vert 1 and 3
		buffer[i * 16 + 0] = i == 0 ? perp.x : -perp.x;
		buffer[i * 16 + 1] = 0;
		buffer[i * 16 + 2] = i == 0 ? perp.z : -perp.z;
		buffer[i * 16 + 3] = normal.x;
		buffer[i * 16 + 4] = normal.y;
		buffer[i * 16 + 5] = normal.z;
		buffer[i * 16 + 6] = tex_x + i * tex_width;
		buffer[i * 16 + 7] = 0;
		//Vert 2 and 4
		buffer[i * 16 + 8] = i == 0 ? perp.x : -perp.x;
		buffer[i * 16 + 9] = height;
		buffer[i * 16 + 10] = i == 0 ? perp.z : -perp.z;
		buffer[i * 16 + 11] = normal.x;
		buffer[i * 16 + 12] = normal.y;
		buffer[i * 16 + 13] = normal.z;
		buffer[i * 16 + 14] = tex_x + i * tex_width;
		buffer[i * 16 + 15] = tex_height;
	}
	
	//Push the vertex data
	int rid = obj->render_obj_id;
	GLuint vao_id = s->vaos[rid];
	glBindBuffer (GL_ARRAY_BUFFER, s->vbos[rid]);
	glBufferData (GL_ARRAY_BUFFER, sizeof (float) * 8 * 4, buffer, GL_DYNAMIC_DRAW);
	glBindVertexArray (0);
	free (buffer);
	
	//Set the model matrix accordingly
	matrix_trans4 (&(s->models[rid]), obj->x, 0, obj->y);
	
}