$input v_color0, v_texcoord0, v_normal

#include <bgfx_shader.sh>

//SAMPLER2D(s_textureUniform, 0);

void main()
{
	//v_texcoord0.y = fmod(v_texcoord0.y, 15);
	//float floor = 0;//v_texcoord0.y < 0.1;
	//float wall = ((v_texcoord0.y / 15.0 - 0.5) * 2.0 + 1.0) / 2.0;
	//gl_FragColor = vec4(floor * 0.85, wall + floor * 0.8, (1.0 - wall * wall) * (!floor) * 0.40 +  floor * 0.9, 1.0);
	//float wall = (cos(v_texcoord0.y * 3.14 + sin(v_texcoord0.x + v_texcoord0.z)) + 2.75) / 3.75;
	//gl_FragColor = vec4(wall, wall, wall, 1.0);
	//gl_FragColor = vec4(wall + floor * 0.8, wall + floor * 0.8, wall + floor * 0.8, 1.0);
	//gl_FragColor *= v_color0;
	//v_texcoord0.y = fmod(v_texcoord0.y, 8);
	float alpha = 1.0;//(-cos(v_texcoord0.y * 3.14 * 0.125) + 1.0) * 0.5 * 0.1 + 0.9;
	vec3 color = vec3(alpha, alpha, alpha);
	color *= vec3(v_color0.x, v_color0.y, v_color0.z);
	float ndotl = dot(normalize(v_normal), vec3(0.427436, 0.7268582, -0.6971823));
	gl_FragColor = vec4( color * 0.3 +  0.7 * color * pow(ndotl,2), 1.0);

}