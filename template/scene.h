#pragma once


namespace Tmpl8 {

#ifdef TWOLEVEL


	//material list
	inline std::vector <Material> MaterialList = {
		Material(0, 1.0f, 0.0f, 0.0f, 1.0f) // Default material
	};

	struct Brick {
		unsigned int* grid;
		std::atomic<size_t> voxelCount = 0;
		int3 gridPosition;

		Brick(const int3 gridPosition) {
			voxelCount = 0;
			this->gridPosition = gridPosition;
			grid = (uint*)MALLOC64(BRICKSIZE3 * sizeof(uint));
			memset(grid, 0, BRICKSIZE3 * sizeof(uint));
		}

		void Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex = 0);
		void Clear(const uint v);
		void FindNearest(Ray& ray, const float& brickEntryT) const;
		bool FindNearestEmpty(Ray& ray, const float& brickEntryT) const;
		bool IsOccluded(const Ray& ray, const float& brickEntryT) const;
		bool IsEmpty() const;
	private:
		bool Setup3DDDA(const Ray& ray, DDAState& state) const;

		static inline int GetVoxelIndex(const int x, const int y, const int z) {
#if MORTON
			return morton_encode(x, y, z);
#else
			return x + y * BRICKSIZE + z * BRICKSIZE2;
#endif
		}
	};
#endif // TWOLEVEL


	class VoxelWorld {
	public:
		VoxelWorld(const int3 newGridDimensions = int3(WORLDSIZE / BRICKSIZE));
		void GenerateGrid();
		void FindNearest(Ray& ray) const;
		void FindNearestEmpty(Ray& ray) const;
		void Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex = 0);
		void Clear(const uint v);

		bool IsOccluded(const Ray& ray) const;
		bool DrawImGui(const int index);

		void Resize(const int3 newGridSize);
		void UpdateTransform();
		void UpdateTransformCentered();
		void UpdateTranformRotateLocal();
		void RandomizeTransform();

		void SetActive(const bool active);
		bool IsActive() const;

		float3 GetSize() const;

		std::vector<float3> GetCorners() const;
		int3 gridDimensions;
		float3 position;
		float NoiseFrequency = 5.0f;
		float3 rotation;
		float NoiseAmplitude = 0.5f;
		float3 scale;
		int NoiseColor = 0;
		Cube cube;
		Brick** bricks;
		mat4 transform;
		mat4 invTransform;

	private:
		bool Setup3DDDA(const Ray& ray, DDAState& state) const;
		void ResizeCube(const int3 newGridSize);

		static inline int GetBrickIndex(const int x, const int y, const int z, const int3 gridDimensions) {
			return x + y * gridDimensions.x + z * gridDimensions.x * gridDimensions.y;
		}

		static inline int GetGridSize(const int3 gridDimensions) {
			return gridDimensions.x * gridDimensions.y * gridDimensions.z;
		}


		int3 newGridDimensions;
	};


	class Scene {
	public:
		Scene();
		void GenerateGrid();
		void FindNearest(Ray& ray) const;
		void FindNearestEmpty(Ray& ray) const;
		bool IsOccluded(Ray& ray) const;
		bool IsOccluded(Ray& ray, const int worldIndex) const;
		bool DrawImGui();
		void Clear(const uint v);
		void Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex = 0, const int worldIndex = 0);

		void ConstructBVH();
		void CLearWorlds();

		std::vector<VoxelWorld*> worlds;
#ifndef _DEBUG
		float2 dummy;
#endif
		//struct BVHNode {
		//	Cube bounds; // 24 bytes
		//	bool isLeaf; // 1 byte
		//	BVHNode* left, * right; // 8 bytes or 16 bytes
		//	int first, count; // 8 bytes

		//	void Subdivide() {
		//		if(count < 3) return;
		//		left = 
		//		isLeaf = false;
		//	}
		//};
	private:
		int3 newWorldSize = int3(16, 16, 16);

		//BVHNode* root;
	};

}