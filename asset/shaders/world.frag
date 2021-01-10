#version 330

precision highp float;

in vec2 uv;

uniform sampler2D image;
uniform isamplerBuffer spriteIndices;

uniform vec4 color;
uniform uvec2 gridCount;
uniform uvec2 gridCountAtlas;

out vec4 FragColor;

void main()
{
	vec2 uv2 = vec2(uv.x, 1.0 - uv.y);
	// Get tile ID two dimensions
	uvec2 tileIDs = uvec2(uv2 * gridCount);
	// Convert to one dimension
	uint tileID = tileIDs.y * gridCount.x + tileIDs.x;
	// Read in a buffer the corresponding tileID from atlas
	int spriteID = texelFetch(spriteIndices, int(tileID)).r;
	// If no sprite, discard
	if (spriteID < 0)
		discard;
	// Convert this id back to 2D
	uvec2 spriteIDs = uvec2(uint(spriteID) % gridCountAtlas.x, uint(spriteID) / gridCountAtlas.x);
	// Map it as an uv
	vec2 spriteUVOffset = vec2(spriteIDs) / vec2(gridCountAtlas);
	vec2 tileUVOffset = vec2(tileIDs) / vec2(gridCount);
	vec2 tileUV = (uv2 - tileUVOffset) / vec2(gridCountAtlas) * vec2(gridCount);
	vec2 spriteUV = spriteUVOffset + tileUV;
	// Get the color
	FragColor = color * texture(image, vec2(spriteUV.x, 1.0 - spriteUV.y));
	if (FragColor.a == 0.0)
		discard;
}
