#include "game.h"

#include "render.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "buffers_gl.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_ENEMIES 128
#define MAX_ITEMS 128
#define MAX_BARRELS 128
#define MAX_DECORATIONS 128
#define MAX_DOORS 128

material* enemy_material = NULL;
material* barrel_material = NULL;
material* door_material = NULL;

game_object* enemy_list = NULL;
int num_enemies;

game_object* item_list = NULL;
void** item_collision_funcs = NULL;
int num_items = 0;

game_object* barrel_list = NULL;
int num_barrels = 0;

game_object* decoration_list = NULL;
int num_decorations = 0;

game_object* door_list = NULL;
int num_doors = 0;

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
	int i;
	for (i = 0; i < num_enemies; i++) {
		billboard (&(enemy_list[i]), s, 2, 2);
	}
	for (i = 0; i < num_barrels; i++) {
		billboard (&(barrel_list[i]), s, 2, 2);
	}
	for (i = 0; i < num_decorations; i++) {
		billboard (&(decoration_list[i]), s, 2, 2);
	}
	for (i = 0; i < num_items; i++) {
		billboard (&(item_list[i]), s, 2, 2);
	}
}

//Object-specific functions
game_object* init_enemy (scene* s, float x, float y) {
	
	//Init enemy material/list if not initialized
	if (enemy_list == NULL) {
		enemy_list = malloc (sizeof (game_object) * MAX_ENEMIES);
	}
	if (enemy_material == NULL) {
		enemy_material = malloc (sizeof (material));
		init_material (enemy_material, "resources/demon_idle.png", "resources/theme_5_specular.png");
	}
	
	//Allocate enemy
	game_object* tobj = &(enemy_list[num_enemies++]);
	//Initialize enemy
	init_game_object (tobj, s, enemy_material, 4, -1);
	tobj->x = x;
	tobj->y = y;
	tobj->is_animated = 1;
	tobj->num_frames = 6;
	tobj->frame_time = .1;
	return tobj;
	
}

game_object* init_barrel (scene* s, float x, float y) {
	
	//Init enemy material/list if not initialized
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
	
	//Init enemy material/list if not initialized
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
	
	//Init enemy material/list if not initialized
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
	matrix_trans4 (&(s->models[rid]), 33, 1, 33);
	
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
		int curr_frame = frames_elapsed % obj->num_frames;
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