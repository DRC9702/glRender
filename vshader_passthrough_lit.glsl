#version 120


//This is the stuff we need for 2.1
uniform vec4 lightPos;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 lightAmbient;

uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform vec4 materialAmbient;
uniform float materialShininess;

uniform vec4 eye;

//We're gonna need to track all the triangle normals as attributes
attribute vec4 triNorms;


// input transform from the host program, for an orthographic projection (see
// the class notes and the code that makes this projection to verify!).
// a rotation around the world Y (up) axis has already been applied on the host.
uniform mat4 MVP;

//We need two transformation matrices. The view(lookat) and the projection(projection).
uniform mat4 view;
uniform mat4 projection;


// we are going to be getting color values, per-vertex, from the main
// program running on the cpu (and that are then stored in the VBO on the
// card. this color, called "vCol", is just going to be passed through
// to the fragment shader, which will intrpolate its value per-fragment
// amongst the 3 vertex colors on the triangle that fragment is a part of.
//
// on mac you may have to say: "attribute" instead of "in":
//attribute vec4 vCol;

// we are going to be getting an attribute from the main program, named
// "vPos", one for each vertex.
//
// on mac you may have to say: "attribute vec4 vPosition;" instead of "in...":
attribute vec4 vPos;

// our shader outputs one color value per vertex: we will send it to the
// fragment shader, and along the way we want the device to interpolate
// the per-vertex values:
varying vec4 color;

//These are the guys I'm gonna use to calculate color when I do my shading
varying vec4 ambient_color, diffuse_color, specular_color;

//I'm not quite sure what these do but I'm also going to need them
varying vec4 diffuse_product, spec_product;

void main()
{
    gl_Position = projection * view * vPos;
    //color = vCol;
    
    ambient_color = materialAmbient * lightAmbient;
    diffuse_product = lightDiffuse * materialDiffuse;
    spec_product = lightSpecular * materialSpecular;

	vec4 light_dir = normalize(lightPos - vPos);
    float dd1 = dot(light_dir, triNorms);
        
    if(dd1>0.0)
        diffuse_color = diffuse_product * dd1;

    // compute the half vector, for specular reflection:
    vec4 view_vec = normalize(eye - vPos);
   	vec4 half = normalize(light_dir + view_vec);
        
    float dd2 = dot(half, triNorms);

    if(dd2 > 0.0)
        specular_color = spec_product * exp(materialShininess*log(dd2));
        
    color = ambient_color + diffuse_color;
	color =  color + specular_color;
	color[3] = 1.0; // set the alpha to 1    
    
    
    //color = triNorms;
}


