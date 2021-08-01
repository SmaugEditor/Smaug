$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 gridScale;
uniform vec4 gridDirMask;


void main()
{
	vec3 gridCoords = mod(v_texcoord0 * gridScale.xyz, 1.0);
	vec3 midLineCoords = mod(gridCoords * 2.0, 1.0);
	gridCoords = abs(gridCoords - 0.5);
	midLineCoords = abs(midLineCoords - 0.5);
	
	float lineWidth = 0.25 * gridScale.x;
	float lineWidthGrid = 0.5 - lineWidth * 0.5;// / gridScale.x;
	
	
	float drawLine = 0.0;//  (|| gridCoords.y > lineWidth || gridCoords.z > lineWidth  ) 
				         //+ (|| midLineCoords.y > lineWidth || midLineCoords.z > lineWidth ) * 0.50;
	
	// White lines
	drawLine += gridDirMask.x * ( float(gridCoords.x > lineWidthGrid) );
	drawLine += gridDirMask.y * ( float(gridCoords.y > lineWidthGrid) );
	drawLine += gridDirMask.z * ( float(gridCoords.z > lineWidthGrid) );
	drawLine = clamp(drawLine, 0.0, 1.0);

	// Gray lines
	float grayLine = 0.0;
	
	grayLine += gridDirMask.x * float(midLineCoords.x > lineWidthGrid);
	grayLine += gridDirMask.y * float(midLineCoords.y > lineWidthGrid);
	grayLine += gridDirMask.z * float(midLineCoords.z > lineWidthGrid);
	drawLine += clamp(grayLine, 0.0, (1.0 - drawLine) * 0.5);
	
	// Axis lines
	vec3 axisLines = abs(v_texcoord0 * gridScale.xyz);
	float red = 0.0;
	
	red += gridDirMask.x * (float(axisLines.x < lineWidth) * 0.5);
	red += gridDirMask.y * (float(axisLines.y < lineWidth) * 0.5);
	red += gridDirMask.z * (float(axisLines.z < lineWidth) * 0.5);
	float redDisable = float( red == 0.0 );
	float redDampen =  float(red > 0.0) * 0.5;

	
	float lineWidthBigGrid = 0.5 - lineWidth * 0.125 * 0.5;
	// Big blue
	vec3 bigGridCoords = mod(v_texcoord0 * gridScale.xyz * 0.125, 1.0);
	bigGridCoords = abs(bigGridCoords - 0.5);
	float blue = 0.0;
	blue += gridDirMask.x * float(bigGridCoords.x > lineWidthBigGrid);
	blue += gridDirMask.y * float(bigGridCoords.y > lineWidthBigGrid);
	blue += gridDirMask.z * float(bigGridCoords.z > lineWidthBigGrid);
	blue =  clamp(blue, 0.0, 1.0);

	vec3 col = vec3(drawLine * ( 1.0 - blue ) * redDisable + redDampen, drawLine * redDisable * ( 1.0 - blue ), (drawLine * (1.0 - blue) + blue * 0.75) * redDisable);
	gl_FragColor = vec4(col, float(col.x > 0 || col.y > 0 || col.z > 0));

	
}