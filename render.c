#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "render.h"
#include "matrix.h"
#include "inputs.h"
#include "camera.h"
#include "render_info.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float last_frame = 0;
float delta_time = -1;

v3 camera_pos, camera_direction, camera_up;

GLfloat lightPos[3];

float cubePositions[] = {
    0.0f,  0.0f,  0.0f, 
    2.0f,  5.0f, -15.0f, 
    -1.5f, -2.2f, -2.5f,  
    -3.8f, -2.0f, -12.3f,  
    2.4f, -0.4f, -3.5f,  
    -1.7f,  3.0f, -7.5f,  
    1.3f, -2.0f, -2.5f,  
    1.5f,  2.0f, -2.5f, 
    1.5f,  0.2f, -1.5f, 
    -1.3f,  1.0f, -1.5f  
};

float pointLightPositions[] = {
	0.7f,  0.2f,  2.0f,
	35.0f, 1.0f, 35.0f,
	-4.0f,  2.0f, -12.0f,
	0.0f,  0.0f, -3.0f
}; 

float frame_delta_time () {
	return delta_time;
}

scene* init_scene (void* loc) {
	mat4 sample, result;
	matrix_init4 (&sample, 1, 2, 4, 0,
						   2, 9, 4, 5,
						   14, 99, 12, 4,
						   14, 18, 2, 0);
	matrix_print4 (&sample);
	matrix_inverse4 (&result, &sample);
	scene* ptr = (scene*)loc;
	ptr->num_objs = 0;
	ptr->vaos = malloc (sizeof (void*) * 128);
	ptr->programs = malloc (sizeof (void*) * 128);
	ptr->models = malloc (sizeof (mat4) * 128);
	ptr->mesh_sizes = malloc (sizeof (int*) * 128);
	ptr->materials = malloc (sizeof (material) * 128);
	lightPos[0] = 3.0;
	lightPos[1] = 1.0;
	lightPos[2] = 3.0;
}

material* init_material (void* loc, char* diffuse_path, char* specular_path) {
	material* mat = (material*)loc;
	mat->diff_map = texture_load_from_file (malloc (sizeof (texture)), diffuse_path);
	mat->spec_map = texture_load_from_file (malloc (sizeof (texture)), specular_path);
	return mat;
}

void render_init (scene* init_scene) {
	glEnable(GL_DEPTH_TEST); 
	camera* cam = camera_init (malloc (sizeof (camera)));
	set_active_camera (cam);
}

//Assumes a large enough buffer in dest
char* getuniformarrstr (char* dest, char* name, int index, char* attribute) {
	sprintf (dest, "%s[%d].%s", name, index, attribute);
	return dest;
}

void render_frame (scene* render_scene) {
	
	//Save timings related to deltaTime calculation
	float curr_time = glfwGetTime ();
	if (last_frame == 0) {
		last_frame = curr_time;
	}
	delta_time = curr_time - last_frame;
	last_frame = curr_time;
	
	//Process key inputs for camera controls
	camera_process_key_inputs ();
	
	//Clear the screen
	glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	camera* cam = get_active_camera ();
	
	//Setup the view matrix
	mat4* view = camera_get_view_matrix (cam);
	
	//Setup the perspective matrix
	mat4* proj = camera_get_proj_matrix (cam);
	
	int i;
	for (i = 0; i < render_scene->num_objs; i++) {
		
		//Set the correct shader program...
		int model_idx = i;
		glUseProgram (render_scene->programs[model_idx]);
		glBindVertexArray (render_scene->vaos[model_idx]);
		
		//Select the correct point light
		if (i > 0) {
			lightPos[0] = pointLightPositions [i * 3];
			lightPos[1] = pointLightPositions [i * 3 + 1];
			lightPos[2] = pointLightPositions [i * 3 + 2];
		}
		
		//Render
		mat4 lightModel;
		//Set up uniforms for the object shader
		texture_enable (render_scene->materials[i].diff_map);
		texture_enable (render_scene->materials[i].spec_map);
		glUniform1i (glGetUniformLocation (render_scene->programs[0], "material.diffuse"), render_scene->materials[i].diff_map->texture_unit_idx);
		glUniform1i (glGetUniformLocation (render_scene->programs[0], "material.specular"), render_scene->materials[i].spec_map->texture_unit_idx); //We really ought to be querying the texture structs here instead of assuming their texture units
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "eyePos"), cam->pos.x, cam->pos.y, cam->pos.z);
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "material.specular"), 0.5, 0.5, 0.5);
		glUniform1f (glGetUniformLocation (render_scene->programs[0], "material.shininess"), 32.0);
		
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "dirLight.ambient"), 0.1, 0.1, 0.1);
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "dirLight.specular"), 1.0, 1.0, 1.0);
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "dirLight.diffuse"), 0.0, 0.0, 0.0);
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "dirLight.direction"), 0.0, -1.0, 0.0);
		glUniform1f (glGetUniformLocation (render_scene->programs[0], "light.cutoff"), cos (.436332));
		glUniform1f (glGetUniformLocation (render_scene->programs[0], "light.outerCutoff"), cos (0.610865));
		
		pointLightPositions [0] = cam->pos.x;
		pointLightPositions [1] = cam->pos.y;
		pointLightPositions [2] = cam->pos.z;
		char buff[1024];
		int lidx;
		for (lidx = 0; lidx < 4; lidx++) {
			glUniform3f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "position")), pointLightPositions[lidx * 3], pointLightPositions[lidx * 3 + 1], pointLightPositions[lidx * 3 + 2]);
			glUniform3f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "ambient")), 0.2, 0.2, 0.2);
			glUniform3f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "diffuse")), 0.8, 0.8, 0.8);
			glUniform3f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "specular")), 1.0, 1.0, 1.0);
			if (lidx == 0) {
				//Player light
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "constant")), 1.0);
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "linear")), 0.09);
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "quadratic")), 0.032);
			} else {
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "constant")), 1.0);
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "linear")), 0.14);
				glUniform1f (glGetUniformLocation (render_scene->programs[0], getuniformarrstr (buff, "pointLights", lidx, "quadratic")), 0.07);
			}
		}
		
		if (flashlight_active ()) {
			glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.diffuse"), 0.8, 0.8, 0.8);
			glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.specular"), 1.0, 1.0, 1.0);
		} else {
			glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.diffuse"), 0.0, 0.0, 0.0);
			glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.specular"), 0.0, 0.0, 0.0);
		}
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.position"), cam->pos.x, cam->pos.y, cam->pos.z);
		glUniform3f (glGetUniformLocation (render_scene->programs[0], "spotlight.direction"), cam->dir.x, cam->dir.y, cam->dir.z);
		glUniform1f (glGetUniformLocation (render_scene->programs[0], "spotlight.innerCutoff"), cos (.436332)); //25 degrees
		glUniform1f (glGetUniformLocation (render_scene->programs[0], "spotlight.outerCutoff"), cos (.610865)); //35 degrees
		
		
		//Send the model, view, and proj matrices to the vertex shader as uniforms
		GLfloat* gl_mat = malloc (sizeof (GLfloat) * 16);
		mat4 a, b, cubeModel;
		mat4* model = &(render_scene->models[model_idx]);
		mat4 normal;
		matrix_inverse4 (&a, model);
		matrix_transpose4 (&normal, &a);
		to_gl_matrix4 (gl_mat, model);
		glUniformMatrix4fv (glGetUniformLocation (render_scene->programs[model_idx], "model"), 1, GL_FALSE, gl_mat);
		to_gl_matrix4 (gl_mat, &normal);
		glUniformMatrix4fv (glGetUniformLocation (render_scene->programs[model_idx], "normal"), 1, GL_FALSE, gl_mat);
		to_gl_matrix4 (gl_mat, view);
		glUniformMatrix4fv (glGetUniformLocation (render_scene->programs[model_idx], "view"), 1, GL_FALSE, gl_mat);
		to_gl_matrix4 (gl_mat, proj);
		glUniformMatrix4fv (glGetUniformLocation (render_scene->programs[model_idx], "proj"), 1, GL_FALSE, gl_mat);
		free (gl_mat);
		
		//Render the vertices
		glDrawArrays (GL_TRIANGLE_STRIP, 0, render_scene->mesh_sizes[model_idx]);
		
		//Disable the textures
		texture_disable (render_scene->materials[i].diff_map);
		texture_disable (render_scene->materials[i].spec_map);
	
	}
	
}