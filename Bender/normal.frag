#version 130
precision highp float;

uniform sampler2D fboImage; // this is actually the depth

uniform int displayMode = 2;

float stepW = 1.0/640.0;
float stepH = 1.0/480.0;

void main() {
	vec2 texCoord = vec2( gl_TexCoord[0].x , gl_TexCoord[0].y );
	if(displayMode != 2)
	{ 
	    vec3 color = texture(fboImage, texCoord).rgb;
	    gl_FragColor = vec4(color,1.0);
	}
	else
	{

	      /*

		  a  b  c
		  d  e  f
		  g  h  i

		  */
		
		  vec3 e;
		  e.xy = vec2(0.0,0.0);
		  e.z = texture(fboImage, texCoord).r;

		  vec3 f;
		  f.xy = vec2(stepW, 0.0);
		  f.z = texture(fboImage, texCoord + vec2(stepW, 0.0)).r;
		  
		  vec3 i;
		  i.xy = vec2(stepW, stepH);
		  i.z= texture(fboImage, texCoord + vec2(stepW, stepH)).r;  //i

		  vec3 h; 
		  h.xy = vec2(0.0, stepH);
		  h.z= texture(fboImage, texCoord + vec2(0.0, stepH)).r;  //h

		  vec3 g;
		  g.xy = vec2(-stepW, stepH);
		  g.z = texture(fboImage, texCoord + vec2(-stepW, stepH)).r; //g

		  vec3 d;
		  d.xy = vec2(-stepW, 0.0);
		  d.z = texture(fboImage, texCoord + vec2(-stepW, 0.0)).r; //d
		  
		  vec3 a;
		  a.xy = vec2(-stepW, -stepH);
		  a.z = texture(fboImage, texCoord + vec2(-stepW, -stepH)).r; //a

		  vec3 b;
		  b.xy =  vec2(0.0, -stepH);
		  b.z = texture(fboImage, texCoord + vec2(0.0, -stepH)).r; //b
		  
		  vec3 c;
		  c.xy = vec2(stepW, -stepH);
		  c.z = texture(fboImage, texCoord + vec2(stepW, -stepH)).r; //c
    	 
		  vec3 fe = (f- e);
		  vec3 ie = (i -e);
		  vec3 he = (h -e);
		  vec3 ge = (g -e);
		  vec3 de = (d -e);
		  vec3 ae = (a -e);
		  vec3 be = (b -e);
		  vec3 ce = (c -e);

		  vec3 normal = vec3(0.0);
		  normal += cross(fe,ie);
		  normal += cross(ie,he);
		  normal += cross(he,ge);
		  normal += cross(ge,de);
		  normal += cross(de,ae);
		  normal += cross(ae,be);
		  normal += cross(be,ce);
		  
		  // normalize the normal
		  normal = normalize(normal);
		  
  
		  vec4 normalMap = vec4(normal,0.0);


		 
		  gl_FragColor =normalMap;
	}

}