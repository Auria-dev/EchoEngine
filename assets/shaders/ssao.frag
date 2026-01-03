#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gNoiseTexture;

uniform vec3 samples[64];
int kernelSize = 64;
float bias = 0.025;

uniform vec2 uResolution;
uniform mat4 uProjection;

void main() {
	vec2 noiseScale = uResolution / 4.0;
	vec3 fragPos = texture(gPosition, TexCoords).rgb;
	vec3 normal = normalize(texture(gNormal, TexCoords).rgb);

	vec3 randomVec = normalize(texture(gNoiseTexture, TexCoords * noiseScale).xyz);
	float radius = mix(0.1, 0.5, clamp(fragPos.z / 10.0, 0.0, 1.0));

	vec3 T = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 B = cross(normal, T);
	mat3 TBN = mat3(T, B, normal);

	float occlusion = 0.0; // 0 = not occluded, 1 = fully occluded
	for (int i = 0; i < kernelSize; ++i) {
		vec3 samplePos = TBN * samples[i];
		samplePos = fragPos + samplePos * radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = uProjection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	FragColor = 1.0 - (occlusion / kernelSize);
}