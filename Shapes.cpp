#include "precomp.h"


void DrawSphere(Scene& scene, const int3& center, float radius, const uint color, const int materialIndex, const int worldIndex, float probability) {

	const int3 min = center - make_int3(static_cast<int>(radius));
	const int3 max = center + make_int3(static_cast<int>(radius));

	for (int z = min.z; z < max.z; z++) {
		for (int y = min.y; y < max.y; y++) {
			for (int x = min.x; x < max.x; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
				float3 delta = pos - center;
				float distance = length(delta) - radius;
				if (distance < 0) {
					scene.Set(x, y, z, RandomFloat() < probability ? color : 0, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawHollowSphere(Scene& scene, const float3& center, const float radius, const uint color, const int materialIndex, const int worldIndex) {
	const int3 min = make_int3(center - float3(radius));
	const int3 max = make_int3(center + float3(radius));

	for (int z = min.z; z < max.z; z++) {
		for (int y = min.y; y < max.y; y++) {
			for (int x = min.x; x < max.x; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
				float3 delta = pos - center;
				float distance = length(delta) - radius;
				if (distance < 1.0f && distance > -1.0f) {
					scene.Set(x, y, z, color, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawCube(Scene& scene, const float3& center, float size, const uint color, const int materialIndex, const int worldIndex) {

	for (int z = 0; z < WORLDSIZE; z++) {
		for (int y = 0; y < WORLDSIZE; y++) {
			for (int x = 0; x < WORLDSIZE; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) / static_cast<float>(WORLDSIZE);
				float3 delta = pos - center;
				float distance = max(abs(delta.x), max(abs(delta.y), abs(delta.z))) - size;
				if (distance < 0) {
					scene.Set(x, y, z, color, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawHollowCube(Scene& scene, const float3& center, float size, const uint color, const int materialIndex, const int worldIndex) {

	for (int z = 0; z < WORLDSIZE; z++) {
		for (int y = 0; y < WORLDSIZE; y++) {
			for (int x = 0; x < WORLDSIZE; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) / static_cast<float>(WORLDSIZE);
				float3 delta = pos - center;
				float distance = max(abs(delta.x), max(abs(delta.y), abs(delta.z))) - size;
				if (distance < 0.1f && distance > -0.1f) {
					scene.Set(x, y, z, color, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawBox(Scene& scene, const float3& center, const float3& size, const uint color, const int materialIndex, const int worldIndex) {

	for (int z = 0; z < WORLDSIZE; z++) {
		for (int y = 0; y < WORLDSIZE; y++) {
			for (int x = 0; x < WORLDSIZE; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) / static_cast<float>(WORLDSIZE);
				float3 delta = pos - center;
				float3 distance = fabs(delta) - size;
				if (distance.x < 0 && distance.y < 0 && distance.z < 0) {
					scene.Set(x, y, z, color, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawHollowBox(Scene& scene, const float3& center, const float3& size, const uint color, const int materialIndex, const int worldIndex) {

	for (int z = 0; z < WORLDSIZE; z++) {
		for (int y = 0; y < WORLDSIZE; y++) {
			for (int x = 0; x < WORLDSIZE; x++) {
				float3 pos = float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) / static_cast<float>(WORLDSIZE);
				float3 delta = pos - center;
				float3 distance = fabs(delta) - size;
				if (distance.x < 0.1f && distance.y < 0.1f && distance.z < 0.1f) {
					scene.Set(x, y, z, color, materialIndex, worldIndex);
				}
			}
		}
	}
}

void DrawLine(Scene& scene, const float3& start, const float3& end, const uint color, const int materialIndex, const int) {
	float3 delta = end - start;
	float length = max(abs(delta.x), max(abs(delta.y), abs(delta.z)));
	for (int i = 0; i < length * WORLDSIZE; i++) {
		float t = i / (length * WORLDSIZE);
		float3 pos = lerp(start, end, t);
		scene.Set(int(pos.x * WORLDSIZE), int(pos.y * WORLDSIZE), int(pos.z * WORLDSIZE), color, materialIndex);
	}
}

void DrawVoxFile(Scene& scene, const char* filename, float3 position, const int worldIndex) {
	// Open file
	FILE* file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to open file with name: %s\n", filename);
		return; // Failed to open file
	}

	// Determine file size
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	if (fileSize <= 0) {
		fclose(file);
		printf("File is empty or error in ftell\n");
		return; // File is empty or error in ftell
	}
	fseek(file, 0, SEEK_SET);

	// Allocate buffer based on file size
	uint8_t* buffer = (uint8_t*)MALLOC64(fileSize);
	if (!buffer) {
		fclose(file);
		printf("Memory allocation failed\n");
		return; // Memory allocation failed
	}

	// Read file into buffer
	size_t bytesRead = fread(buffer, 1, fileSize, file);
	if (bytesRead != fileSize) {
		// Error reading file or file read partially
		FREE64(buffer);
		fclose(file);
		printf("Error reading file or file read partially\n");
		return;
	}

	// Close the file as it's no longer needed
	fclose(file);

	// Read VOX scene from buffer
	const ogt_vox_scene* voxelScene = ogt_vox_read_scene(buffer, fileSize);
	const ogt_vox_matl_array* palette = &voxelScene->materials;
	FREE64(buffer); // Free buffer immediately after use

	if (!voxelScene) {
		printf("Failed to read VOX scene\n");
		return; // Failed to read VOX scene
	}

	position *= WORLDSIZE;

	// Process VOX scene
	printf("# of layers: %u\n", voxelScene->num_layers);
	printf("# of models: %u\n", voxelScene->num_models);
	printf("# of groups: %u\n", voxelScene->num_groups);

	for (uint32_t i = 0; i < voxelScene->num_models; i++) {
		const ogt_vox_model* model = voxelScene->models[i];
		const ogt_vox_instance instance = voxelScene->instances[i];
		int sizeX = model->size_x;
		int sizeY = model->size_y;
		int sizeZ = model->size_z;

		int3 offset = transformToInt3(instance.transform);
		//convert from right handed to left handed coordinate system
		int3 newOffset = int3(offset.x, offset.z, offset.y);

		for (int x = 0; x < sizeX; x++) {
			for (int y = 0; y < sizeY; y++) {
				for (int z = 0; z < sizeZ; z++) {
					uint8_t colorIndex = model->voxel_data[x + y * sizeX + z * sizeX * sizeY];
					if (colorIndex != 0) {

						//get the material of the voxel from the voxel scene
						ogt_vox_rgba voxColor = voxelScene->palette.color[colorIndex];
						//convert the color to a RGB uint format
						uint color = (voxColor.r << 16) | (voxColor.g << 8) | voxColor.b;

						ogt_vox_matl matl = palette->matl[colorIndex];

						int materialIndex = static_cast<int>(MaterialList.size());
						Material newMaterial = Material(materialIndex, matl.rough, matl.metal, matl.trans, matl.ior);

						//check if the material already exists
						for (int m = 0; m < MaterialList.size(); m++) {
							if (newMaterial == MaterialList[m]) {
								materialIndex = m;
								break;
							}
						}
						//if the material does not exist, add it to the list
						if (materialIndex == MaterialList.size()) {
							MaterialList.push_back(newMaterial);
						}

						//store the material index in the first 8 bits of the color
						color |= (materialIndex << 24);

						// Adjusted positions for different coordinate system
						int adjustedX = x + newOffset.x;
						int adjustedY = z + newOffset.y;
						int adjustedZ = y + newOffset.z;

						adjustedX += static_cast<int>(position.x);
						adjustedY += static_cast<int>(position.y);
						adjustedZ += static_cast<int>(position.z);

						//print position
						//printf("x: %d, y: %d, z: %d\n", adjustedX, adjustedY, adjustedZ);

						//check if the position is within the scene
						scene.Set(adjustedX, adjustedY, adjustedZ, color, materialIndex, worldIndex);
						if (adjustedX >= 0 && adjustedX < WORLDSIZE && adjustedY >= 0 && adjustedY < WORLDSIZE && adjustedZ >= 0 && adjustedZ < WORLDSIZE) {
						}

						//scene.Set(adjustedX + position.x, adjustedY + position.y, adjustedZ + position.z , color, materialIndex);
					}
				}
			}
		}
	}

	printf("VOX scene processed\n");
	// Free VOX scene after processing
	ogt_vox_destroy_scene(voxelScene);
}

void LoadVoxFile(Scene& scene, const char* filename) {
	// Open file
	FILE* file = fopen(filename, "rb");
	if (!file) {
		printf("Failed to open file with name: %s\n", filename);
		return; // Failed to open file
	}

	// Determine file size
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	if (fileSize <= 0) {
		fclose(file);
		printf("File is empty or error in ftell\n");
		return; // File is empty or error in ftell
	}
	fseek(file, 0, SEEK_SET);

	// Allocate buffer based on file size
	uint8_t* buffer = (uint8_t*)MALLOC64(fileSize);
	if (!buffer) {
		fclose(file);
		printf("Memory allocation failed\n");
		return; // Memory allocation failed
	}

	// Read file into buffer
	size_t bytesRead = fread(buffer, 1, fileSize, file);
	if (bytesRead != fileSize) {
		// Error reading file or file read partially
		FREE64(buffer);
		fclose(file);
		printf("Error reading file or file read partially\n");
		return;
	}

	// Close the file as it's no longer needed
	fclose(file);

	// Read VOX scene from buffer
	const ogt_vox_scene* voxelScene = ogt_vox_read_scene(buffer, fileSize);
	const ogt_vox_matl_array* palette = &voxelScene->materials;
	FREE64(buffer); // Free buffer immediately after use

	if (!voxelScene) {
		printf("Failed to read VOX scene\n");
		return; // Failed to read VOX scene
	}

	// Process VOX scene
	printf("# of layers: %u\n", voxelScene->num_layers);
	printf("# of models: %u\n", voxelScene->num_models);
	printf("# of groups: %u\n", voxelScene->num_groups);

	for (uint32_t i = 0; i < voxelScene->num_models; i++) {
		const ogt_vox_model* model = voxelScene->models[i];
		const ogt_vox_instance instance = voxelScene->instances[i];
		int sizeX = model->size_x;
		int sizeY = model->size_y;
		int sizeZ = model->size_z;



		float3 offset = transformToFloat3(instance.transform);
		//convert from right handed to left handed coordinate system
		float offsetX = offset.x / static_cast<float>(WORLDSIZE);
		float offsetY = offset.y / static_cast<float>(WORLDSIZE);
		float offsetZ = offset.z / static_cast<float>(WORLDSIZE);
		offset = float3(offsetX, offsetZ, offsetY);

		int gridX = static_cast<int>(ceilf(static_cast<float>(sizeX) / static_cast<float>(BRICKSIZE)));
		int gridY = static_cast<int>(ceilf(static_cast<float>(sizeZ) / static_cast<float>(BRICKSIZE)));
		int gridZ = static_cast<int>(ceilf(static_cast<float>(sizeY) / static_cast<float>(BRICKSIZE)));
		int3 gridSize = make_int3(gridX, gridY, gridZ);

		VoxelWorld* newWorld = new VoxelWorld(gridSize);
		newWorld->position = offset;
		newWorld->UpdateTransform();
		scene.worlds.push_back(newWorld);
		newWorld->Resize(gridSize);


		for (int x = 0; x < sizeX; x++) {
			for (int y = 0; y < sizeY; y++) {
				for (int z = 0; z < sizeZ; z++) {
					uint8_t colorIndex = model->voxel_data[x + y * sizeX + z * sizeX * sizeY];
					if (colorIndex != 0) {

						//get the material of the voxel from the voxel scene
						ogt_vox_rgba voxColor = voxelScene->palette.color[colorIndex];
						//convert the color to a RGB uint format
						uint color = (voxColor.r << 16) | (voxColor.g << 8) | voxColor.b;

						ogt_vox_matl matl = palette->matl[colorIndex];

						int materialIndex = static_cast<int>(MaterialList.size());
						Material newMaterial = Material(materialIndex, matl.rough, matl.metal, matl.trans, matl.ior);

						//check if the material already exists
						for (int m = 0; m < MaterialList.size(); m++) {
							if (newMaterial == MaterialList[m]) {
								materialIndex = m;
								break;
							}
						}
						//if the material does not exist, add it to the list
						if (materialIndex == MaterialList.size()) {
							MaterialList.push_back(newMaterial);
						}

						//store the material index in the first 8 bits of the color
						color |= (materialIndex << 24);

						// Adjusted positions for different coordinate system
						int adjustedX = x ;
						int adjustedY = z ;
						int adjustedZ = y ;

						//print position
						//printf("x: %d, y: %d, z: %d\n", adjustedX, adjustedY, adjustedZ);

						//check if the position is within the scene
						newWorld->Set(adjustedX, adjustedY, adjustedZ, color, materialIndex);
						if (adjustedX >= 0 && adjustedX < WORLDSIZE && adjustedY >= 0 && adjustedY < WORLDSIZE && adjustedZ >= 0 && adjustedZ < WORLDSIZE) {
						}

						//scene.Set(adjustedX + position.x, adjustedY + position.y, adjustedZ + position.z , color, materialIndex);
					}
				}
			}
		}
	}

	printf("VOX scene processed\n");
	// Free VOX scene after processing
	ogt_vox_destroy_scene(voxelScene);
}

std::vector<Line> VisualizeSphere(const float radius, const float3 center, const float3, const int latitudeLines, const int longitudeLines) {
	std::vector<Line> lines;

	// Latitude lines
	for (int lat = 0; lat < latitudeLines; lat++) {
		float latAngle1 = PI * (-0.5f + (float)lat / latitudeLines);
		float latAngle2 = PI * (-0.5f + (float)(lat + 1) / latitudeLines);

		for (int lon = 0; lon <= longitudeLines; lon++) {
			float lonAngle = 2 * PI * (float)lon / longitudeLines;

			float3 point1 = center + float3(
				radius * cos(latAngle1) * cos(lonAngle),
				radius * cos(latAngle1) * sin(lonAngle),
				radius * sin(latAngle1)
			);

			float3 point2 = center + float3(
				radius * cos(latAngle2) * cos(lonAngle),
				radius * cos(latAngle2) * sin(lonAngle),
				radius * sin(latAngle2)
			);
			lines.push_back(Line(point1, point2, float4(1, 1, 1, 1)));
		}
	}

	// Longitude lines
	for (int lon = 0; lon < longitudeLines; lon++) {
		float lonAngle1 = 2.0f * PI * (float)lon / longitudeLines;
		float lonAngle2 = 2.0f * PI * (float)(lon + 1.0f) / longitudeLines;

		for (int lat = 0; lat <= latitudeLines; lat++) {
			float latAngle = PI * (-0.5f + (float)lat / latitudeLines);

			float3 point1 = center + float3(
				radius * cos(latAngle) * cos(lonAngle1),
				radius * cos(latAngle) * sin(lonAngle1),
				radius * sin(latAngle)
			);

			float3 point2 = center + float3(
				radius * cos(latAngle) * cos(lonAngle2),
				radius * cos(latAngle) * sin(lonAngle2),
				radius * sin(latAngle)
			);

			lines.push_back(Line(point1, point2, float4(1, 1, 1, 1)));
		}
	}

	return lines;
}

std::vector<Line> VisualizeCube(const float3& center, const float3& size, const float3 color) {
	std::vector<Line> lines;

	float3 min = center - size / 2.0f;
	float3 max = center + size / 2.0f;

	// Bottom
	lines.push_back(Line(float3(min.x, min.y, min.z), float3(max.x, min.y, min.z), color));
	lines.push_back(Line(float3(max.x, min.y, min.z), float3(max.x, max.y, min.z), color));
	lines.push_back(Line(float3(max.x, max.y, min.z), float3(min.x, max.y, min.z), color));
	lines.push_back(Line(float3(min.x, max.y, min.z), float3(min.x, min.y, min.z), color));

	// Top
	lines.push_back(Line(float3(min.x, min.y, max.z), float3(max.x, min.y, max.z), color));
	lines.push_back(Line(float3(max.x, min.y, max.z), float3(max.x, max.y, max.z), color));
	lines.push_back(Line(float3(max.x, max.y, max.z), float3(min.x, max.y, max.z), color));
	lines.push_back(Line(float3(min.x, max.y, max.z), float3(min.x, min.y, max.z), color));

	// Sides
	lines.push_back(Line(float3(min.x, min.y, min.z), float3(min.x, min.y, max.z), color));
	lines.push_back(Line(float3(max.x, min.y, min.z), float3(max.x, min.y, max.z), color));
	lines.push_back(Line(float3(max.x, max.y, min.z), float3(max.x, max.y, max.z), color));
	lines.push_back(Line(float3(min.x, max.y, min.z), float3(min.x, max.y, max.z), color));

	return lines;
}
