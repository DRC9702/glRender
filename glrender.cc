
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "parse_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>


typedef vec4  point4;
typedef vec4  color4;

// source provided for function to load and compile shaders
GLuint InitShader( const char* vertexShaderFile,
                  const char* fragmentShaderFile );


int NumVertices = 0;

vec4  *vertices = nullptr;

bool oldlightingmode = true;


// we render from this setup:

// viewer, light & material definitions, for lighting calculations:
point4 viewer = {0.0, 0.0, 1.0, 1.0};
point4 light_position = {100.0, 100.0, 100.0, 1.0};
color4 light_ambient = {0.2, 0.2, 0.2, 1.0};
color4 light_diffuse = {1.0, 1.0, 1.0, 1.0};
color4 light_specular = {1.0, 1.0, 1.0, 1.0};


// two lighting setups, that we can switch between when the user types "O":
// viewer, light & material definitions, for lighting calculations:
point4 viewer1 = {0.0, 0.0, 1.0, 1.0};
point4 light_position1 = {0.0, 0.0, 1.0, 1.0};
color4 light_ambient1 = {0.2, 0.2, 0.2, 1.0};
color4 light_diffuse1 = {1.0, 1.0, 1.0, 1.0};
color4 light_specular1 = {1.0, 1.0, 1.0, 1.0};


// viewer, light & material definitions, for lighting calculations:
point4 viewer2 = {0.0, 0.0, 10.0, 1.0};
point4 light_position2 = {0., 0., 10., 1.0};
color4 light_ambient2 = {0.2, 0.2, 0.2, 1.0};
color4 light_diffuse2 = {.80, .80, .80, 1.0};
color4 light_specular2 = {.50, .50, .50, 1.0};


// material definitions:
color4 material_ambient = {1.0, 0.0, 1.0, 1.0};
color4 material_diffuse = {1.0, 0.8, 0.0, 1.0};
color4 material_specular = {1.0, 0.8, 0.0, 1.0};
float material_shininess = 100.0;

// we will copy our transformed points to an array stored here:
point4 *points = nullptr;

// and we will store the colors, per face per vertex, here. since there is
// only 1 triangle, with 3 vertices, there will just be 3 here:
//color4 *colors = nullptr;
//No longer tracking colors, but triangle norms
vec4 *norms = nullptr;

// a transformation matrix for the camera //I'm repurposing this variable
mat4x4 ctm;


// "names" for the various buffers, shaders, programs etc:
GLuint vertex_buffer, program;
GLint mvp_location, vpos_location;//, vcol_location; //We no longer want vcol
//We're doing colors in the shader (Gpu) not cpu

//We're gonna need the locations for all the light and material info
GLint lightPos_location, lightDiffuse_location, lightSpecular_location, lightAmbient_location,
	materialDiffuse_location, materialSpecular_location,
	materialAmbient_location, materialShininess_location;

//Gonna need the location for the triangle norms we're tracking
GLint triNorms_location;

//We need to track the eye
GLint eye_location;

//These are our transformation matrices
GLint view_location, projection_location;
mat4x4 look; //This is our view transformation matrix
mat4x4 perspective; //This is our projection transformation matrix

// for keeping track of the rotation or translation:
float theta = 0.0;  // mouse rotation around the Y (up) axis
float posx = 0.0;   // translation along X
float posy = 0.0;   // translation along Y
float phi = 90.0; //This is the other angle for spherical coordinates
float radius = 3.0; //Radius for spherical coordinates

//Our up vector and origin point
vec4 up = {0.0, 1.0, 0.0, 0.0};
point4 origin = {0.0, 0.0, 0.0, 1.0};


// for converting:
const float deg_to_rad = (3.1415926 / 180.0);


// three helper functions for the vec4 class:
void vecproduct(vec4 &res, const vec4 &v1, const vec4 &v2) {
    for (int i = 0; i < 4; ++i) res[i] = v1[i] * v2[i];
}

void vecset(vec4 &res, const vec4 &v1) {
    for (int i = 0; i < 4; ++i) res[i] = v1[i];
}

void vecclear(vec4 &res) {
    for (int i = 0; i < 4; ++i) res[i] = 0.0;
}

// transform the ith triangle's vertex data and put it into the points array.
// also, compute the lighting at each vertex, and put that into the colors
// array.
void tri(int i)
{
    // compute the lighting at each vertex, then set it as the color there:

//    // transform the points by the rotation matrix:
//    mat4x4_mul_vec4 (points[3*i+0], ctm, vertices[3*i+0]);
//    mat4x4_mul_vec4 (points[3*i+1], ctm, vertices[3*i+1]);
//    mat4x4_mul_vec4 (points[3*i+2], ctm, vertices[3*i+2]);
    
	for(int j=0; j<3; j++){
		vecset(points[3*i+j],vertices[3*i+j]);
	}

    // compute the triangle's normal:
    vec4 e1, e2, n;
    vec4_sub(e1, points[3*i+1], points[3*i+0]);
    vec4_sub(e2, points[3*i+2], points[3*i+0]);
    vec4_mul_cross(n, e1, e2);
    n[3] = 0.f; // cross product in 4d sets this value to 1, which we do not want...
    vec4_norm(n, n);
    for (int j = 0; j < 3; ++j){
    	vecset(norms[3*i+j],n);
    }

}



static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    // exit if the ESC key is pressed:
    if (key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    
//    // if it's O or SHIFT-O, toggle between the 2 lighting setups:
//    if (key == GLFW_KEY_O and mods == GLFW_MOD_SHIFT) {
//        vecset(viewer, viewer2);
//        vecset(light_position, light_position2);
//        vecset(light_ambient, light_ambient2);
//        vecset(light_diffuse, light_diffuse2);
//        vecset(light_specular, light_specular2);
//        std::cout << "Shift O" << std::endl;
//    }
//    else if(key == GLFW_KEY_O) {
//        vecset(viewer, viewer1);
//        vecset(light_position, light_position1);
//        vecset(light_ambient, light_ambient1);
//        vecset(light_diffuse, light_diffuse1);
//        vecset(light_specular, light_specular1);
//        std::cout << "O" << std::endl;
//    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS){
    	radius++;
    	if(radius > 50)
    		radius = 50;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS){
    	radius--;
    	if(radius < 3)
    		radius = 3;
    }

//    //Resending the light and material info to the shader because you're changing it
//    glUniform4fv(eye_location,1,viewer);
//	glUniform4fv(lightPos_location,1,light_position);
//	glUniform4fv(lightDiffuse_location,1,light_diffuse);
//	glUniform4fv(lightSpecular_location,1,light_specular);
//	glUniform4fv(lightAmbient_location,1,light_ambient);
//	glUniform4fv(materialDiffuse_location,1,material_diffuse);
//	glUniform4fv(materialSpecular_location,1,material_specular);
//	glUniform4fv(materialAmbient_location,1,material_ambient);
//	glUniform1f(materialShininess_location,material_shininess);
//	glfwPollEvents();

}

// set up the vertex data buffers and the program (shaders):
void init()
{
    
    // set up vertex buffer object - this will be memory on the GPU where
    // we are going to store our vertex data (that is currently in the "points"
    // array)
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    
    
    // specify that its used for vertex data, what its size is, and where the
    // data is located, and finally a "hint" about how we are going to use
    // the data (the driver will put it in a good memory location, hopefully)
    glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(color4))*NumVertices,
                 nullptr, GL_DYNAMIC_DRAW);
    
    // load in these two shaders...  (note: InitShader is defined in the
    // accompanying initshader.c code).
    // the shaders themselves must be text glsl files in the same directory
    // as we are running this program:
    program = InitShader("vshader_passthrough_lit.glsl", "fshader_passthrough_lit.glsl");
    
    // ...and set them to be active
    glUseProgram(program);
    
    // get access to the various things we will be sending to the shaders:
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    //vcol_location = glGetAttribLocation(program, "vCol");
    triNorms_location = glGetAttribLocation(program, "triNorms");
    
    //We need info of where the eye is
    eye_location = glGetUniformLocation(program, "eye");
    view_location = glGetUniformLocation(program, "view");
    projection_location = glGetUniformLocation(program, "projection");

    //Get access to the light and material info we'll send to the shaders
    lightPos_location = glGetUniformLocation(program,"lightPos");
    lightDiffuse_location = glGetUniformLocation(program, "lightDiffuse");
    lightSpecular_location = glGetUniformLocation(program, "lightSpecular");
    lightAmbient_location = glGetUniformLocation(program, "lightAmbient");
    materialDiffuse_location = glGetUniformLocation(program, "materialDiffuse");
    materialSpecular_location = glGetUniformLocation(program, "materialSpecular");
    materialAmbient_location = glGetUniformLocation(program, "materialAmbient");
    materialShininess_location = glGetUniformLocation(program, "materialShininess");

    //Sending the light and material info to the shader
    glUseProgram(program);

    point4 eyeTemp;
    eyeTemp[0] = radius * cosf(theta*deg_to_rad) * sinf(phi*deg_to_rad);
    eyeTemp[1] = radius * cosf(phi*deg_to_rad);
    eyeTemp[2] = radius * sinf(theta*deg_to_rad) * sinf(phi*deg_to_rad);
    eyeTemp[3] = 1;
    glUniform4fv(eye_location,1,eyeTemp);


    glUniform4fv(lightPos_location,1,light_position);
    glUniform4fv(lightDiffuse_location,1,light_diffuse);
    glUniform4fv(lightSpecular_location,1,light_specular);
    glUniform4fv(lightAmbient_location,1,light_ambient);
    glUniform4fv(materialDiffuse_location,1,material_diffuse);
    glUniform4fv(materialSpecular_location,1,material_specular);
    glUniform4fv(materialAmbient_location,1,material_ambient);
    glUniform1f(materialShininess_location,material_shininess);


    glEnableVertexAttribArray(vpos_location);
    
    // the vPosition attribute is a series of 4-vecs of floats, starting at the
    // beginning of the buffer
    glVertexAttribPointer(vpos_location, 4, GL_FLOAT, GL_FALSE,
                          0, (void*) (0));
    
    
    //glEnableVertexAttribArray(vcol_location);
    //glVertexAttribPointer(vcol_location, 4, GL_FLOAT, GL_FALSE,
                          //0, (void*) (sizeof(point4) * NumVertices));

    //Instead of tracking colors, we can track triangle norms in the same way
    glEnableVertexAttribArray(triNorms_location);
    glVertexAttribPointer(triNorms_location, 4, GL_FLOAT, GL_FALSE,
    		0, (void*) (sizeof(point4) * NumVertices));
}

// use this motionfunc to demonstrate rotation - it adjusts "theta" based
// on how the mouse has moved.
static void mouse_move_rotate (GLFWwindow* window, double x, double y)
{
    static float lastx = 0;// keep track of where the mouse was last:
    static float lasty = 0;// keep track of where the mouse was last:
    
    float amntX = x - lastx;
    
    if (amntX != 0.)
        theta +=  amntX;
    
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0 )  theta += 360.0;
        
    lastx = x;

    int amntY = y - lasty;
	if (amntY != 0) {
		phi +=  amntY;
		if (phi > 175.0 ) phi = 175.0;
		if (phi < 5.0 ) phi = 5.0;
	}

	lasty = y;
}


void load_obj_file (char *filename)
{
    
    std::vector<int> tri_ids;
    std::vector<float> tri_verts;
    
    read_wavefront_file(filename, tri_ids, tri_verts);
    
    NumVertices = tri_ids.size();
    
    vertices = new point4[NumVertices];
    points   = new point4[NumVertices];
    //colors   = new color4[NumVertices];
    norms = new point4[NumVertices];
    
    // tri_ids is a list of the vertex indices for each triangle, so the first
    // triangle uses up the first 3 indices, etc.
    for (int k = 0; k < tri_ids.size() / 3; ++k) {
        
        vertices[3*k][0] = tri_verts[3*tri_ids[3*k]];
        vertices[3*k][1] = tri_verts[3*tri_ids[3*k]+1];
        vertices[3*k][2] = tri_verts[3*tri_ids[3*k]+2];
        vertices[3*k][3] = 1.;
        
        vertices[3*k+1][0] = tri_verts[3*tri_ids[3*k+1]];
        vertices[3*k+1][1] = tri_verts[3*tri_ids[3*k+1]+1];
        vertices[3*k+1][2] = tri_verts[3*tri_ids[3*k+1]+2];
        vertices[3*k+1][3] = 1.;
        
        vertices[3*k+2][0] = tri_verts[3*tri_ids[3*k+2]];
        vertices[3*k+2][1] = tri_verts[3*tri_ids[3*k+2]+1];
        vertices[3*k+2][2] = tri_verts[3*tri_ids[3*k+2]+2];
        vertices[3*k+2][3] = 1.;
        
    }
    
}

static void displayVec4(vec4& bap){
	std::cout << "[" << bap[0] << ", " << bap[1] << ", " << bap[2] << ", " << bap[3] << "]" << std::endl;
}

int main(int argc, char** argv)
{

    if (argc != 2) {
        std::cout << "usage: glrender objfile" << std::endl;
        exit (0);
    }
        
    // if there are errors, call this routine:
    glfwSetErrorCallback(error_callback);

    // start up GLFW:
    if (!glfwInit())
        exit(EXIT_FAILURE);

    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    
    // for more modern version of OpenGL:
    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(640, 480, "hello triangle transform!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    
    // call only once: demo for rotation:
    glfwSetCursorPosCallback(window, mouse_move_rotate);
    

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // set the background to white:
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    // enable the z-buffer for depth tests:
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // clear everything:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // read the obj file and put it into the points array:
    load_obj_file (argv[1]);

    init();



	//You shouldn't be doing this everytime in the loop, maybe just once.

	// tri() will multiply the points by ctm, and figure out the lighting
	for (int i = 0; i < NumVertices / 3; ++i) {
	   tri(i);
	}

	// tell the VBO to re-get the data from the points and colors arrays:
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*NumVertices, points );
	//glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*NumVertices, sizeof(color4)*NumVertices, colors );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*NumVertices, sizeof(color4)*NumVertices, norms );

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 p;

        // in case the  window viewport size changed, we will need to adjust the
        // projection:
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        
        // clear the window (with white) and clear the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        std::cout << "Theta[" << theta << "]" << std::endl;
        //displayVec4(viewer);
        point4 eyeTemp;
        vecclear(eyeTemp);


        // make up a transform that rotates around screen "Z" with time:
//        mat4x4_identity(ctm);
//		//mat4x4_translate(ctm, 0,0,radius);
//		mat4x4_rotate_X(ctm, ctm, phi * deg_to_rad);
//		mat4x4_rotate_Y(ctm, ctm, theta * deg_to_rad);
//		mat4x4_mul_vec4 (eyeTemp, ctm, viewer);
        eyeTemp[0] = radius * cosf(theta*deg_to_rad) * sinf(phi*deg_to_rad);
        eyeTemp[1] = radius * cosf(phi*deg_to_rad);
        eyeTemp[2] = radius * sinf(theta*deg_to_rad) * sinf(phi*deg_to_rad);
        eyeTemp[3] = 1;


		displayVec4(eyeTemp);
		vec3_norm(eyeTemp, eyeTemp);
		displayVec4(eyeTemp);
		vec4_scale(eyeTemp,eyeTemp,radius);


	    glUseProgram(program);
		glUniform4fv(eye_location,1,eyeTemp);


        // orthographically project to screen:
        //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, -1.f, 1.f);
		mat4x4_look_at(look, eyeTemp, origin, up);
		mat4x4_perspective(perspective, 45.0, ratio, 0.1, 100.0);

		displayVec4(eyeTemp);
		//displayVec4(origin);
		//displayVec4(up);

		std::cout << "p00[" << p[0][0] << "]" << std::endl;
        
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) p);

        glUniformMatrix4fv(view_location, 1, GL_FALSE, (const GLfloat*) look);
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, (const GLfloat*) perspective);

        glDrawArrays(GL_TRIANGLES, 0, NumVertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // free up GLFW objects:
    glfwDestroyWindow(window);

    // free up GL objects:
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(program);
    
    // free up memory:
    delete [] vertices;
    delete [] points;
    //delete [] colors;
    delete[] norms;
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}



