#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "inputs.h"
#include "buffers_gl.h"
#include "shaders_gl.h"
#include "textures_gl.h"
#include "render.h"
#include "matrix.h"
#include "room.h"

#define WINDOW_RESOLUTION_WIDTH 1280
#define WINDOW_RESOLUTION_HEIGHT 720

float obj_vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

float light_vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

scene render_scene;

void framebuffer_size_callback (GLFWwindow* window, int width, int height) {
	glViewport (0, 0, width, height);
}

void init () {
	
	init_scene (&render_scene);
	textures_init ();
	render_init (&render_scene);
	init_floor (&render_scene);
	import_walls (&render_scene, "map.txt");
	
	/*texture* tex1 = texture_load_from_file (malloc (sizeof (texture)), "resources/container2.png");
	texture* tex2 = texture_load_from_file (malloc (sizeof (texture)), "resources/container2_specular.png");
	texture_enable (tex1);
	texture_enable (tex2); */
	
	//Object 0 (cube)
	/*render_scene.programs[0] = make_program_from_files ("obj_vertex_shader.glsl", NULL, "obj_frag_shader.glsl");
	glGenVertexArrays (1, &(render_scene.vaos[0]));
	glBindVertexArray (render_scene.vaos[0]);
	VBO* vertex_vbo = VBO_init (malloc (sizeof (VBO)), obj_vertices, sizeof (obj_vertices), GL_ARRAY_BUFFER);
	texture* tex1 = texture_load_from_file (malloc (sizeof (texture)), "resources/container2.png");
	texture* tex2 = texture_load_from_file (malloc (sizeof (texture)), "resources/container2_specular.png");
	texture_enable (tex1);
	texture_enable (tex2);  
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof (float)));
	glEnableVertexAttribArray (1);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof (float)));
	glEnableVertexAttribArray (2);
	render_scene.num_objs++;
	render_scene.mesh_sizes[0] = 36;
	matrix_trans4 (&(render_scene.models[0]), 0.0, 0.0, 0.0);*/
	
		//Object 0 (cube)
	/*render_scene.programs[1] = make_program_from_files ("obj_vertex_shader.glsl", NULL, "obj_frag_shader.glsl");
	glGenVertexArrays (1, &(render_scene.vaos[1]));
	glBindVertexArray (render_scene.vaos[1]);
	vertex_vbo = VBO_init (malloc (sizeof (VBO)), obj_vertices, sizeof (obj_vertices), GL_ARRAY_BUFFER);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof (float)));
	glEnableVertexAttribArray (1);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof (float)));
	glEnableVertexAttribArray (2);
	render_scene.num_objs++;
	render_scene.mesh_sizes[1] = 36;
	matrix_trans4 (&(render_scene.models[1]), 3.0, 0.0, 0.0);*/
	
}

int main () {
	
	//Initialize GLFW
    glfwInit ();
	printf ("GLFW Initialized\n");
	
	//Set the window hints
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//Create the window
	GLFWwindow* window = glfwCreateWindow (WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT, "Window Title", NULL, NULL);
	if (window == NULL) {
		//Window failed to initialize
		printf ("Failed to create GLFW window\n");
		glfwTerminate ();
		return -1;
	}
	glfwMakeContextCurrent (window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	//Init GLEW
	glewInit ();
	
	//Set the initial viewport
	glViewport (0, 0, WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT);
	
	//Set the resize callback
	glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);
	
	//Init rendering code
	init ();
	
	//Display/event loop until the window is closed
	while (!glfwWindowShouldClose (window)) {
		
		//Input detection
		inputs_register_callbacks (window);
		
		//Render
		render_frame (&render_scene);
		
		//Swap buffers and poll events
		glfwSwapBuffers (window);
		glfwPollEvents ();
		
	}
  
	//Terminate GLFW and exit
	glfwTerminate ();
    return 0;
	
}