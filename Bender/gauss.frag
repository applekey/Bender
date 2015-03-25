#version 130
precision highp float;

precision highp float;

uniform sampler2D wrapImage;
uniform float kernel[11];
uniform int mode;

in vec2 fragTexCoordOffset[11];

 
void main(void)
{
    vec4 filteredImage = vec4(0.0);
	if(mode == 0)
	{
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 0]))*0.0031;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 1]))*0.0044;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 2]))*0.0058;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 3]))* 0.0069;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 4]))* 0.0076;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 5]))* 0.0087;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 6]))* 0.0076;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 7]))*0.0069;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 8]))* 0.0058;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[ 9]))* 0.0044;
		filteredImage += sin(texture(wrapImage, fragTexCoordOffset[10]))*0.0031;
	}
	else
	{
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 0]))*0.0031;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 1]))*0.0044;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 2]))*0.0058;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 3]))* 0.0069;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 4]))* 0.0076;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 5]))* 0.0087;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 6]))* 0.0076;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 7]))*0.0069;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 8]))* 0.0058;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[ 9]))* 0.0044;
		filteredImage += (texture(wrapImage, fragTexCoordOffset[10]))*0.0031;
	}
   

	gl_FragColor = filteredImage;

}
