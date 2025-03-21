#pragma once
static inline uint ComputeVoxelColor(const int x, const int y, const int z, const int3 worldSize) {
	// Calculate color components based on voxel position
	//Source: OneBogdan01 on github
	float r = (float)x / worldSize.x * 255;
	float g = (float)y / worldSize.y * 255;
	float b = (float)z / worldSize.z * 255;

	// Clamp the color components to the range [0, 255]
	r = clamp(r, 0.0f, 255.0f);
	g = clamp(g, 0.0f, 255.0f);
	b = clamp(b, 0.0f, 255.0f);

	// Combine color components to form the final color value
	return ((uint)r << 16) | ((uint)g << 8) | (uint)b;
}

static inline uint RandomColor() {
	// Generate a random color
	return 
		(rand() & 0xFF) << 16 | // Red
		(rand() & 0xFF) << 8 | // Green
		(rand() & 0xFF);       // Blue
}