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

uniform float fShading;
varying float fShading2;

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


//These are the variables to pass to the fragment shader
varying vec4 fLightPos;
varying vec4 fLightDiffuse;
varying vec4 fLightSpecular;
varying vec4 fLightAmbient;

varying vec4 fMaterialDiffuse;
varying vec4 fMaterialSpecular;
varying vec4 fMaterialAmbient;
varying float fMaterialShininess;

varying vec4 fNormal;
varying vec4 fEye;
varying vec4 fVPos;

void main()
{
	fShading2 = fShading;
	if(fShading>=0){ //Fragment Shading
		fLightPos = view * lightPos;
//		fLightPos = lightPos;
		fLightDiffuse = lightDiffuse;
		fLightSpecular = lightSpecular;
		fLightAmbient = lightAmbient;

		fMaterialDiffuse = materialDiffuse;
		fMaterialSpecular = materialSpecular;
		fMaterialAmbient = materialAmbient;
		fMaterialShininess = materialShininess;

//		fNormal = normalize(view *triNorms);
		fNormal = view*triNorms;
//		fNormal = normalize(triNorms);
		vec4 zeroedEye = vec4(0.0,0.0,0.0,1.0);
		//fEye = eye;
		fEye = zeroedEye;
		fVPos = view * vPos;
//		fVPos = vPos;

	}
	else{ //Vertex Shading
		vec4 vertexPos = view * vPos;
		vec4 transformedLightPos = view * lightPos;
		vec4 transformedNorm = view * triNorms;
		vec4 zeroedEye = vec4(0.0,0.0,0.0,1.0);

			//These are the guys I'm gonna use to calculate color when I do my shading
			vec4 ambient_color = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 diffuse_color  = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 specular_color  = vec4(0.0, 0.0, 0.0, 0.0);

			//I'm not quite sure what these do but I'm also going to need them
			vec4 diffuse_product = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 spec_product = vec4(0.0, 0.0, 0.0, 0.0);

		    ambient_color = materialAmbient * lightAmbient;
		    diffuse_product = lightDiffuse * materialDiffuse;
		    spec_product = lightSpecular * materialSpecular;

			vec4 light_dir = normalize(transformedLightPos - vertexPos);

			float cosnl = max(0.0, dot(transformedNorm, light_dir));

		        diffuse_color = diffuse_product * cosnl;

		    // compute the half vector, for specular reflection:
		    vec4 view_vec = normalize(zeroedEye-vertexPos);

		    vec4 half = normalize(light_dir + view_vec);


		    float cosnh = max(0.0, dot(transformedNorm, half));
		    specular_color = spec_product * pow(cosnh, materialShininess);

		    color = ambient_color + diffuse_color;
			color =  color + specular_color;
			color[3] = 1.0; // set the alpha to 1

		    //color = triNorms;
	}

	gl_Position = projection * view * vPos;
    //color = triNorms;
}


