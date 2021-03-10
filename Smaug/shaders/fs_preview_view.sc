$input v_texcoord0

#include <bgfx_shader.sh>

//SAMPLER2D(s_textureUniform, 0);

void main()
{
	v_texcoord0.y = fmod(v_texcoord0.y, 15);
	float floor = v_texcoord0.y < 0.1;
	float wall = v_texcoord0.y / 15.0;
	gl_FragColor = vec4(floor * 0.85, wall + floor * 0.8, (1.0 - wall * wall) * (!floor) * 0.40 +  floor * 0.9, 1.0);
	
	//gl_FragColor = vec4(texture2D(s_textureUniform, vec2(v_texcoord0.x, 1.0 - v_texcoord0.y) ).xyz * ndotl, 1.0);
}