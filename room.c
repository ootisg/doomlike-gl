#include "room.h"
#include "buffers_gl.h"
#include "vector.h"
#include "char_buffer.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

v4* wall_data;
int num_walls;

v4* get_wall_data () {
	return wall_data;
}

int get_num_walls () {
	return num_walls;
}

float min (float a, float b) {
	return a < b ? a : b;
}

float max (float a, float b) {
	return a > b ? a : b;
}

int lines_colliding (v2* result, v4* line_1, v4* line_2) {
	
	float tolerance = .0002;
	int vertical_case = 0;
	
	//Both lines vertical
	if (line_1->x == line_1->z && line_2->x == line_2->z) {
		//Parallel vertical lines are considered to be non-colliding
		return 0;
	}
	
	//In the case of vertical lines, line_1 is always vertical
	if (abs (line_2->x - line_2->z) < tolerance) {
		v4* temp = line_1;
		line_1 = line_2;
		line_2 = temp;
	}
	if (abs (line_1->x - line_1->z) < tolerance) {
		vertical_case = 1;
	}
	
	//Compute parameters for line 2
	float m2 = (line_2->y - line_2->w) / (line_2->x - line_2->z); //m = dy/dx
	float b2 = (line_2->y - m2 * line_2->x); //b = y - mx
	
	//Compute the intersection point
	float isect_x, isect_y;
	if (vertical_case) {
		//Vertical line case
		isect_x = line_1->x;
		isect_y = m2 * isect_x + b2; //y = mx + b
	} else {
		//Neither line is degenerate, solve the linear system of equations
		float m1 = (line_1->y - line_1->w) / (line_1->x - line_1->z); //m = dy/dx
		float b1 = (line_1->y - m1 * line_1->x); //b = y - mx
		isect_x = (b1 - b2) / (m2 - m1); //Find solution by substitution
		isect_y = m1 * isect_x + b1;
	}
	
	//Check if the intersection point is inside the line segments
	float min_x, min_y, max_x, max_y;
	//Line 1
	min_x = min (line_1->x, line_1->z);
	max_x = max (line_1->x, line_1->z);
	min_y = min (line_1->y, line_1->w);
	max_y = max (line_1->y, line_1->w);
	if (!(isect_x >= min_x && isect_x <= max_x && isect_y >= min_y && isect_y <= max_y)) {
		return 0;
	}
	//Line 2
	min_x = min (line_2->x, line_2->z);
	max_x = max (line_2->x, line_2->z);
	min_y = min (line_2->y, line_2->w);
	max_y = max (line_2->y, line_2->w);
	if (!(isect_x >= min_x && isect_x <= max_x && isect_y >= min_y && isect_y <= max_y)) {
		return 0;
	}
	//Intersection point is inside segments
	result->x = isect_x;
	result->y = isect_y;
	return 1;
	
}

float floor_vertices[] = {
    // positions          // normals           // texture coords
    -100.0f, 0.0f, -100.0f,0.0f, 1.0f, 0.0f, 0.0f,   0.0f,
	100.0f,  0.0f, -100.0f,0.0f, 1.0f, 0.0f, 100.0f, 0.0f,
	-100.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f,   100.0f,
	100.0f,  0.0f, 100.0f, 0.0f, 1.0f, 0.0f, 100.0f, 100.0f
};

float ceiling_vertices[] = {
    // positions          // normals           // texture coords
    -100.0f, 2.0f, -100.0f,0.0f, -1.0f, 0.0f, 0.0f,   0.0f,
	100.0f,  2.0f, -100.0f,0.0f, -1.0f, 0.0f, 100.0f, 0.0f,
	-100.0f, 2.0f, 100.0f, 0.0f, -1.0f, 0.0f, 0.0f,   100.0f,
	100.0f,  2.0f, 100.0f, 0.0f, -1.0f, 0.0f, 100.0f, 100.0f
};

void init_floor (scene* render_scene) {
	int i;
	for (i = 0; i < 2; i++) {
		int idx = render_scene->num_objs;
		render_scene->programs[idx] = make_program_from_files ("obj_vertex_shader.glsl", NULL, "obj_frag_shader.glsl");
		glGenVertexArrays (1, &(render_scene->vaos[idx]));
		glBindVertexArray (render_scene->vaos[idx]);
		VBO* vertex_vbo = VBO_init (malloc (sizeof (VBO)), (i == 0 ? floor_vertices : ceiling_vertices), sizeof (floor_vertices), GL_ARRAY_BUFFER);
		render_scene->vbos[render_scene->num_objs] = vertex_vbo->buffer_id;
		render_scene->mesh_sizes[render_scene->num_objs] = 4;
		render_scene->obj_names[render_scene->num_objs] = (i == 0 ? "floor" : "ceiling");
		if (i == 0) {
			//Floor material
			init_material (&(render_scene->materials[render_scene->num_objs]), "resources/floor_diffuse.jpg", "resources/floor_specular.png");
		} else {
			//Ceiling material
			init_material (&(render_scene->materials[render_scene->num_objs]), "resources/ceiling_diffuse.jpg", "resources/ceiling_specular.png");
		}
		glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray (0);
		glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof (float)));
		glEnableVertexAttribArray (1);
		glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof (float)));
		glEnableVertexAttribArray (2);
		render_scene->num_objs++;
		matrix_iden4 (&(render_scene->models[idx]));
	}
}

void import_walls (scene* render_scene, char* path) {
	material* wall_mats = malloc (sizeof (material) * 5);
	init_material (&(wall_mats[0]), "resources/theme_1_diffuse.png", "resources/theme_1_specular.png");
	init_material (&(wall_mats[1]), "resources/theme_2_diffuse.png", "resources/theme_2_specular.png");
	init_material (&(wall_mats[2]), "resources/theme_3_diffuse.png", "resources/theme_3_specular.png");
	init_material (&(wall_mats[3]), "resources/theme_4_diffuse.png", "resources/theme_4_specular.png");
	init_material (&(wall_mats[4]), "resources/theme_5_diffuse.png", "resources/theme_5_specular.png");
	FILE* file = fopen (path, "r");
	int x, z;
	int ret;
	int pos = 0;
	int wall_type = 0;
	int wall_idx = 0;
	int num_verts = 0;
	int last_x = -1;
	int last_z = -1;
	float* vert_arr;
	int strip;
	int altern = 0;
	float tex_pos = 0;
	char_buffer* geom = make_char_buffer ();
	while (1) {
		fscanf (file, "%d", &x);
		ret = fscanf (file, "%d", &z);
		
		//Break condition
		if (ret == EOF) {
			break;
		}
		
		if (pos == 0) {
			wall_idx = x;
			num_verts = z;
			vert_arr = malloc ((num_verts) * sizeof (float) * 8 * 2);
			//Wall initialization here
			pos++;
			render_scene->mesh_sizes[render_scene->num_objs] = num_verts * 2;
		} else {
			//Add vertex here
			if (pos != 1) {
				v4 line;
				line.x = wall_idx > 0 ? last_x : x / 32;
				line.y = wall_idx > 0 ? last_z : z / 32;
				line.z = wall_idx > 0 ? x / 32 : last_x;
				line.w = wall_idx > 0 ? z / 32 : last_z;
				char_buffer_add_item (geom, &line, sizeof (v4));
			}
			v3 curr, last, a, b, c;
			initv3 (&curr, x / 32, 0, z / 32);
			initv3 (&last, last_x, 0, last_z);
			vector_diff3 (&a, &last, &curr);
			double len = sqrt (a.x * a.x + a.y * a.y + a.z * a.z);
			if (pos != 0) {
				tex_pos += len / 2;
			}
			vector_normalize3 (&a, &a);
			initv3 (&b, 0, 1, 0);
			vector_cross3 (&c, &a, &b);
			if (wall_idx < 0) {
				vector_scale3 (&c, &c, -1.0);
			}
			
			//Bottom vertex
			int idx = (pos - 1) * 16;
			vert_arr[idx] = x / 32; //Position
			vert_arr[idx + 1] = 0;
			vert_arr[idx + 2] = z / 32;
			vert_arr[idx + 3] = c.x; //Normal
			vert_arr[idx + 4] = 0;
			vert_arr[idx + 5] = c.z;
			vert_arr[idx + 6] = tex_pos; //Texcoords
			vert_arr[idx + 7] = 0;
			//Top vertex
			idx += 8;
			vert_arr[idx] = x / 32; //Position
			vert_arr[idx + 1] = 2;
			vert_arr[idx + 2] = z / 32;
			vert_arr[idx + 3] = c.x; //Normal
			vert_arr[idx + 4] = 0;
			vert_arr[idx + 5] = c.z;
			vert_arr[idx + 6] = tex_pos; //Texcoords
			vert_arr[idx + 7] = 1;
			altern = !altern;
			if (pos == num_verts) {
				//wall is done
				int idx = render_scene->num_objs;
				render_scene->programs[idx] = make_program_from_files ("obj_vertex_shader.glsl", NULL, "obj_frag_shader.glsl");
				glGenVertexArrays (1, &(render_scene->vaos[idx]));
				glBindVertexArray (render_scene->vaos[idx]);
				VBO* vertex_vbo = VBO_init (malloc (sizeof (VBO)), vert_arr, num_verts * sizeof (float) * 8 * 2, GL_ARRAY_BUFFER);
				render_scene->vbos[render_scene->num_objs] = vertex_vbo->buffer_id;
				render_scene->materials[render_scene->num_objs] = wall_mats[abs (wall_idx) - 1];
				render_scene->obj_names[render_scene->num_objs] = "wall_strip";
				glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray (0);
				glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof (float)));
				glEnableVertexAttribArray (1);
				glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof (float)));
				glEnableVertexAttribArray (2);
				render_scene->num_objs++;
				matrix_trans4 (&(render_scene->models[idx]), 0, 0.0, 0);
				pos = 0;
				tex_pos = 0;
			} else {
				pos++;
			}
		}
		last_x = x / 32;
		last_z = z / 32;
	}
	wall_data = (v4*)geom->data;
	num_walls = geom->length / sizeof (v4);
}

void import_objs (scene* render_scene, char* path) {
	
	material plant_mat, health_mat, bombs_mat, speed_mat, boots_mat, dmg_mat;
	init_material (&plant_mat, "resources/plant.png", "resources/theme_5_specular.png");
	init_material (&health_mat, "resources/health.png", "resources/theme_5_specular.png");
	init_material (&bombs_mat, "resources/bombs.png", "resources/theme_5_specular.png");
	init_material (&speed_mat, "resources/speed.png", "resources/theme_5_specular.png");
	init_material (&boots_mat, "resources/boots.png", "resources/theme_5_specular.png");
	init_material (&dmg_mat, "resources/dmg.png", "resources/theme_5_specular.png");
	
	FILE* file = fopen (path, "r");
	int ret, x, z, obj_id, num_pts;
	while (1) {
		
		//EOF check and obj id scan
		ret = fscanf (file, "%d", &obj_id);
		if (ret == EOF) {
			break;
		}

		if (obj_id != 2) {
			fscanf (file, "%d", &x);
			fscanf (file, "%d", &z);
		} else {
			fscanf (file, "%d", &num_pts);
			int i;
			for (i = 0; i < num_pts; i++) {
				fscanf (file, "%d", &x);
				fscanf (file, "%d", &z);
			}
		}
		
		float fx, fz;
		fx = x / 32;
		fz = z / 32;
		switch (obj_id) {
			case 1:
				init_enemy (render_scene, fx, fz);
				break;
			case 2:
				//TODO door
				break;
			case 3:
				init_decoration (render_scene, &plant_mat, fx, fz);
				break;
			case 4:
				init_barrel (render_scene, fx, fz);
				break;
			case 5:
				init_item (render_scene, &bombs_mat, NULL, fx, fz); //TODO callback func
				break;
			case 6:
				init_item (render_scene, &health_mat, NULL, fx, fz); //TODO callback func
				break;
			case 7:
				init_item (render_scene, &speed_mat, NULL, fx, fz); //TODO callback func
				break;
			case 8:
				init_item (render_scene, &boots_mat, NULL, fx, fz); //TODO callback func
				break;
			case 9:
				init_item (render_scene, &dmg_mat, NULL, fx, fz); //TODO callback func
				break;
			default:
				break;
				
		}
		
	}
	
}