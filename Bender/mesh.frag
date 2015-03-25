#version 130
precision highp float;

uniform sampler2D meshDepthImage; 
uniform sampler2D maskImage; 

uniform int mode;
uniform float depthCull;

float step_w = 1.0/640.0;
float step_h = 1.0/480.0;
 
void main(void)
{
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	vec3 color = texture2D( meshDepthImage, texCoord ).rgb;
	
	if(mode != 2)
	{
		
		gl_FragColor = vec4(color,1.0);
		
	}
	else
	{
		float depth = color.x;

		float maskvalue = texture2D( maskImage, texCoord ).r;

		if(maskvalue == 0.0||depth > depthCull/2.0  || depth <-2.0)
		{
			depth = 0.0;
		}

		int filtSize =11;
        float total = 0.0;
        for(int i =0;i<filtSize;i++)
        {
                for(int j =0;j<filtSize;j++)
                {
                        vec2 offset = vec2(step_w*(i- filtSize/2),step_h*(j- filtSize/2));
                        total += texture(meshDepthImage, texCoord.xy+offset).x;
                }
        }
		
		float tsize = float(filtSize);
        total = total/pow(tsize,2.0); 
                
        if(abs(depth/total)>2.0)
        {
                depth = total;
        }

	
		gl_FragColor = vec4(depth,depth,depth,depth);
	}

	

}
