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
	float lineWidthGrid = 0.5 - lineWidth * 0.5;
	
	
	float drawLine = 0.0;//  (|| gridCoords.y > lineWidth || gridCoords.z > lineWidth  ) 
				         //+ (|| midLineCoords.y > lineWidth || midLineCoords.z > lineWidth ) * 0.50;
	
	// White lines
	drawLine += gridDirMask.x * ( (gridCoords.x > lineWidthGrid) );
	drawLine += gridDirMask.y * ( (gridCoords.y > lineWidthGrid) );
	drawLine += gridDirMask.z * ( (gridCoords.z > lineWidthGrid) );
	drawLine = clamp(drawLine, 0.0, 1.0);

	// Gray lines
	float grayLine = 0.0;
	
	grayLine += gridDirMask.x * (midLineCoords.x > lineWidthGrid);
	grayLine += gridDirMask.y * (midLineCoords.y > lineWidthGrid);
	grayLine += gridDirMask.z * (midLineCoords.z > lineWidthGrid);
	drawLine += clamp(grayLine, 0.0, (1.0 - drawLine) * 0.5);
	
	// Axis lines
	vec3 axisLines = abs(v_texcoord0 * gridScale);
	float red = 0.0;
	
	red += gridDirMask.x * (axisLines.x < lineWidth * 0.5);
	red += gridDirMask.y * (axisLines.y < lineWidth * 0.5);
	red += gridDirMask.z * (axisLines.z < lineWidth * 0.5);
	float redDisable = ( red == 0.0 );
	float redDampen =  (red > 0) * 0.5;

	
	float lineWidthBigGrid = 0.5 - lineWidth * 0.125 * 0.5;
	// Big blue
	vec3 bigGridCoords = mod(v_texcoord0 * gridScale * 0.125, 1);
	bigGridCoords = abs(bigGridCoords - 0.5);
	float blue = 0.0;
	blue += gridDirMask.x * (bigGridCoords.x > lineWidthBigGrid);
	blue += gridDirMask.y * (bigGridCoords.y > lineWidthBigGrid);
	blue += gridDirMask.z * (bigGridCoords.z > lineWidthBigGrid);
	blue =  clamp(blue, 0.0, 1.0);


	gl_FragColor = vec4(drawLine * ( 1.0 - blue ) * redDisable + redDampen, drawLine * redDisable * ( 1.0 - blue ), (drawLine * (1.0 - blue) + blue * 0.75) * redDisable , drawLine + blue);

	
}