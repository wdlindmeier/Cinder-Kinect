#version 150

uniform sampler2D uDepthTex;
uniform mat4 ciModelViewProjection;
uniform float uPointSize;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec4 vVertex;
out float vDepth;

void main()
{
	vDepth				= texture( uDepthTex, ciTexCoord0 ).b;
    vVertex				= vec4( ciPosition );
	vVertex.z			+= vDepth * 1000.0;
    gl_PointSize        = uPointSize;
	gl_Position			= ciModelViewProjection * vVertex;
}
