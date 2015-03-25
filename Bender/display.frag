#version 330 
precision highp float;

out vec3 color;
in vec2 UV;
in vec3 vertexPos;
flat in int mask;

uniform sampler2D normalImage; 

uniform int mode;

// set the stuff for phong shading

void main() 
{
    vec2 flipUv = vec2(UV.x, 1.0 - UV.y);
	//vec2 flipUv = vec2(UV.x, UV.y);

    if(mode != 2) // just act as a pass through  
	{
	   // flip the god dam texture
	  
	   color = texture2D( normalImage, flipUv ).rgb;
	}
	else
	{
		if(mask == 1)
		{
			color = vec3(0.0,0.0,0.0);
		}
		else
		{
			// light material properties
			vec4 lightPosition = vec4(0.0f, 10.0f, 10.0f, 100.0f);
			vec4 lightAmbient = vec4(0.3f, 0.3f, 0.3f, 0.3f);
			vec4 lightDifuse = vec4(0.5f, 0.5f, 0.5f, 1.0f);
			vec4 lightSpec = vec4(0.7f, 0.7f, 0.7f, 0.7f);

			// set up paramters
	
			vec3 normal = texture2D( normalImage, flipUv ).rgb;
	
			vec3 L = normalize(lightPosition.xyz - vertexPos);   
			vec3 E = normalize(-vertexPos); // we are in Eye Coordinates, so EyePos is (0,0,0)  
			vec3 R = normalize(-reflect(L,normal));  
 
			//calculate Ambient Term:  
			vec4 Iamb = lightAmbient;    

			//calculate Diffuse Term:  
			vec4 Idiff = lightDifuse * max(dot(normal,L), 0.0);
			Idiff = clamp(Idiff, 0.0, 1.0);     
   
		   // material properties
		   float shiny =  3.0f;

			// calculate Specular Term:
			vec4 Ispec = lightSpec 
					* pow(max(dot(R,E),0.0),0.3*shiny);
			Ispec = clamp(Ispec, 0.0, 1.0); 
			// write Total Color:  

			//color = texture2D( normalImage, UV ).rgb;
			color = vec3(0.5,0.5,0.5) + (Iamb.xyz/2.0 + Idiff.xyz + Ispec.xyz)/3.0;
		}
	}	
}