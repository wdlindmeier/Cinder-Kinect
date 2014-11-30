#version 150

in float vDepth;
out vec4 oColor;

void main()
{
//	if( vDepth < 0.1 ) discard;

	oColor.rgb = vec3( vDepth );
	oColor.a = 1.0;
}





