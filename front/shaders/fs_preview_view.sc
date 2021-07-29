$input v_color0, v_texcoord0, v_texcoord1, v_normal

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

void main()
{
	vec3 color = vec3(1.0, 1.0, 1.0);
	//color *= vec3(v_color0.x, v_color0.y, v_color0.z);
	float ndotl = dot(normalize(v_normal), vec3(0.427436, 0.7268582, -0.6971823));
	gl_FragColor = vec4( color * 0.3 +  0.7 * color * pow(ndotl,2), v_color0.w);
	gl_FragColor *= texture2D(u_texture, v_texcoord0.xy * v_texcoord1.zw + v_texcoord1.xy);
}