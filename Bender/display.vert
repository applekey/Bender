#version 330 

in vec3 inPosition;
in vec2 vertexUV;

uniform mat4 MVP;

in vec2 texCoord;
out vec2 UV;
out vec3 vertexPos;

flat out int mask;

uniform sampler2D meshDepthImage; 

uniform int mode;



void main()
{
	if(mode != 2)
	{
		UV = inPosition.xy;
	    gl_Position = MVP * vec4(inPosition.xyz,1);
	  	vertexPos = gl_Position.xyz;
	}
	else
	{
		vec2 sampleCoord = inPosition.xy;
		sampleCoord.y = 1.0 - sampleCoord.y;
		float depth = texture(meshDepthImage, sampleCoord).x;
		vec3 newPosition = inPosition;
		newPosition.z = depth;

		if(depth ==0.0)
		{
		    newPosition.z = 0.0;
			mask = 1;
		}
			
		else
		{
			mask = 0;
		}
			
		gl_Position =  MVP * vec4(newPosition,1);
		UV = inPosition.xy;
		vertexPos = gl_Position.xyz;
	}

}
