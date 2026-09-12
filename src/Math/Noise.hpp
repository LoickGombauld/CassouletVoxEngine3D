#pragma once

namespace Voxel
{

	float noise(
		float x,
		float z
	);

	float fractalNoise(
		float x,
		float z,
		int octaves,
		float persistence,
		float lacunarity
	);
}