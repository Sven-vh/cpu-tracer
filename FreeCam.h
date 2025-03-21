#pragma once

namespace Tmpl8{
	class VoxelWorld;
}

struct ExplosionLocation {
	int3 position;
	float duration;
	int worldIndex;
	VoxelWorld* world;
	float size;
	ExplosionLocation(int3 position, int worldIndex, VoxelWorld* newOwner) : position(position), duration(0.0f), worldIndex(worldIndex), size(0.0f), world(newOwner) {}
};

class FreeCam : public GameScene {
public:
	// Inherited via GameScene
	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;

	Renderer* renderer;

private:
	VoxelWorld* earth;
	VoxelWorld* wings;
	VoxelWorld* sun;
	VoxelWorld* bulletPrefab;

	float totalTime;
	float3 wingsOffset;
	float bulletSpeed;

	float sunSpeed;
	float3 sunOffset;

	float explosionMaxSize = 20.0f;
	float explosionProbabilty = 0.15f;

	float wave1 = 0.1f;
	float wave2 = 0.317f;
	float wave3 = 0.7f;
	int explosionMaterialIndex = 0;

	bool canShoot = true;
	bool rotateObjects = true;

	std::vector<ExplosionLocation> explosions;

	void HanldeInput();
	void HandleExplosions(const float deltaTime);
};

