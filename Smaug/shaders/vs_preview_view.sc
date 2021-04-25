$input a_position, a_normal
$output v_color0, v_texcoord0, v_normal

#include <bgfx_shader.sh>

uniform vec4 color;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_texcoord0 = vec3(mul(u_model[0], vec4(a_position, 1.0)).xyz);
	v_color0 = color;
	
	v_normal = mul(u_model[0], vec4(a_normal, 0.0) );
}