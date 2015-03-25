#version 130
in vec2 fragTexCoord;
precision highp float;

#define M_PI 3.14159

uniform sampler2D filteredPhasesCos;
uniform sampler2D filteredPhasesSin;

uniform float wavelength1 = 54.1;
uniform float wavelength2 = 60.0;
uniform float div = 8.0;
uniform float quality_threshold = 0.3;

uniform float constraint = 0.5;

uniform int displayMode;

float mmod(float x, float y)
{
	float n = floor(x/y); 
	return x - n*y;	
}

float UNWRAP(float p1,float p2, float w1,float w2)
{
	float pitch12 = w1* w2/abs(w1-w2);
	float w12 = mod((p1 - p2), 2.0*M_PI);

	float k = floor ((w12*(pitch12/w1)-p1)/(2.0*M_PI));
	return p1 + k* 2.0*M_PI;
}

int mask(float I1,float I2,float I3)
{
   float texture = (I1+I2+I3)/3.0;
   float quality = pow((I1 - texture),2)+pow((I2 - texture),2)+pow((I3 - texture),2);
   quality = sqrt(quality)/texture;

   if(quality> quality_threshold)
   {
     return 0;
   }
   else
   {
     return -1;
   }
}

vec3 FalseColor(float depth)
{
	return vec3(0.0+depth/2,depth,255.0-depth/5);
}

vec4 wrap(vec4 sin, vec4 cos)
{
   return atan(sin,cos);
}

void main() {
 
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	

	vec4 sinImage = texture2D( filteredPhasesSin, texCoord ).rgba;
	vec4 cosImage = texture2D( filteredPhasesCos, texCoord ).rgba;
	vec4 filteredImages = wrap(sinImage,cosImage);
	   
	float upc = UNWRAP(filteredImages.r,filteredImages.g,wavelength1,wavelength2);
	float upr = UNWRAP(filteredImages.b,filteredImages.a,wavelength1,wavelength2);
	
	float depth = -(upc-upr)/div;
	
	float coordX = gl_TexCoord[0].x;



	vec3 falseColor; 
	if(displayMode == 0)   // capture mode
	{
	  upc/=(div*10);
	  falseColor = FalseColor(upc);
	}
	else if(displayMode ==1) // reference mode
	{
	  upr/=(div*10);
	  falseColor = FalseColor(upr);
	}
	else // depth mode
	{
	  falseColor= vec3(depth,depth,depth);
	  //falseColor = FalseColor(depth);
	}
	
	
	gl_FragColor = vec4(falseColor,1.0);
	
 
}
