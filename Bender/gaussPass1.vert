#version 130

in vec3 vert;
in vec2 vertTexCoord;

out vec2 fragTexCoordOffset[11];

float step_w = 1.0/640.0;
float step_h = 1.0/480.0;

void main()
{
		vec2 uvCoord = vec2(gl_MultiTexCoord0.x,1.0 - gl_MultiTexCoord0.y);

		fragTexCoordOffset[ 0] = uvCoord.xy + vec2(5.0 * -step_w, 0.0);
		fragTexCoordOffset[ 1] = uvCoord.xy + vec2(4.0 * -step_w, 0.0);
		fragTexCoordOffset[ 2] = uvCoord.xy + vec2(3.0 * -step_w, 0.0);
		fragTexCoordOffset[ 3] = uvCoord.xy + vec2(2.0 * -step_w, 0.0);
		fragTexCoordOffset[ 4] = uvCoord.xy + vec2(1.0 * -step_w, 0.0);
		fragTexCoordOffset[ 5] = uvCoord.xy;
		fragTexCoordOffset[ 6] = uvCoord.xy + vec2(1.0 * step_w, 0.0);
		fragTexCoordOffset[ 7] = uvCoord.xy + vec2(2.0 * step_w, 0.0);
		fragTexCoordOffset[ 8] = uvCoord.xy + vec2(3.0 * step_w, 0.0);
		fragTexCoordOffset[ 9] = uvCoord.xy + vec2(4.0 * step_w, 0.0);
		fragTexCoordOffset[10] = uvCoord.xy + vec2(5.0 * step_w, 0.0);
		
        gl_Position = ftransform();
}
