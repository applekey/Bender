#version 130 

in vec3 vert;
in vec2 vertTexCoord;

out vec2 fragTexCoord;

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
