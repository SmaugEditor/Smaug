$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 gridScale;
uniform vec4 gridDirMask;


void main()
{
	vec3 gridCoords = mod(v_texcoord0 * gridScale, 1);
	vec3 midLineCoords = mod(gridCoords * 2, 1);
	gridCoords = abs(gridCoords - 0.5);
	midLineCoords = abs(midLineCoords - 0.5);
	
	float lineWidth = 0.05;
	lineWidth = 0.5 - lineWidth / 2.0;
	
	
	float drawLine = 0.0;//  (|| gridCoords.y > lineWidth || gridCoords.z > lineWidth  ) 
				         //+ (|| midLineCoords.y > lineWidth || midLineCoords.z > lineWidth ) * 0.50;
	
	// White lines
	drawLine += gridDirMask.x * ( (gridCoords.x > lineWidth) );
	drawLine += gridDirMask.y * ( (gridCoords.y > lineWidth) );
	drawLine += gridDirMask.z * ( (gridCoords.z > lineWidth) );
	drawLine = clamp(drawLine, 0.0, 1.0);

	// Gray lines
	float grayLine = 0.0;
	grayLine += gridDirMask.x * (midLineCoords.x > lineWidth);
	grayLine += gridDirMask.y * (midLineCoords.y > lineWidth);
	grayLine += gridDirMask.z * (midLineCoords.z > lineWidth);
	drawLine += clamp(grayLine, 0.0, (1.0 - drawLine) * 0.5);


	// 

	gl_FragColor = vec4(drawLine, drawLine, drawLine, drawLine);

	
}