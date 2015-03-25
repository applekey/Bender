#version 130
in vec2 fragTexCoord;
precision highp float;

#define M_PI 3.14159

uniform sampler2D packedCaptures_firstWavelength;
uniform sampler2D packedCaptures_secondWavelength;
uniform sampler2D referenceCaptures_firstWavelength;
uniform sampler2D referenceCaptures_secondWavelength;


float Wrap(float I1,float I2,float I3)
{
	I1 = I1*255.0;
	I2 = I2*255.0;
	I3 = I3*255.0;

	return atan((sqrt(3.0)*(I1-I3)),(2.0*I2-I1-I3));
}
/*
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
*/
void main() {
 
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
		  
	vec3 image_c1 = texture2D( packedCaptures_firstWavelength, texCoord ).rgb;
	vec3 image_c2 = texture2D( packedCaptures_secondWavelength, texCoord ).rgb;
	   
	vec3 image_r1 = texture2D( referenceCaptures_firstWavelength, texCoord ).rgb;
	vec3 image_r2 = texture2D( referenceCaptures_secondWavelength, texCoord ).rgb;
	   

	float pc1 = Wrap(image_c1.r,image_c1.g,image_c1.b);
	float pc2 = Wrap(image_c2.r,image_c2.g,image_c2.b);
	float pr1 = Wrap(image_r1.r,image_r1.g,image_r1.b);
	float pr2 = Wrap(image_r2.r,image_r2.g,image_r2.b);
	  	
	gl_FragData [0] = vec4(pc1,pc2,pr1,pr2);
	gl_FragData [1] = vec4(pc1,pc2,pr1,pr2);
}
