$input a_position
$output v_texcoord0

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_texcoord0 = vec3(mul(u_model[0], vec4(a_position, 1.0)).xyz);
}