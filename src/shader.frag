#version 330 core

in vec4 fragColor;
in vec2 UV;

out vec4 color;

uniform sampler2D myTextureSampler;
uniform bool texture_state;
uniform bool color_state;
uniform float movement;
uniform float alpha;

float plot (vec2 st, float pct){
  return  smoothstep(pct - 0.01, pct, st.y) - smoothstep(pct, pct + 0.01, st.y);
}

void main() {

	vec3 colorA = vec3(sin(0.149 * movement), sin(0.141 * movement), sin(0.712 * movement));
	vec3 colorB = vec3(sin(1.000 * movement), sin(0.833 * movement), sin(0.224 * movement));
	vec3 colorAB = vec3(0.0);

	vec3 pct = vec3(fragColor.x);
	colorAB = mix(colorA, colorB, pct);

	mediump vec4 texture = texture(myTextureSampler, UV).rgba;
	mediump vec4 color_type;
	if (!color_state)
		color_type = vec4(mod(fragColor.x * movement, 1), mod(fragColor.y * movement, 1), mod(fragColor.z * movement, 1), 1);
	else
		color_type = vec4(colorAB, 1.0);
	color_type.rgb *= 1.0 - alpha;
	texture.rgb *= alpha;
	color = vec4(color_type.rgb + texture.rgb, 1.0);
};
