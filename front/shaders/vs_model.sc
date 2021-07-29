$input a_position, a_texcoord0, a_normal
$output v_color0, v_texcoord0, v_normal

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_color0 = vec4(a_position.x, a_position.y, a_position.z, 1.0);
	v_texcoord0 = a_texcoord0;

	
	//vec3 normal = a_normal.xyz*2.0 - 1.0;
	v_normal = mul(u_modelView, vec4(a_normal, 0.0) ).xyz;
}