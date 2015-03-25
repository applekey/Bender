#version 130


in vec2 fragTexCoord;

precision highp float;

#define M_PI 3.14159

uniform sampler2D cfbo1;
uniform sampler2D cfbo2;
uniform sampler2D rfbo1;
uniform sampler2D rfbo2;

uniform float wavelength1;
uniform float wavelength2;
uniform float div;

float mmod(float x, float y)
{
	float n = floor(x/y); 
	return x - n*y;	

}

float Wrap(float I1,float I2,float I3)
{
	I1 = I1*255.0;
	I2 = I2*255.0;
	I3 = I3*255.0;

	return atan((sqrt(3.0)*(I1-I3)),(2.0*I2-I1-I3));
}

float UNWRAP(float p1,float p2, float w1,float w2)
{
	float pitch12 = w1* w2/abs(w1-w2);
	float w12 = mod((p1 - p2), 2.0*M_PI);

	float k = floor ((w12*(pitch12/w1)-p1)/(2.0*M_PI));
	return p1 + k* 2.0*M_PI;
}

vec3 FalseColor(float depth)
{
	return vec3(0+depth/2,depth,255-depth/5);

}


void main() {
/*
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	vec3 cw1 = texture2D( cfbo1, texCoord ).rgb;
	vec3 cw2 = texture2D( cfbo2, texCoord ).rgb;
	vec3 rw1 = texture2D( rfbo1, texCoord ).rgb;
	vec3 rw2 = texture2D( rfbo2, texCoord ).rgb;

	// WRAP THE PHASE
	float pc1 = Wrap(cw1.r,cw1.g,cw1.b);
	float pc2 = Wrap(cw2.r,cw2.g,cw2.b);
	float pr1 = Wrap(rw1.r,rw1.g,rw1.b);
	float pr2 = Wrap(rw2.r,rw2.g,rw2.b);

	//UNWRAP THE PHASE
	float upc = UNWRAP(pc1,pc2,wavelength1,wavelength2);
	float upr = UNWRAP(pr1,pr2,wavelength1,wavelength2);

	//CALCULATE THE DEPTH
	float depth = (upc-upr)/div;
*/
    vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	vec3 cw1 = texture2D( cfbo1, texCoord ).rgb;
    gl_FragColor = vec4(0.5f,1.0f,0.7f,1.0);
}
