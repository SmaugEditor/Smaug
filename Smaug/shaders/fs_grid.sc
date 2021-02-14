$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 s_gridScale;

void main()
{
	vec2 gridCoords = mod(v_texcoord0.xy * s_gridScale.xy, 1);
	vec2 midLineCoords = mod(gridCoords * 2, 1);
	gridCoords = abs(gridCoords - 0.5);
	midLineCoords = abs(midLineCoords - 0.5);
	
	float lineWidth = 0.05;
	lineWidth = 0.5 - lineWidth / 2.0;
	
	float drawLine = (gridCoords.x > lineWidth || gridCoords.y > lineWidth ) + (midLineCoords.x > lineWidth || midLineCoords.y > lineWidth ) * 0.50;
	gl_FragColor = vec4(drawLine, drawLine, drawLine, drawLine);

	
}