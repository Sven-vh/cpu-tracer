#include "precomp.h"
Scene::Scene() {
	worlds = std::vector<VoxelWorld*>(1);
	worlds[0] = new VoxelWorld();
#ifdef TWOLEVEL

	//bricks = (Brick*)MALLOC64(GRIDSIZE3 * sizeof(Brick*));
	//memset(bricks, 0, GRIDSIZE3 * sizeof(Brick));
	// the voxel world sits in a 1x1x1 cube
#else
	// the voxel world sits in a 1x1x1 cube
	cube = Cube(float3(0, 0, 0), float3(1, 1, 1));
	grid = (uint*)MALLOC64(GRIDSIZE3 * sizeof(uint));
	memset(grid, 0, GRIDSIZE3 * sizeof(uint));
#endif // TWOLEVEL

}

void Scene::GenerateGrid() {
	//for each world call the generate grid function
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			worlds[i]->GenerateGrid();
		} else {
			worlds[i] = new VoxelWorld();
			worlds[i]->GenerateGrid();
		}
	}
}

void Scene::Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex, const int worldIndex) {
	VoxelWorld* w = worlds[worldIndex];

	if (!w) {
		// create a new world
		w = new VoxelWorld();
		worlds[worldIndex] = w;
	}

	w->Set(x, y, z, v, materialIndex);
}

void Tmpl8::Scene::ConstructBVH() {

}

void Tmpl8::Scene::CLearWorlds() {
	//delete all worlds
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			delete worlds[i];
		}
	}
	worlds.clear();
}

void Scene::FindNearestEmpty(Ray& ray) const {
	float nearest = 1e34f;
	Ray nearestRay = ray;
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			Ray copy = ray;
			worlds[i]->FindNearestEmpty(copy);
			copy.worldIndex = i;
			if (copy.t <= nearest) {
				nearest = copy.t;
				nearestRay = copy;
			}
		}
	}
	ray = nearestRay;
}

void Scene::FindNearest(Ray& ray) const {
	float nearest = 1e34f;
	Ray nearestRay = ray;
	int mostSteps = 0;
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			Ray copy = ray;
			worlds[i]->FindNearest(copy);
			copy.worldIndex = i;
			if (copy.t <= nearest) {
				nearest = copy.t;
				nearestRay = copy;
				if (copy.steps > mostSteps) {
					mostSteps = copy.steps;
				}
			}
		}
	}
	ray = nearestRay;
	ray.steps = mostSteps;
}


bool Scene::IsOccluded(Ray& ray) const {
	float nearest = 1e34f;
	Ray nearestRay = ray;
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			Ray copy = ray;
			bool occluded = worlds[i]->IsOccluded(copy);
			copy.worldIndex = i;
			if (occluded) {
				nearest = copy.t;
				nearestRay = copy;
				ray = nearestRay;
				return true;
			}
		}
	}
	ray = nearestRay;
	return false;
}

bool Tmpl8::Scene::IsOccluded(Ray& ray, const int worldIndex) const {
	if (worldIndex < 0 || worldIndex >= worlds.size()) {
		return false;
	}
	return worlds[worldIndex]->IsOccluded(ray);
}

bool Tmpl8::Scene::DrawImGui() {
	//for each world call the draw function

	//Add world button
	bool changed = false;

	//text for the new world size
	ImGui::Text("New World Setting");
	ImGui::DragInt3("Grid Dimensions", &newWorldSize.x, 1.0f, 1, 256);

	if (ImGui::Button("Add World")) {
		VoxelWorld* w = new VoxelWorld(newWorldSize);
		worlds.push_back(w);
		//w->RandomizeTransform();
		w->GenerateGrid();
		changed = true;
	}

	int indexToRemove = -1;
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			std::string label = "Voxel World " + std::to_string(i);
			if (!ImGui::CollapsingHeader(label.c_str())) continue;
			std::string deleteLabel = "Delete##" + std::to_string(i);
			if (ImGui::Button(deleteLabel.c_str())) {
				indexToRemove = i;
			}

			changed |= worlds[i]->DrawImGui(i);
		}
	}

	if (indexToRemove != -1) {
		delete worlds[indexToRemove];
		worlds.erase(worlds.begin() + indexToRemove);
		changed = true;
	}

	return changed;
}

void Tmpl8::Scene::Clear(const uint v) {
	for (int i = 0; i < worlds.size(); i++) {
		if (worlds[i]) {
			worlds[i]->Clear(v);
		}
	}
}

#ifdef TWOLEVEL
void Brick::Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex) {
	const uint newX = x & (BRICKSIZE - 1);
	const uint newY = y & (BRICKSIZE - 1);
	const uint newZ = z & (BRICKSIZE - 1);

	if (newX >= BRICKSIZE || newY >= BRICKSIZE || newZ >= BRICKSIZE) {
		return;
	}

	uint index = GetVoxelIndex(newX, newY, newZ);
	uint gridVoxel = grid[index] & 0x00FFFFFF;

	uint voxel = (materialIndex << 24) | v;
	uint voxelWithNoMaterial = voxel & 0x00FFFFFF;

	if (gridVoxel == 0 && voxelWithNoMaterial != 0) {
		voxelCount.fetch_add(1, std::memory_order_relaxed);
	} else if (gridVoxel != 0 && voxelWithNoMaterial == 0) {
		voxelCount.fetch_sub(1, std::memory_order_relaxed);
	}

	grid[index] = voxel;
}

void Tmpl8::Brick::Clear(const uint v) {
	//clear the grid
	memset(grid, v, BRICKSIZE3 * sizeof(uint));
	voxelCount = 0;
	if (v != 0) {
		voxelCount = BRICKSIZE3;
	}
}


// Helper function to advance the traversal state in the X direction
void AdvanceInXDirection(DDAState& state) {
	state.travelDistance = state.nextIntersection.x;
	state.posX += state.stepDirection.x;
	state.nextIntersection.x += state.deltaDistance.x;
}

// Helper function to advance the traversal state in the Y direction
void AdvanceInYDirection(DDAState& state) {
	state.travelDistance = state.nextIntersection.y;
	state.posY += state.stepDirection.y;
	state.nextIntersection.y += state.deltaDistance.y;
}

// Helper function to advance the traversal state in the Z direction
void AdvanceInZDirection(DDAState& state) {
	state.travelDistance = state.nextIntersection.z;
	state.posZ += state.stepDirection.z;
	state.nextIntersection.z += state.deltaDistance.z;
}

// Helper function to check if the current state is outside the grid bounds
bool IsOutsideGrid(const DDAState& state) {
	return state.posX >= BRICKSIZE || state.posY >= BRICKSIZE || state.posZ >= BRICKSIZE;
}
//find nearest voxel inside brick
void Brick::FindNearest(Ray& ray, const float& brickEntryT) const {
	// Initialize traversal state for 3D DDA algorithm
	DDAState s;
	s.travelDistance = brickEntryT;

	if (!Setup3DDDA(ray, s)) {
		return; // Exit if ray setup fails
	}

	// Traverse the grid to find the nearest intersecting voxel
	while (true) {
		// Calculate the current cell index and check for intersection
		//clamp the values to the brick size using bitwise AND
		s.posX = s.posX & (BRICKSIZE - 1);
		s.posY = s.posY & (BRICKSIZE - 1);
		s.posZ = s.posZ & (BRICKSIZE - 1);

		int index = GetVoxelIndex(s.posX, s.posY, s.posZ);
		uint cell = grid[index];
		ray.steps++; // Increment the number of steps the ray has taken

		// If an intersecting voxel is found, update the ray and exit the loop
		uint voxelWithNoMaterial = cell & 0x00FFFFFF;
		if (voxelWithNoMaterial) {

#if SPHERES
			float3 voxelCenter = float3(s.posX + 0.5f, s.posY + 0.5f, s.posZ + 0.5f);
			voxelCenter += gridPosition * BRICKSIZE;
			voxelCenter /= WORLDSIZE;

			float sphereRadius = 0.5f / WORLDSIZE;
			// Perform ray-sphere intersection test
			float3 oc = ray.O - voxelCenter;
			float a = dot(ray.D, ray.D);
			float b = 2.0f * dot(oc, ray.D);
			float c = dot(oc, oc) - sphereRadius * sphereRadius;
			float discriminant = b * b - 4 * a * c;

			if (discriminant > 0) {
				float dist = (-b - sqrt(discriminant)) / (2.0f * a);
				if (dist < ray.t) {
					ray.t = dist;
					ray.voxel = cell;
					ray.index = index;
					ray.N = (ray.O + ray.t * ray.D - voxelCenter) / sphereRadius;
					break; // Intersection found, exit traversal
				}
			}
#else
			ray.t = s.travelDistance;
			ray.voxel = cell;
			ray.index = index;
			break;
#endif
		}

		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= BRICKSIZE) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= BRICKSIZE) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}

		// Break the loop if the ray exits the grid bounds
		if (IsOutsideGrid(s)) {
			break;
		}
	}
}

bool Tmpl8::Brick::FindNearestEmpty(Ray& ray, const float& brickEntryT) const {
	// Initialize traversal state for 3D DDA algorithm
	DDAState s;
	s.travelDistance = brickEntryT;
	if (!Setup3DDDA(ray, s)) {
		return false; // Exit if ray setup fails
	}

	// Traverse the grid to find the nearest intersecting voxel
	while (true) {
		// Calculate the current cell index and check for intersection
		//clamp the values to the brick size using bitwise AND
		s.posX = s.posX & (BRICKSIZE - 1);
		s.posY = s.posY & (BRICKSIZE - 1);
		s.posZ = s.posZ & (BRICKSIZE - 1);

		int index = GetVoxelIndex(s.posX, s.posY, s.posZ);
		uint cell = grid[index];
		ray.steps++; // Increment the number of steps the ray has taken

		int materialIndex = cell >> 24;
		Material m = MaterialList[materialIndex];
		uint voxelWithNoMaterial = cell & 0x00FFFFFF;
		if (voxelWithNoMaterial || m.transparency == 0.0f) {
			ray.t = s.travelDistance;
			ray.voxel = cell;
			ray.index = index;
			return true;
		}

		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= BRICKSIZE) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= BRICKSIZE) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}

		// Break the loop if the ray exits the grid bounds
		if (IsOutsideGrid(s)) {
			break;
		}
	}
	return false;
}

bool Tmpl8::Brick::IsOccluded(const Ray& ray, const float& brickEntryT) const {

	// Initialize traversal state for 3D DDA algorithm
	DDAState s;
	s.travelDistance = brickEntryT;

	if (!Setup3DDDA(ray, s)) {
		return false; // Exit if ray setup fails
	}

	// Traverse the grid to find the nearest intersecting voxel
	while (true) {
		// Calculate the current cell index and check for intersection
		//clamp the values to the brick size using bitwise AND
		s.posX = s.posX & (BRICKSIZE - 1);
		s.posY = s.posY & (BRICKSIZE - 1);
		s.posZ = s.posZ & (BRICKSIZE - 1);

		int index = GetVoxelIndex(s.posX, s.posY, s.posZ);
		uint cell = grid[index];

		// If an intersecting voxel is found, return true
		uint voxelWithNoMaterial = cell & 0x00FFFFFF;

		if (voxelWithNoMaterial) {
#if SPHERES
			float3 voxelCenter = float3(s.posX + 0.5f, s.posY + 0.5f, s.posZ + 0.5f);
			voxelCenter += gridPosition * BRICKSIZE;
			voxelCenter /= BRICKSIZE;

			float sphereRadius = 0.5f / BRICKSIZE;
			// Perform ray-sphere intersection test
			float3 oc = ray.O - voxelCenter;
			float b = dot(oc, ray.D);
			float c = dot(oc, oc) - sphereRadius * sphereRadius; // Using sphereRadius directly, assuming it's defined
			float discriminant = b * b - c;

			if (discriminant > 0) {
				float dist = -b - sqrt(discriminant);
				bool hit = dist < ray.t && dist > 0;
				if (hit) {
					return true;
				}
			}

#else
			return s.travelDistance < ray.t;
#endif
		}

		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= BRICKSIZE) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= BRICKSIZE) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= BRICKSIZE) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}

		// Break the loop if the ray exits the grid bounds
		if (IsOutsideGrid(s)) {
			break;
		}
	}

	return false;
}

bool Tmpl8::Brick::IsEmpty() const {
	return voxelCount == 0;
}

bool Tmpl8::Brick::Setup3DDDA(const Ray& ray, DDAState& state) const {
	state.stepDirection = make_int3(1 - ray.Dsign * 2);
	const float3 posInGrid = WORLDSIZE * (ray.O + (state.travelDistance + 0.000005f) * ray.D);
	const float3 gridPlanes = (ceilf(posInGrid) - ray.Dsign) * VOXELSIZE;
	const int3 min = gridPosition * BRICKSIZE;
	const int3 max = min + BRICKSIZE;
	const int3 P = clamp(make_int3(posInGrid), min, max - 1);
	state.posX = P.x, state.posY = P.y, state.posZ = P.z;
	state.deltaDistance = VOXELSIZE * float3(state.stepDirection) * ray.rD;
	state.nextIntersection = (gridPlanes - ray.O) * ray.rD;
	return true;
}
#endif // TWOLEVEL



bool VoxelWorld::Setup3DDDA(const Ray& ray, DDAState& state) const {
	// if ray is not inside the world: advance until it is
	state.travelDistance = 0;
	if (!cube.Contains(ray.O)) {
		state.travelDistance = cube.Intersect(ray);
		if (state.travelDistance > 1e33f) return false; // ray misses voxel data entirely
	}

	// setup amanatides & woo - assume world is 1x1x1, from (0,0,0) to (1,1,1)
	static const float brickSize = 1.0f / GRIDDIMENSIONS;
	state.stepDirection = make_int3(1 - ray.Dsign * 2);
	const float3 posInGrid = GRIDDIMENSIONS * (ray.O + (state.travelDistance + 0.00001f) * ray.D);
	const float3 gridPlanes = (ceilf(posInGrid) - ray.Dsign) * brickSize;
	const int3 P = clamp(make_int3(posInGrid), 0, gridDimensions - 1);
	state.posX = P.x, state.posY = P.y, state.posZ = P.z;
	state.deltaDistance = brickSize * float3(state.stepDirection) * ray.rD;
	state.nextIntersection = (gridPlanes - ray.O) * ray.rD;

	// proceed with traversal
	return true;
}

void Tmpl8::VoxelWorld::ResizeCube(const int3 newGridSize) {
	const float3 newSize = float3(GRIDDIMENSIONS) / newGridSize;
	const float3 cubeSize = float3(1) / newSize;
	cube = Cube(float3(0, 0, 0), cubeSize);
}

VoxelWorld::VoxelWorld(const int3 _newGridDimensions) {
	NoiseColor = 0;
	SetActive(true);
	gridDimensions = _newGridDimensions;
	newGridDimensions = gridDimensions;

	const float3 newSize = float3(GRIDDIMENSIONS) / gridDimensions;
	const float3 cubeSize = float3(1) / newSize;
	cube = Cube(float3(0, 0, 0), cubeSize);
	position = float3(0, 0, 0);
	rotation = float3(0, 0, 0);
	scale = float3(1, 1, 1);
	UpdateTransform();

	bricks = new Brick * [GetGridSize(gridDimensions)];

	//set each brick to null
	const int gridSize = GetGridSize(gridDimensions);
	for (int i = 0; i < gridSize; i++) {
		bricks[i] = 0;
	}
}

void Tmpl8::VoxelWorld::GenerateGrid() {
	const int zGridSize = BRICKSIZE * gridDimensions.z;
	const int yGridSize = BRICKSIZE * gridDimensions.y;
	const int xGridSize = BRICKSIZE * gridDimensions.x;
	const int3 worldSize = gridDimensions * BRICKSIZE;
#pragma omp parallel for schedule(dynamic)
	for (int z = 0; z < zGridSize; z++) {
		const float fz = (float)z / zGridSize;
		for (int y = 0; y < yGridSize; y++) {
			const float fy = (float)y / yGridSize;
			float fx = 0;
			for (int x = 0; x < xGridSize; x++, fx += 1.0f / xGridSize) {
				const float n = noise3D(fx, fy, fz, NoiseFrequency, NoiseAmplitude);

				//uint color = RandomColor();
				uint color;
				if (NoiseColor == 0) {
					color = ComputeVoxelColor(x, y, z, worldSize);
				} else {
					color = NoiseColor;
				}
				if (n > 0.09f) Set(x, y, z, color, 0);
			}
		}
	}
}
//find nearest brick inside world
void VoxelWorld::FindNearest(Ray& ray) const {
	if(!IsActive()) return;
	const float3 transformedOrigin = invTransform.TransformPoint(ray.O);

	const float3 transformDirection = invTransform.TransformVector(ray.D);

	Ray transformedRay = Ray(transformedOrigin, transformDirection);

	transformedRay.steps = ray.steps;
	//Find the nearest intersection in the bricks
	// setup Amanatides & Woo grid traversal

	DDAState s;
	if (!Setup3DDDA(transformedRay, s)) {
		return;
	}
	// start stepping
	while (1) {// loop unrolling?
		// calculate the brick index
		const uint bx = s.posX;
		const uint by = s.posY;
		const uint bz = s.posZ;
		// get the brick
		const int index = GetBrickIndex(bx, by, bz, gridDimensions);
		const Brick* b = bricks[index];
		transformedRay.steps++;
		if (b && !b->IsEmpty()) {
			//ray.t = s.travelDistanceravelDistance;
			//ray.voxel = 0xff0000;
			//return;
			// find the nearest intersection in the brick
			float brickEntryT = s.travelDistance;
			b->FindNearest(transformedRay, brickEntryT);

			// if an intersection was found, return
			if (transformedRay.voxel != 0) {
				ray.t = transformedRay.t;

				//ray.D = transformedRay.D;
				//ray.O = transformedRay.O;
				ray.voxel = transformedRay.voxel;
				ray.index = transformedRay.index;
				ray.steps = transformedRay.steps;
				ray.worldIndex = transformedRay.worldIndex;
				ray.Dsign = transformedRay.Dsign;
#if SPHERES
				ray.N = transformedRay.N;
#endif // SPHERES

				ray.worldTransform = transform;
				ray.invWorldTransform = invTransform;
				return;
			}
		}
		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= static_cast<uint>(gridDimensions.x)) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= static_cast<uint>(gridDimensions.y)) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}
	}
	ray.steps = transformedRay.steps;
}

void Tmpl8::VoxelWorld::FindNearestEmpty(Ray& ray) const {
	if (!IsActive()) return;
	const float3 transformedOrigin = invTransform.TransformPoint(ray.O);
	const float3 transformDirection = invTransform.TransformVector(ray.D);

	Ray transformedRay = Ray(transformedOrigin, transformDirection);
	transformedRay.steps = ray.steps;

	//Find the nearest intersection in the bricks
	// setup Amanatides & Woo grid traversal

	DDAState s;
	if (!Setup3DDDA(transformedRay, s)) {
		return;
	}
	// start stepping
	while (1) {// loop unrolling?
		// calculate the brick index
		const uint bx = s.posX /*>> BRICKBITS*/;
		const uint by = s.posY /*>> BRICKBITS*/;
		const uint bz = s.posZ /*>> BRICKBITS*/;
		// get the brick
		const int index = GetBrickIndex(bx, by, bz, gridDimensions);
		const Brick* b = bricks[index];
		transformedRay.steps++;
		if (b) {
			if (b->IsEmpty()) {
				return;
			}
			// find the nearest intersection in the brick
			const float brickEntryT = s.travelDistance;

			if (b->FindNearestEmpty(transformedRay, brickEntryT)) {

				const Material m = transformedRay.GetMaterial();

				if (m.transparency == 0.0f || transformedRay.voxel == 0) {
					ray.t = transformedRay.t;
					//ray.O = transformedRay.O;
					//ray.D = transformedRay.D;

					ray.voxel = transformedRay.voxel;
					ray.index = transformedRay.index;
					ray.steps = transformedRay.steps;
					ray.worldIndex = transformedRay.worldIndex;
					ray.Dsign = transformedRay.Dsign;
#if SPHERES
					ray.N = transformedRay.N;
#endif // SPHERES

					ray.worldTransform = transform;
					ray.invWorldTransform = invTransform;
					return;
				}
			}
		}
		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= static_cast<uint>(gridDimensions.x)) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= static_cast<uint>(gridDimensions.y)) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}
	}
	ray.steps = transformedRay.steps;
}

bool Tmpl8::VoxelWorld::IsOccluded(const Ray& ray) const {
	if (!IsActive()) return false;
	float3 transformedOrigin = invTransform.TransformPoint(ray.O);
	float3 transformedDirection = invTransform.TransformVector(ray.D); // Normalization might be necessary

	Ray transformedRay(transformedOrigin, transformedDirection, ray.t);
	//Find the nearest intersection in the bricks
	//setup Amanatides & Woo grid traversal

	DDAState s, bs;
	if (!Setup3DDDA(transformedRay, s)) return false;
	// start stepping
	while (1) {// loop unrolling?
		// calculate the brick index
		uint bx = s.posX /*>> BRICKBITS*/;
		uint by = s.posY /*>> BRICKBITS*/;
		uint bz = s.posZ /*>> BRICKBITS*/;
		// get the brick
		int index = GetBrickIndex(bx, by, bz, gridDimensions);
		Brick* b = bricks[index];
		if (b && !b->IsEmpty()) {
			float brickEntryT = s.travelDistance;

			bool occluded = b->IsOccluded(transformedRay, brickEntryT);

			// if an intersection was found, return
			if (occluded) return true;
		}
		if (s.nextIntersection.x < s.nextIntersection.y) {
			if (s.nextIntersection.x < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.x, s.posX += s.stepDirection.x;
				if (s.posX >= static_cast<uint>(gridDimensions.x)) break;
				s.nextIntersection.x += s.deltaDistance.x;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		} else {
			if (s.nextIntersection.y < s.nextIntersection.z) {
				s.travelDistance = s.nextIntersection.y, s.posY += s.stepDirection.y;
				if (s.posY >= static_cast<uint>(gridDimensions.y)) break;
				s.nextIntersection.y += s.deltaDistance.y;
			} else {
				s.travelDistance = s.nextIntersection.z, s.posZ += s.stepDirection.z;
				if (s.posZ >= static_cast<uint>(gridDimensions.z)) break;
				s.nextIntersection.z += s.deltaDistance.z;
			}
		}
	}
	return false;
}

bool Tmpl8::VoxelWorld::DrawImGui(const int index) {

	// Start with the assumption that nothing has changed
	bool changed = false;

	// Begin a tab bar
	if (ImGui::BeginTabBar("##TabBar", ImGuiTabBarFlags_FittingPolicyScroll)) {

		// Tab for transform settings
		if (ImGui::BeginTabItem("Transform")) {
			std::string resetLabel = "Reset##" + std::to_string(index);
			if (ImGui::Button(resetLabel.c_str())) {
				position = float3(0, 0, 0);
				rotation = float3(0, 0, 0);
				scale = float3(1, 1, 1);
				changed = true;
			}

			float speed = 0.005f;
			if (InputManager::GetInstance().GetKey(KeyValues::SHIFT)) speed = 0.02f;

			std::string positionLabel = "Position##" + std::to_string(index);
			std::string rotationLabel = "Rotation##" + std::to_string(index);
			std::string scaleLabel = "Scale##" + std::to_string(index);
			std::string enabledLabel = "Enabled##" + std::to_string(index);
			changed |= ImGui::DragFloat3(positionLabel.c_str(), &position.x, speed);
			changed |= ImGui::DragFloat3(rotationLabel.c_str(), &rotation.x, speed);
			changed |= ImGui::DragFloat3(scaleLabel.c_str(), &scale.x, speed);
			bool enabled = IsActive();
			if (ImGui::Checkbox(enabledLabel.c_str(), &enabled)) {
				changed = true;
				SetActive(enabled);
			}

			ImGui::EndTabItem();
		}

		//Tab for brick settings
		if (ImGui::BeginTabItem("Brick Settings")) {

			if (ImGui::SliderInt3("Grid Size", &newGridDimensions.x, 1, 265)) {
				Resize(newGridDimensions);
				changed = true;
			}

			//Imgui text with the new world size
			int3 worldSize = gridDimensions * BRICKSIZE;
			std::string worldSizeText = "World Size: " + std::to_string(worldSize.x) + "x" + std::to_string(worldSize.y) + "x" + std::to_string(worldSize.z);
			ImGui::Text(worldSizeText.c_str());

			ImGui::EndTabItem();
		}

		// Tab for noise settings
		if (ImGui::BeginTabItem("Noise Settings")) {
			if (ImGui::Button("Clear World")) {
				for (int i = 0; i < GetGridSize(gridDimensions); i++) {
					if (bricks[i]) {
						delete bricks[i];
						bricks[i] = 0;
					}
				}
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("3D Noise")) {
				GenerateGrid();
				changed = true;
			}

			intColorPicker("Noise Color", NoiseColor);
			ImGui::SliderFloat("Noise Amplitude", &NoiseAmplitude, 0.0f, 1.0f);
			ImGui::SliderFloat("Noise Frequency", &NoiseFrequency, 0.0f, 10.0f);
			ImGui::EndTabItem();
		}

		//tab that shows the corners of the world
		if (ImGui::BeginTabItem("Matrix")) {
			MatrixProperties("testets", transform);
		}

		if (ImGui::BeginTabItem("Bricks")) {
			for (int i = 0; i < GetGridSize(gridDimensions); i++) {
				Brick* b = bricks[i];
				if (b) {
					//imgui text with the brick index
					std::string brickIndexText = "Brick " + std::to_string(i);
					ImGui::Text(brickIndexText.c_str());
					//Imgui text with voxel count
					std::string voxelCountText = "Voxel Count: " + std::to_string(b->voxelCount);
					ImGui::Text(voxelCountText.c_str());

				}
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	// If anything was changed, update the transform
	if (changed) {
		UpdateTransform();
	}

	// Return whether anything was changed
	return changed;
}

void Tmpl8::VoxelWorld::Set(const uint x, const uint y, const uint z, const uint v, const int materialIndex) {
	// calculate the brick index
	const uint bx = x >> BRICKBITS;
	const uint by = y >> BRICKBITS;
	const uint bz = z >> BRICKBITS;

	//if out of bounds, return
	if (bx >= static_cast<uint>(gridDimensions.x) || by >= static_cast<uint>(gridDimensions.y) || bz >= static_cast<uint>(gridDimensions.z)) return;

	// Calculate the index of the brick in the 1D array
	int index = GetBrickIndex(bx, by, bz, gridDimensions);

	Brick* b = bricks[index];

	if (!b) {
		// create a new brick
		int3 gridPos = int3(bx, by, bz);
		b = new Brick(gridPos);
		bricks[index] = b;
	}
	// set the voxel in the brick
	b->Set(x, y, z, v, materialIndex);
}

//clear the world with a specific value
void Tmpl8::VoxelWorld::Clear(const uint v) {
	//iterate over all bricks
	for (int i = 0; i < GetGridSize(gridDimensions); i++) {
		//if the brick exists, clear it
		if (bricks[i]) {
			bricks[i]->Clear(v);
		}
	}
}

void Tmpl8::VoxelWorld::RandomizeTransform() {
	position = float3(RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f), RandomFloat(-1.0f, 1.0f));
	rotation = float3(RandomFloat(-PI, PI), RandomFloat(-PI, PI), RandomFloat(-PI, PI));
	float size = RandomFloat(0.1f, 2.0f);
	scale = float3(size, size, size);
	UpdateTransform();
}

void Tmpl8::VoxelWorld::SetActive(const bool active) {
	if (active) {
		NoiseColor = -1;
	} else {
		NoiseColor = 0;
	}
}

bool Tmpl8::VoxelWorld::IsActive() const {
	return NoiseColor == -1;
}

float3 Tmpl8::VoxelWorld::GetSize() const {
	//return cube size
	return cube.GetSize();
}

std::vector<float3> Tmpl8::VoxelWorld::GetCorners() const {
	const float3 cubeSize = cube.GetSize();
	std::vector<float3> corners;
	corners.push_back(transform.TransformPoint(float3(0, 0, 0) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(1, 0, 0) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(0, 1, 0) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(1, 1, 0) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(0, 0, 1) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(1, 0, 1) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(0, 1, 1) * cubeSize));
	corners.push_back(transform.TransformPoint(float3(1, 1, 1) * cubeSize));
	return corners;
}

//function to resize the world to the new grid dimensions
void Tmpl8::VoxelWorld::Resize(const int3 newGridSize) {
	// Calculate the total size for the new grid
	const int newGridTotalSize = GetGridSize(newGridSize);

	// Create a new array of Brick pointers for the new grid size
	Brick** newBricks = new Brick * [newGridTotalSize];

	// Initialize the new bricks array to null
	for (int i = 0; i < newGridTotalSize; i++) {
		newBricks[i] = nullptr;
	}

	// Copy existing bricks to the new array
	// Assuming GetGridSize(int3) computes the total number of bricks for the given dimensions
	int minSizeX = std::min(gridDimensions.x, newGridSize.x);
	int minSizeY = std::min(gridDimensions.y, newGridSize.y);
	int minSizeZ = std::min(gridDimensions.z, newGridSize.z);

	for (int x = 0; x < minSizeX; ++x) {
		for (int y = 0; y < minSizeY; ++y) {
			for (int z = 0; z < minSizeZ; ++z) {
				int oldIndex = x + y * gridDimensions.x + z * gridDimensions.x * gridDimensions.y;
				int newIndex = x + y * newGridSize.x + z * newGridSize.x * newGridSize.y;

				// Copy the pointer from the old to the new array, preserving the brick information
				newBricks[newIndex] = bricks[oldIndex];
			}
		}
	}

	// Delete the old array of bricks
	delete[] bricks;

	// Set the new array as the current one
	bricks = newBricks;

	// Update the grid dimensions
	gridDimensions = newGridSize;

	// Resize other related properties if needed (not shown)
	ResizeCube(newGridSize);
}


void Tmpl8::VoxelWorld::UpdateTransform() {
	const float3 center = cube.GetSize() / 2.0f;
	const float3 negativeCenter = -center;

	transform = mat4::Translate(position)
		* mat4::Translate(center) // Move to center
		* mat4::RotateX(rotation.x) * mat4::RotateY(rotation.y) * mat4::RotateZ(rotation.z)
		* mat4::Scale(scale)
		* mat4::Translate(negativeCenter); // Move back after rotation

	invTransform = transform.Inverted();
}

void Tmpl8::VoxelWorld::UpdateTransformCentered() {
	//make the transform centered
	const float3 center = cube.GetSize() / 2.0f;
	const float3 negativeCenter = -center;

	transform = mat4::Translate(position)
		* mat4::RotateX(rotation.x) * mat4::RotateY(rotation.y) * mat4::RotateZ(rotation.z)
		* mat4::Scale(scale)
		* mat4::Translate(negativeCenter); // Move back after rotation

	invTransform = transform.Inverted();
}

void Tmpl8::VoxelWorld::UpdateTranformRotateLocal() {
	const float3 center = cube.GetSize() / 2.0f;
	const float3 negativeCenter = -center;

	transform =
		mat4::RotateZ(rotation.z)
		* mat4::Translate(position) * mat4::RotateX(rotation.x) * mat4::RotateY(rotation.y);

	invTransform = transform.Inverted();
}
