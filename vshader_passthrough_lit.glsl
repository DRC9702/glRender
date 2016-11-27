#version 120



// input transform from the host program, for an orthographic projection (see
// the class notes and the code that makes this projection to verify!).
// a rotation around the world Y (up) axis has already been applied on the host.
uniform mat4 MVP;


// we are going to be getting color values, per-vertex, from the main
// program running on the cpu (and that are then stored in the VBO on the
// card. this color, called "vCol", is just going to be passed through
// to the fragment shader, which will intrpolate its value per-fragment
// amongst the 3 vertex colors on the triangle that fragment is a part of.
//
// on mac you may have to say: "attribute" instead of "in":
attribute vec4 vCol;

// we are going to be getting an attribute from the main program, named
// "vPos", one for each vertex.
//
// on mac you may have to say: "attribute vec4 vPosition;" instead of "in...":
attribute vec4 vPos;

// our shader outputs one color value per vertex: we will send it to the
// fragment shader, and along the way we want the device to interpolate
// the per-vertex values:
varying vec4 color;

void main()
{
    gl_Position = MVP * vPos;
    color = vCol;
}


