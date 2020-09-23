$input a_position
$output v_color0

#include <bgfx_shader.sh>

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_color0 = vec4(0.0, a_position.y / 10.0, 0.0, 1.0);
}