#version 130

uniform float width;
uniform float height;

in vec3 vert;
in vec2 vertTexCoord;

out vec2 fragTexCoordOffset[11];

float step_w = 1.0/640.0;
float step_h = 1.0/480.0;

void main()
{
		vec2 uvCoord = vec2(gl_MultiTexCoord0.x,1.0 - gl_MultiTexCoord0.y);

		fragTexCoordOffset[ 0] = uvCoord.xy + vec2(0.0, 5.0 * -step_h);
		fragTexCoordOffset[ 1] = uvCoord.xy + vec2(0.0, 4.0 * -step_h);
		fragTexCoordOffset[ 2] = uvCoord.xy + vec2(0.0, 3.0 * -step_h);
		fragTexCoordOffset[ 3] = uvCoord.xy + vec2(0.0, 2.0 * -step_h);
		fragTexCoordOffset[ 4] = uvCoord.xy + vec2(0.0, 1.0 * -step_h);
		fragTexCoordOffset[ 5] = uvCoord.xy;
		fragTexCoordOffset[ 6] = uvCoord.xy + vec2(0.0, 1.0 * step_h);
		fragTexCoordOffset[ 7] = uvCoord.xy + vec2(0.0, 2.0 * step_h);
		fragTexCoordOffset[ 8] = uvCoord.xy + vec2(0.0, 3.0 * step_h);
		fragTexCoordOffset[ 9] = uvCoord.xy + vec2(0.0, 4.0 * step_h);
		fragTexCoordOffset[10] = uvCoord.xy + vec2(0.0, 5.0 * step_h);
		
        gl_Position = ftransform();

}
