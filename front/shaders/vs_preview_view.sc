$input a_position, a_normal, a_texcoord1
$output v_color0, v_texcoord0, v_texcoord1, v_normal

#include <bgfx_shader.sh>

uniform vec4 color;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_color0 = color;

	v_normal = mul(u_model[0], vec4(a_normal, 0.0) ).xyz;

	vec3 tw = abs(normalize(v_normal));

	vec3 pos = vec3(mul(u_model[0], vec4(a_position, 1.0)).xyz);
	if(tw.x >= tw.z && tw.x >= tw.y)
	{
		v_texcoord0 = vec3(pos.zy, 0.0);
		v_texcoord0.x *= v_normal.x > 0.0 ? 1.0 : -1.0;
	}
	else if(tw.z >= tw.y && tw.z >= tw.x)
	{
		v_texcoord0 = vec3(pos.xy, 0.0);
		v_texcoord0.x *= v_normal.z < 0.0 ? 1.0 : -1.0;
	}
	else
	{
		v_texcoord0 = vec3(pos.xz, 0.0);
		v_texcoord0.x *= v_normal.y < 0.0 ? -1.0 : 1.0;
	}
	v_texcoord0.y *= -1.0;

	v_texcoord1 = a_texcoord1;
}