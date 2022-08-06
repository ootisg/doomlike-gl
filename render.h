#ifndef RENDER_H
#define RENDER_H

#include "matrix.h"
#include "textures_gl.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct material {
	texture* diff_map;
	texture* spec_map;
	float shininess;
};

typedef struct material material;

struct scene { //TODO for now, this just contains a list of VAOs, a list of programs, and a list of model matrices
	int num_objs;
	char** obj_names;
	int* mesh_sizes;
	GLuint* vaos;
	GLuint* programs;
	material* materials;
	mat4* models;
};

typedef struct scene scene;

/// Initializes a new scene struct at the given pointer, then returns it.
/// @param loc the location to initialie the struct at
/// @return loc cast to scene*
scene* init_scene (void* loc);

material* init_material (void* loc, char* diffuse_path, char* specular_path);

/// Initializes the rendering pipeline for the given scene
/// @param init_scene the scene to initialize
void render_init (scene* init_scene);

/// Renders a frame of the given scene
/// @param render_scene the scene to render
void render_frame (scene* render_scene);

#endif