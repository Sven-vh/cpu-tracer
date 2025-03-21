#pragma once
#define LAMP_INDEX 3
#define ANTI_LAMP_INDEX 4

//forward declaration
namespace Tmpl8 {
	class Scene;
}

struct Voxel {
	int3 position;
	uint color;
	int materialIndex;
	int colorIndex;
};

struct Lamp {
	Voxel voxel;
	int requiredRays;
	bool isFound = false;
};

class PuzzleLevel {
public:
	PuzzleLevel();
	PuzzleLevel(const std::string levelPath);
	~PuzzleLevel();

	void DrawLevel(Scene scene, const int3 position);
	void LoadLevel(const std::string levelPath);

	std::vector<Lamp> Lamps;
	std::vector<Voxel> Voxels;
	int3 Size;
};

