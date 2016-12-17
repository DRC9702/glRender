#version 120

// the simplest fragment shader: get the color (from the vertex shader -
// actually interpolated from values specified in the vertex shader - and
// just pass them through to the render:
// 
// on the mac, you may need to say "varying vec4 color;" instead of this:
//varying vec4 color;


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
varying float fShading2;


// in GLSL 150, use layouts for this:
// layout (location = 0) in color

// in later GLSL, use "in", and "out" respectively

// for lit:
varying vec4 color;

void main() 
{ 
  // "gl_FragColor" is already defined for us - it's the one thing you have
  // to set in the fragment shader:

	if(fShading2>=0){ //Fragment Shading
		vec4 normalizedNormal = normalize(fNormal);
		//These are the guys I'm gonna use to calculate color when I do my shading
		vec4 ambient_color = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 diffuse_color  = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 specular_color  = vec4(0.0, 0.0, 0.0, 0.0);

		//I'm not quite sure what these do but I'm also going to need them
		vec4 diffuse_product = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 spec_product = vec4(0.0, 0.0, 0.0, 0.0);


		ambient_color = fMaterialAmbient * fLightAmbient;
		diffuse_product = fLightDiffuse * fMaterialDiffuse;
		spec_product = fLightSpecular * fMaterialSpecular;

		vec4 light_dir = normalize(fLightPos - fVPos);

		float cosnl = max(0.0, dot(normalizedNormal, light_dir));

		diffuse_color = diffuse_product * cosnl;

		// compute the half vector, for specular reflection:
		vec4 view_vec = normalize(fEye-fVPos);
		vec4 half = normalize(light_dir + view_vec);


		float cosnh = max(0.0, dot(normalizedNormal, half));
		specular_color = spec_product * pow(cosnh, fMaterialShininess);

		gl_FragColor = ambient_color + diffuse_color;
		gl_FragColor =  gl_FragColor + specular_color;
		gl_FragColor[3] = 1.0; // set the alpha to 1



//		ambient_color = fMaterialAmbient * fLightAmbient;
//		diffuse_product = fLightDiffuse * fMaterialDiffuse;
//		spec_product = fLightSpecular * fMaterialSpecular;
//
//		//Adding in diffuse component
//		//vec4 light_dir = normalize(transformedLightPos - vertexPos);
//		vec4 light_dir = normalize(fLightPos - fVPos);
//
//		//float cosnl = max(0.0, dot(transformedNorm, light_dir));
//		float dd = dot (fLightPos, fNormal);
//
////		diffuse_color = diffuse_product * cosnl;
//		if (dd > 0.0)
//			diffuse_color = fMaterialDiffuse * fLightDiffuse * dd;
//
//		// compute the half vector, for specular reflection:
////		vec4 view_vec = normalize(eye-vertexPos);
//		vec4 view_vec = normalize(fEye-fVPos);
//		vec4 half = normalize(light_dir + view_vec);
//		dd = dot(fNormal,half);
//
//		if (dd > 0.0)
//			specular_color = fMaterialSpecular * fLightSpecular * pow(dd, fMaterialShininess);
//
//
//		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
//		gl_FragColor = ambient_color + diffuse_color + specular_color;
//		gl_FragColor[3] = 1.0; // set the alpha to 1

//		gl_FragColor = color;
//		float cosnh = max(0.0, dot(transformedNorm, half));
//		specular_color = spec_product * pow(cosnh, materialShininess);

		//gl_FragColor = fVPos;
	}
	else{ //Vertex Shading
		gl_FragColor = color;
	}
}

