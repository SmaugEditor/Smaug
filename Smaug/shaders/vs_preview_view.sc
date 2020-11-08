$input a_position
$output v_color0

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_color0 = vec4(a_position.x, a_position.y, a_position.z, 1.0);
}