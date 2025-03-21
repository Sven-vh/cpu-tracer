#include "precomp.h"
#include "SvenUtils/TweenManager.h"
class PointLight;

void PuzzleScene::Init() {
	totalTime = 0.0f;
	printf("Puzzle Scene Init\n");

	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_1.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_2.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_3.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_4.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_5.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_6.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_7.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_8.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_9.vox"));
	levels.push_back(std::make_shared<PuzzleLevel>("Assets/levels/Level_10.vox"));

	if (StartLevel != -1 && StartLevel < levels.size()) {
		LoadLevel(levels[StartLevel]);
	} else {
		MenuScene* menu = new MenuScene();
		menu->renderer = renderer;
		renderer->sceneManager.LoadScene(menu);
	}
	renderer->CanSelectWorld = false;

	SetSimpleRenderer();
}

void PuzzleScene::Update(float deltaTime) {
	totalTime += deltaTime;
	lightIntensity = 0.5f + sin(totalTime * fadeSpeed) * 0.5f;
	HandleMouse(deltaTime);
	HandleLamps(deltaTime);

	if (findRandomLevel) {
		for (int i = 0; i < 100; i++) {
			if (GenerateRandomLevel()) {
				findRandomLevel = false;
				break;
			}
		}
	}

	if (playAnimation) {
		if (isRandomLevel) return;
		FinishLevelAnimation(deltaTime);
	}

	if (InputManager::GetInstance().GetKeyDown(KeyValues::ESCAPE)) {
		MenuScene* menu = new MenuScene();
		menu->renderer = renderer;
		renderer->sceneManager.LoadScene(menu);
	}

	TweenManager::GetInstance().Update(deltaTime);
}

void PuzzleScene::Render() {
	DrawImGUI();
}

void PuzzleScene::Exit() {
	printf("Puzzle Scene Exit\n");

	TweenManager::GetInstance().Stop();
}

void PuzzleScene::HandleMouse(const float deltaTime) {
	if (InputManager::GetInstance().GetMouseButton(0)) {

		//if Imgui is using the mouse, don't move the light
		if (ImGui::GetIO().WantCaptureMouse) return;

		Ray mouseRay = renderer->GetMouseRay();
		renderer->scene.FindNearest(mouseRay);
		if (mouseRay.voxel == 0) return;

		float3 lightPos = renderer->pointLight->position;

		float3 hitPos = mouseRay.IntersectionPoint();

		if (hitPos.y - EPSILON >= VOXELSIZE) return;

		float3 normal = mouseRay.GetNormal();
		hitPos += float3(0, VOXELSIZE * 6.75f, 0);
		hitPos += normal * (VOXELSIZE / 2.0f);

		// Calculate direction from lightPos to hitPos
		float3 direction = normalize(hitPos - lightPos);

		// Move light by a constant speed in the direction of hitPos
		float3 result = lightPos + direction * (lightSpeed * deltaTime);

		// Ensure we don't overshoot the target
		if (length(hitPos - lightPos) < length(result - lightPos)) {
			result = hitPos;
		}

		renderer->pointLight->position = result;
	}
}

void PuzzleScene::HandleLamps(const float deltaTime) {
	if (currentLevel == nullptr || playAnimation || !animationFinished) return;
	const float3 lightPos = renderer->pointLight->position;
	const float halfVoxelSize = VOXELSIZE / 2.0f;
	std::vector<float3> rayOffsets = {
		float3(halfVoxelSize, halfVoxelSize, halfVoxelSize),
		float3(-halfVoxelSize, halfVoxelSize, halfVoxelSize),
		float3(-halfVoxelSize, halfVoxelSize, -halfVoxelSize),
		float3(halfVoxelSize, halfVoxelSize, -halfVoxelSize)
	};

	for (auto& lamp : currentLevel->Lamps) {
		float3 lampPosition = (lamp.voxel.position + float3(0.5f)) / WORLDSIZE;
		lampPosition = renderer->scene.worlds[0]->transform.TransformPoint(lampPosition);
		int count = 0;
		for (const auto& offset : rayOffsets) {
			float3 rayOrigin = lampPosition + offset + float3(0, EPSILON, 0);
			float3 rayDirection = normalize(lightPos - rayOrigin);
			float distance = length(lightPos - rayOrigin);
			Ray ray(rayOrigin, rayDirection, distance);

			bool result = !renderer->scene.IsOccluded(ray);
			if (result) {
				count++;
			}

			if (result) {
				//draw a line from the lamp into the ray direction
				renderer->DrawLine(rayOrigin, rayOrigin + rayDirection * distance, float3(0, 1, 0));
			} else {
				renderer->scene.FindNearest(ray);
				float3 hitPos = ray.IntersectionPoint();
				renderer->DrawLine(rayOrigin, hitPos, float3(1, 0, 0));
			}
		}

		if (count == lamp.requiredRays) {
			lamp.isFound = true;
		} else {
			lamp.isFound = false;
		}

		if (lamp.isFound) {
			float4 lineColor = float4(0, 1, 0, lightIntensity);
			std::vector<Line> lines = VisualizeCube(lampPosition, VOXELSIZE, float3(1.0f));
			for (const auto& line : lines) {
				renderer->DrawLine(line.start, line.end, lineColor, true);
			}
		} else {
			float4 lineColor = float4(1, 0, 0, lightIntensity);
			std::vector<Line> lines = VisualizeCube(lampPosition, VOXELSIZE, float3(1.0f));
			for (const auto& line : lines) {
				renderer->DrawLine(line.start, line.end, lineColor, true);
			}
		}
	}

	bool allLampsFound = true;
	for (const auto& lamp : currentLevel->Lamps) {
		if (!lamp.isFound) {
			allLampsFound = false;
			break;
		}
	}

	if (allLampsFound) {
		//printf("All lamps found\n");
		//playAnimation = true;
		allLampsFoundTime += deltaTime;

		if (allLampsFoundTime > allLampsFoundTimeMax) {
			playAnimation = true;
		}
	} else {
		allLampsFoundTime = 0;
	}
}

void PuzzleScene::SetSimpleRenderer() {
	renderer->settings.VignetteIntensity = 0.2f;
	renderer->settings.VignetteIntensity = 0.6f;
	renderer->camera.fov = 0.52f;
	renderer->sunLight->Intensity = 0.0f;
	renderer->pointLight->Intensity = 0.016f;
	renderer->settings.EnvironmentLight = false;
	renderer->settings.DebugLines = false;
	renderer->settings.fps = 0.0f;
	renderer->settings.Exposure = 1.63f;
	renderer->renderState = PUZZLE;


	renderer->settings.Reprojection = false;
	renderer->settings.PathTracing = false;
	renderer->ambientLight->Intensity = 0.01f;
	renderer->settings.KSigma = 0.0f;
	lightSpeed = 0.15f;
	renderer->camera.UpdateProjection();
	renderer->UpdateSHaderSettings();
}

void PuzzleScene::SetAdvancedRenderer() {
	renderer->settings.VignetteIntensity = 0.2f;
	renderer->settings.VignetteIntensity = 0.6f;
	renderer->camera.fov = 0.52f;
	renderer->sunLight->Intensity = 0.0f;
	renderer->pointLight->Intensity = 0.016f;
	renderer->settings.EnvironmentLight = false;
	renderer->settings.DebugLines = false;
	renderer->settings.fps = 0.0f;
	renderer->settings.Exposure = 1.63f;

	renderer->settings.Reprojection = true;
	renderer->settings.PathTracing = true;
	renderer->ambientLight->Intensity = 0.0f;
	renderer->settings.KSigma = 0.2f;
	lightSpeed = 0.1f;
	renderer->camera.UpdateProjection();
}

void PuzzleScene::LoadLevel(std::shared_ptr<PuzzleLevel> level) {
	renderer->scene.CLearWorlds();
	currentLevel = level;
	VoxelWorld* world = new VoxelWorld();
	renderer->scene.worlds.push_back(world);

	

	float3 levelSize = currentLevel->Size;
	int3 worldSIze;
	worldSIze.x = static_cast<int>(ceil(levelSize.x / BRICKSIZE));
	worldSIze.y = static_cast<int>(ceil(levelSize.y / BRICKSIZE));
	worldSIze.z = static_cast<int>(ceil(levelSize.z / BRICKSIZE));

	world->Resize(worldSIze);

	currentLevel->DrawLevel(renderer->scene, int3(0));

	float3 camPos = levelSize / WORLDSIZE / 2.0f;
	camPos.y = 1.0f;
	camPos += float3(0.0001f, 0.0f, 0.0001f);

	renderer->camera.camPos = camPos;
	renderer->camera.camTarget = camPos - float3(0, 1, 0) - float3(EPSILON, 0, 0);
	renderer->camera.UpdateProjection();

	renderer->pointLight->position = levelSize / WORLDSIZE / 2.0f;
}

void PuzzleScene::DrawImGUI() {
	ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove);
	ImGui::BeginChild("Scrolling");
	if (!ImGui::CollapsingHeader("Game Settings"))  return;

	if (ImGui::CollapsingHeader("Light Settings")) {
		ImGui::SliderFloat("Light Speed", &lightSpeed, 0.0f, 1.0f);
		ImGui::DragFloat("Fade Speed", &fadeSpeed, 0.1f);
	}

	if (ImGui::CollapsingHeader("Level Settings")) {
		// Read all the .vox files in the assets folder and store them in a vector
		std::vector<std::string> voxFiles;
		std::string locationPath = "assets/levels/";
		for (const auto& entry : std::filesystem::directory_iterator(locationPath)) {
			if (entry.path().extension() == ".vox") {
				voxFiles.push_back(entry.path().filename().string());
			}
		}

		// Dropdown for selecting .vox file
		static int selectedItem = -1; // Index of the selected item in the combo box
		if (!voxFiles.empty()) {
			std::vector<const char*> cStrVoxFiles; // ImGui needs const char* array
			for (const auto& file : voxFiles) {
				cStrVoxFiles.push_back(file.c_str());
			}

			ImGui::Combo("Vox Files", &selectedItem, &cStrVoxFiles[0], static_cast<int>(cStrVoxFiles.size()));
		}

		if (ImGui::Button("Load Vox File") && selectedItem >= 0) {
			// Load the selected .vox file
			std::string path = locationPath + voxFiles[selectedItem];
			std::shared_ptr<PuzzleLevel> level = std::make_shared<PuzzleLevel>(path);
			levels.push_back(level);
			LoadLevel(level);
		}

		//checkbox
		ImGui::Checkbox("Play Animation", &playAnimation);
		//animation settings
		ImGui::SliderFloat("Radius Speed", &radiusSpeed, 0.0f, 5.0f);

		ImGui::SliderFloat("All Lamps Found Time", &allLampsFoundTimeMax, 0.0f, 5.0f);
	}

	if (ImGui::CollapsingHeader("Random Level")) {
		ImGui::DragInt("Seed", (int*)&seed, 1);
		if (ImGui::Button("Generate Random Level")) {
			GenerateRandomLevel();
		}
		ImGui::Checkbox("Find Random Level", &findRandomLevel);
	}

	if (ImGui::Button("Simple Renderer")) {
		SetSimpleRenderer();
	}
	if (ImGui::Button("Advanced Renderer")) {
		SetAdvancedRenderer();
	}
}

void PuzzleScene::FinishLevelAnimation(const float deltaTime) {
	//from the center of the level, remove the voxels one by one in a spiral pattern
	//when all voxels are removed, printf a message that the level is complete


	float3 corners[4] = {
		float3(0, 0, 0),
		float3(static_cast<float>(currentLevel->Size.x), 0, 0),
		float3(static_cast<float>(currentLevel->Size.x), 0, static_cast<float>(currentLevel->Size.z)),
		float3(0, 0, static_cast<float>(currentLevel->Size.z))
	};

	float3 worldCenter = currentLevel->Size / 2.0f;

	float3 sphereCenter = renderer->pointLight->position * WORLDSIZE;

	float maxDistance = 0.0f;
	for (int i = 0; i < 4; i++) {
		float distance = length(sphereCenter - corners[i]);
		if (distance > maxDistance) {
			maxDistance = distance;
		}
	}

	float t = animationRadius / maxDistance * 2.0f;
	t = ApplyEase(t, EASE_OUT_CUBIC);
	float radius = lerp(0.0f, maxDistance, t);
	animationRadius += radiusSpeed * deltaTime;
	animationFinished = false;
	DrawSphere(renderer->scene, make_int3(sphereCenter), radius, 0, 0);

	if (radius > maxDistance) {
		printf("Level Complete\n");
		playAnimation = false;
		animationRadius = 0.0f;

		//load the next level
		int currentLevelIndex = -1;
		for (int i = 0; i < levels.size(); i++) {
			if (levels[i] == currentLevel) {
				currentLevelIndex = i;
				break;
			}
		}
		currentLevelIndex++;
		if (currentLevelIndex >= levels.size()) {
			MenuScene* menu = new MenuScene();
			menu->renderer = renderer;
			renderer->sceneManager.LoadScene(menu);
			return;
		}
		LoadLevel(levels[currentLevelIndex]);
		renderer->scene.worlds[0]->position.z = 0.5f;
		renderer->scene.worlds[0]->UpdateTransform();
		/*Tween* tween = new Tween(renderer->scene.worlds[0]->position.x, 0.0f, 1.0f);*/
		std::unique_ptr<Tween<float>> tween = std::make_unique<Tween<float>>(renderer->scene.worlds[0]->position.z, 0.0f, 1.0f);
		tween->SetEase(Ease::EASE_OUT_CUBIC);
		tween->OnUpdate([this](float, float) {
			renderer->scene.worlds[0]->UpdateTransform();
			});
		tween->OnFinish([this](float) {
			animationFinished = true;
			});
		//update trasnform
		TweenManager::GetInstance().AddTween(std::move(tween));

		//start the point light animation
		float oldIntensity = renderer->pointLight->Intensity;
		renderer->pointLight->Intensity = 0.0f;
		std::unique_ptr<Tween<float>> lightTween = std::make_unique<Tween<float>>(renderer->pointLight->Intensity, oldIntensity, 2.0f);
		lightTween->SetEase(Ease::EASE_IN_QUINT);
		TweenManager::GetInstance().AddTween(std::move(lightTween));
		printf("Tween Finished\n");

		//move the point light to the center of the new level
		//float3 lightPos = currentLevel->Size / WORLDSIZE / 2.0f;
		//lightPos += float3(EPSILON * 10);

		//std::unique_ptr<Tween<float3>> lightPosTween = std::make_unique<Tween<float3>>(renderer->pointLight->position, lightPos, 2.0f);
		//lightPosTween->SetEase(Ease::EASE_IN_QUINT);
		//TweenManager::GetInstance().AddTween(std::move(lightPosTween));
	}
}

bool PuzzleScene::GenerateRandomLevel() {

	uint startSeed = seed;
	//set a seed for the random number generator
	srand(seed);

	std::shared_ptr<PuzzleLevel> emptyLevel = std::make_shared<PuzzleLevel>("Assets/levels/EmptyLevel.vox");
	LoadLevel(emptyLevel);
	isRandomLevel = true;
	//level size without the border
	int3 size = emptyLevel->Size - int3(2);

	std::vector<Lamp> lamps;

	int lampCount = 5;
	int antiLampCount = 5;

	for (int i = 0; i < lampCount; i++) {
		int3 position = int3(rand() % size.x, 0, rand() % size.z);
		Voxel voxel;
		voxel.position = position;
		voxel.color = 0xffffff;
		voxel.materialIndex = 0;
		voxel.colorIndex = 0;
		Lamp lamp;
		lamp.voxel = voxel;
		lamp.requiredRays = 4;
		lamp.isFound = false;
		lamps.push_back(lamp);
	}

	for (int i = 0; i < antiLampCount; i++) {
		int3 position = int3(rand() % size.x, 0, rand() % size.z);
		Voxel voxel;
		voxel.position = position;
		voxel.color = 1;
		voxel.materialIndex = 0;
		voxel.colorIndex = 0;
		Lamp lamp;
		lamp.voxel = voxel;
		lamp.requiredRays = 0;
		lamp.isFound = false;
		lamps.push_back(lamp);
	}

	//pick a random lamp position
	uint seedExtra = seed + 1;
	float2 pointLightSolutionPosition = float2(RandomFloat(seed) * size.x, RandomFloat(seedExtra) * size.z) / WORLDSIZE;
	renderer->pointLight->position.x = pointLightSolutionPosition.x;
	renderer->pointLight->position.z = pointLightSolutionPosition.y;

	//place the lamps and anti lamps
	for (const auto& lamp : lamps) {
		uint color = lamp.voxel.color;
		renderer->scene.worlds[0]->Set(lamp.voxel.position.x + 1, lamp.voxel.position.y + 1, lamp.voxel.position.z + 1, color);
	}


	//generate random walls
	int wallCount = 30;
	int wallSize = 2;

	std::vector<int3> wallPositions;

	std::vector<int> wallIndexesToRemove;
	for (int i = 0; i < wallCount; i++) {
		int3 position = int3(rand() % size.x, 0, rand() % size.z);
		for (int x = 0; x < wallSize; x++) {
			for (int z = 0; z < wallSize; z++) {
				int3 voxelPosition = position + int3(x, 0, z);
				//check if the wall is overlapping with a lamp or anti lamp
				for (const auto& lamp : lamps) {
					if (lamp.voxel.position == voxelPosition) {
						wallIndexesToRemove.push_back(i);
						break;
					}
				}

				if (std::find(wallIndexesToRemove.begin(), wallIndexesToRemove.end(), i) != wallIndexesToRemove.end()) {
					break;
				}

				wallPositions.push_back(position);
			}
		}
	}

	//place valid walls

	for (const auto& position : wallPositions) {
		for (int x = 0; x < wallSize; x++) {
			for (int z = 0; z < wallSize; z++) {
				for (int y = 0; y < size.y; y++) {
					int3 voxelPosition = position + int3(x, y, z);
					renderer->scene.worlds[0]->Set(voxelPosition.x + 1, voxelPosition.y + 1, voxelPosition.z + 1, 255);
				}
			}
		}
	}

	const float halfVoxelSize = VOXELSIZE / 2.0f;
	std::vector<float3> rayOffsets = {
	float3(halfVoxelSize, halfVoxelSize, halfVoxelSize),
	float3(-halfVoxelSize, halfVoxelSize, halfVoxelSize),
	float3(-halfVoxelSize, halfVoxelSize, -halfVoxelSize),
	float3(halfVoxelSize, halfVoxelSize, -halfVoxelSize)
	};
	//shoot rays from the lamps and anti lamps to the point light and check if they hit the point light
	//if they do, remove the lamp or anti lamp
	int successCount = 0;
	for (const auto& lamp : lamps) {
		int3 position = lamp.voxel.position + int3(1);
		float3 lampPosition = (float3(position) + float3(0.5f)) / WORLDSIZE;
		lampPosition = renderer->scene.worlds[0]->transform.TransformPoint(lampPosition);
		int count = 0;
		for (const auto& offset : rayOffsets) {
			float3 rayOrigin = lampPosition + offset + float3(0, EPSILON, 0);

			float3 rayDirection = normalize(renderer->pointLight->position - rayOrigin);
			float distance = length(renderer->pointLight->position - rayOrigin);
			Ray ray(rayOrigin, rayDirection, distance);

			bool result = !renderer->scene.IsOccluded(ray);

			if (result) {
				count++;
				renderer->DrawLine(rayOrigin, rayOrigin + rayDirection * distance, float3(0, 1, 0), true);
			} else {
				renderer->scene.FindNearest(ray);
				float3 hitPos = ray.IntersectionPoint();
				renderer->DrawLine(rayOrigin, hitPos, float3(1, 0, 0), true);
			}

			//draw a line from the lamp into the ray direction
		}


		if (count == lamp.requiredRays) {
			successCount++;


		}

		std::vector<Line> lines = VisualizeCube(lampPosition, VOXELSIZE, float3(1, 1, 1));
		if (count == lamp.requiredRays) {
			for (const auto& line : lines) {
				renderer->DrawLine(line.start, line.end, float3(0, 1, 0), true);
			}
		} else {
			for (const auto& line : lines) {
				renderer->DrawLine(line.start, line.end, float3(1, 0, 0), true);
			}
		}
	}

	if (successCount == lamps.size()) {
		printf("All lamps are visible\n");
		printf("Seed: %d\n", startSeed);
		return true;
	} else {
		printf("Seed: %d\n", startSeed);
		return false;
	}
}


//1459420682
//620932373
//-2001685688