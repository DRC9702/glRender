
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

typedef vec4  point4;
typedef vec4  color4;

// source provided for function to load and compile shaders
GLuint InitShader( const char* vertexShaderFile,
                  const char* fragmentShaderFile );


//const int NumVertices = 3;

int NumVertices = -1;
std::vector<vec4> verts;// = std::vector<vec4>();
std::vector<int> tris;// = std::vector<int> ();

void readObjFile(const char *file, std::vector< int > &tris, std::vector< vec4 > &verts, point4* &points, color4* &colors){
// clear out the tris and verts vectors:
	tris.clear ();
	verts.clear ();

	std::ifstream in(file);
	char buffer[1025];
	std::string cmd;

	for (int line=1; in.good(); line++) {
//    	cout << "Reading in line[" << line << "] of wavefront file." << endl;
		in.getline(buffer,1024);
		buffer[in.gcount()]=0;

		cmd="";

		std::istringstream iss (buffer);

		iss >> cmd;

		//cout << "trisSize: " << tris.size() << endl;

		if (cmd[0]=='#' or cmd.empty()) {
			// ignore comments or blank lines
			continue;
		}
		else if (cmd=="v") {
			// got a vertex:

			// read in the parameters:
			float pa, pb, pc;
			iss >> pa >> pb >> pc;
//			verts.push_back (pa);
//			verts.push_back (pb);
//			verts.push_back (pc);
			//vec4 vertex = vec4(pa,pb,pc, 1.0);
			//const float vertex[4] = {pa,pb,pc, 1.0};
			vec4 vertex = new float[]{pa,pb,pc,1.0};
			verts.push_back(vertex);
			//verts.emplace_back(vertex);
		 }
		else if (cmd=="f") {
			// got a face (triangle)

			// read in the parameters:
			int i, j, k;
			iss >> i >> j >> k;

			// vertex numbers in OBJ files start with 1, but in C++ array
			// indices start with 0, so we're shifting everything down by
			// 1
			tris.push_back (i-1);
			tris.push_back (j-1);
			tris.push_back (k-1);
		}
		else {
			std::cerr << "Parser error: invalid command at line " << line << std::endl;
		}

	 }

	in.close();
	NumVertices = verts.size();

 //   std::cout << "found this many tris, verts: " << tris.size () / 3.0 << "  " << verts.size () / 3.0 << std::endl;
}

// x, y, r, g, b for each of 3 vertices.


//vec4  vertices[3] = {
//    {-0.25,0.0,0.0, 1.0},
//    {0.25,0.0,0.0, 1.0},
//    {0.0, 0.5,0.0, 1.0}};

// viewer's position, for lighting calculations
point4 viewer = {0.0, 0.0, -1.0, 1.0};

// light & material definitions, again for lighting calculations:
point4 light_position = {0.0, 0.0, -1.0, 1.0};
color4 light_ambient = {0.2, 0.2, 0.2, 1.0};
color4 light_diffuse = {1.0, 1.0, 1.0, 1.0};
color4 light_specular = {1.0, 1.0, 1.0, 1.0};

color4 material_ambient = {1.0, 0.0, 1.0, 1.0};
color4 material_diffuse = {1.0, 0.8, 0.0, 1.0};
color4 material_specular = {1.0, 0.8, 0.0, 1.0};
float material_shininess = 100.0;

// we will copy our transformed points to here:
//point4 points[NumVertices];
point4 *points;

// and we will store the colors, per face per vertex, here. since there is
// only 1 triangle, with 3 vertices, there will just be 3 here:
//color4 colors[NumVertices];
color4 *colors;

// a transformation matrix, for the rotation, which we will apply to every
// vertex:
mat4x4 ctm;


// "names" for the various buffers, shaders, programs etc:
GLuint vertex_buffer, program;
GLint mvp_location, vpos_location, vcol_location;

float theta = 0.0;  // mouse rotation around the Y (up) axis
float posx = 0.0;   // translation along X
float posy = 0.0;   // translation along Y

const float deg_to_rad = (3.1415926 / 180.0);


// three helper functions for the vec4 class:
void vecproduct (vec4 &res, const vec4 &v1, const vec4 &v2) {
    for (int i = 0; i < 4; ++i)
        res[i] = v1[i] * v2[i];
}

void vecset(vec4 &res, const vec4 &v1)
{
    for (int i = 0; i < 4; ++i)
        res[i] = v1[i];
}

void vecclear(vec4 &res)
{
    for (int i = 0; i < 4; ++i)
        res[i] = 0.0;
}

// transform the triangle's vertex data and put it into the points array.
// also, compute the lighting at each vertex, and put that into the colors
// array.
void tri()
{
    // compute the lighting at each vertex, then set it as the color there:
	for(int j=0; j<NumVertices; j+=3){
		mat4x4_mul_vec4 (points[tris[j+0]], ctm, verts[tris[j+0]]);
		mat4x4_mul_vec4 (points[tris[j+1]], ctm, verts[tris[j+1]]);
		mat4x4_mul_vec4 (points[tris[j+2]], ctm, verts[tris[j+2]]);

		vec4 e1, e2, n;
		vec4_sub(e1, points[tris[j+1]], points[tris[j+0]]);
		vec4_sub(e2, points[tris[j+2]], points[tris[j+0]]);
		vec4_mul_cross(n, e1, e2);
		n[3] = 0.f; // cross product in 4d sets this to 1.0, which we do not want
		vec4_norm(n, n);

		color4 ambient_color, diffuse_color, specular_color;

		vecclear(diffuse_color);
		vecclear(specular_color);
		vecclear(ambient_color);

		vecproduct(ambient_color, material_ambient, light_ambient);
		color4 diffuse_product, spec_product;
		vecproduct(diffuse_product, light_diffuse, material_diffuse);
		vecproduct(spec_product, light_specular, material_specular);


		for (int i = 0; i < 3; ++i) {
			vec4 light_vec, view_vec;
			vec4_sub(light_vec, light_position, points[tris[j+i]]);
			vec4_norm(light_vec, light_vec);

			float dd1 = vec4_mul_inner(light_vec, n);

			if(dd1>0.0)
				vec4_scale(diffuse_color, diffuse_product, dd1);

			// compute the half vector, for specular reflection:
			vec4_sub(view_vec, viewer, points[j+i]);
			vec4_norm(view_vec, view_vec);
			vec4 half;
			vec4_add(half, light_vec, view_vec);
			vec4_norm(half, half);

			float dd2 = vec4_mul_inner(half, n);

			if(dd2 > 0.0)
				vec4_scale (specular_color, spec_product, exp(material_shininess*log(dd2)));

			vec4_add (colors[tris[j+i]], ambient_color, diffuse_color);
			vec4_add (colors[tris[j+i]], colors[tris[j+i]], specular_color);
			colors[tris[j+i]][3] = 1.0;

		}
	}

}



static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void init()
{

    // set up vertex buffer object - this will be memory on the GPU where
    // we are going to store our vertex data (that is currently in the "points"
    // array)
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);


    // specify that its part of a VAO, what its size is, and where the
    // data is located, and finally a "hint" about how we are going to use
    // the data (the driver will put it in a good memory location, hopefully)
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
                 NULL, GL_DYNAMIC_DRAW);

    // load in these two shaders...  (note: InitShader is defined in the
    // accompanying initshader.c code).
    // the shaders themselves must be text glsl files in the same directory
    // as we are running this program:
    program = InitShader("vshader_passthrough_lit.glsl", "fshader_passthrough_lit.glsl");

    // ...and set them to be active
    glUseProgram(program);

    // get acces to the various things we will be sending to the shaders:
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);

    // the vPosition attribute is a series of 4-vecs of floats, starting at the
    // beginning of the buffer
    glVertexAttribPointer(vpos_location, 4, GL_FLOAT, GL_FALSE,
                          0, (void*) (0));


    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 4, GL_FLOAT, GL_FALSE,
                          0, (void*) (sizeof(points)));
}

// use this motionfunc to demonstrate rotation - it adjusts "theta" based
// on how the mouse has moved.
static void mouse_move_rotate (GLFWwindow* window, double x, double y)
{

    static int lastx = 0;// keep track of where the mouse was last:

    int amntX = x - lastx;
    if (amntX != 0) {
        theta +=  amntX;
        if (theta > 360.0 ) theta -= 360.0;
        if (theta < 0.0 ) theta += 360.0;

        lastx = x;
    }

  //  std::cout << theta << std::endl;
}

// use this motionfunc to demonstrate translation - it adjusts posx and
// posy based on the mouse movement. posx and posy are then used in the
// display loop to generate the transformation that is applied
// to all the vertices before they are displayed:
static void mouse_move_translate (GLFWwindow* window, double x, double y)
{

    static int lastx = 0;
    static int lasty = 0;

    // if we want relative motion, keep track of where the mouse was last:
//    
//    if (x - lastx < 0) --posx;
//    else if (x - lastx > 0) ++posx;
//    lastx = x;
//    
//    if (y - lasty < 0) --posy;
//    else if (y - lasty > 0) ++posy;
//    lasty = y;

    posx = x;
    posy = -y;
}



int main(int argc, char *argv[])
{

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

    std::vector<vec4> verts = std::vector<vec4>();
    std::vector<int> tris = std::vector<int> ();
    readObjFile(argv[1], tris, verts, points, colors);
//	points = point4[NumVertices];
//	colors = color4[NumVertices];
	point4 points[NumVertices];
	color4 colors[NumVertices];

    init();

    // enable z-buffering if you are loading inmore than one triangle -
    // this keeps things closer to the screen drawn in front!
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

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
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // make up a transform that rotates around screen "Z" with time:
        mat4x4_identity(ctm);
        mat4x4_rotate_Y(ctm, ctm, theta * deg_to_rad);

        // tri() will multiply the points by ctm, and figure out the lighting too
        tri();

        // tell the VBO to re-get the data from the points and colors arrays:
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
        glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

        // orthographically project to screen:
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);

        // send that orthographic projection to the device, where the shader
        // will apply it:
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) p);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
