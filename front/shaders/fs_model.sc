$input v_color0, v_texcoord0, v_normal

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

void main()
{
	float ndotl = dot(normalize(v_normal), vec3(0, 0, 1.0));
	gl_FragColor = vec4(texture2D(u_texture, vec2(v_texcoord0.x, 1.0 - v_texcoord0.y) ).xyz * ndotl, 1.0);

	
}