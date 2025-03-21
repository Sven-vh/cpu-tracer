#include "precomp.h"


PuzzleLevel::PuzzleLevel() {
	Size = int3(0, 0, 0);
}

PuzzleLevel::PuzzleLevel(const std::string levelPath) {
	Size = int3(0, 0, 0);
	LoadLevel(levelPath);
}

PuzzleLevel::~PuzzleLevel() {
}

void PuzzleLevel::DrawLevel(Scene scene, const int3) {
	for (int i = 0; i < Voxels.size(); i++) {
		Voxel voxel = Voxels[i];
		scene.Set(voxel.position.x, voxel.position.y, voxel.position.z, voxel.color, voxel.materialIndex, 0);
	}
}
void PuzzleLevel::LoadLevel(const std::string levelPath) {
	// Open file
	FILE* file = fopen(levelPath.c_str(), "rb");
	if (!file) {
		printf("Failed to open file with name: %s\n", levelPath.c_str());
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

	for (uint32_t i = 0; i < voxelScene->num_models; i++) {
		const ogt_vox_model* model = voxelScene->models[i];
		const ogt_vox_instance instance = voxelScene->instances[i];
		int sizeX = model->size_x;
		int sizeY = model->size_y;
		int sizeZ = model->size_z;

		Size = int3(sizeX, sizeZ, sizeY);

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
						for (int m = 0;m < MaterialList.size(); m++) {
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

						Voxel voxel{ int3(adjustedX, adjustedY, adjustedZ), color, materialIndex, colorIndex };

						//print color index
						if (colorIndex == LAMP_INDEX) {
							printf("Color index: %u\n", colorIndex);
							Lamp lamp;
							lamp.voxel = voxel;
							lamp.requiredRays = 4;
							Lamps.push_back(lamp);
						} else if (colorIndex == ANTI_LAMP_INDEX) {
							printf("Color index: %u\n", colorIndex);
							Lamp lamp;
							lamp.voxel = voxel;
							lamp.requiredRays = 0;
							Lamps.push_back(lamp);
						}

						Voxels.push_back(voxel);
					}
				}
			}
		}
	}

	//printf("VOX scene processed\n");
	// Free VOX scene after processing
	ogt_vox_destroy_scene(voxelScene);
}

