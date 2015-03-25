#version 130
in vec2 fragTexCoord;
precision highp float;

uniform sampler2D packedImages;

uniform float quality_threshold;

float mask(float I1,float I2,float I3)
{
   float texture = (I1+I2+I3)/3.0;
   float quality = pow((I1 - texture),2)+pow((I2 - texture),2)+pow((I3 - texture),2);
   quality = sqrt(quality)/texture;

   if(quality> quality_threshold)
   {
     return quality;
   }
   else
   {
     return 0.0;
   }
}


void main() {
 
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	vec3 packedImagesvalues = texture2D( packedImages, texCoord ).rgb;
	
	float maskValue = mask(packedImagesvalues.r,packedImagesvalues.g,packedImagesvalues.b);
	gl_FragColor = vec4(maskValue,maskValue,maskValue,1.0);
	
 
}
