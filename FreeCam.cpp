#include "precomp.h"
#include "SvenUtils/TweenManager.h"
void FreeCam::Init() {
	totalTime = 0.0f;
	wingsOffset = float3(2.0f, 1.0f, 2.0f);
	bulletSpeed = 5.0f;
	sunOffset = float3(5.0f, 1.0f, 5.0f);
	sunSpeed = 0.2f;
	renderer->sunLight->Intensity = 0.0f;
	renderer->areaLight->Intensity = 1.0f;
	renderer->camera.camSpeed = 0.9f;
	renderer->settings.repoBlendFactor = 0.9f;
	renderer->settings.repoDepthThreshold = 0.19f;

	renderer->scene.CLearWorlds();
	LoadVoxFile(renderer->scene, "assets/Earth.vox");
	earth = renderer->scene.worlds[0];
	earth->position = earth->GetSize() * -0.5f;
	earth->scale = float3(2.0f);
	earth->UpdateTransform();

	LoadVoxFile(renderer->scene, "assets/Wings.vox");
	wings = renderer->scene.worlds[1];
	wings->position = float3(1);
	wings->UpdateTransform();

	LoadVoxFile(renderer->scene, "assets/Bullet.vox");
	bulletPrefab = renderer->scene.worlds[2];
	bulletPrefab->position = float3(1000.0f);
	bulletPrefab->UpdateTransform();
	bulletPrefab->SetActive(false);

	LoadVoxFile(renderer->scene, "assets/Sun.vox");
	sun = renderer->scene.worlds[3];
	sun->position = float3(10, 10, 10);
	sun->scale = float3(2.0f);
	sun->UpdateTransform();

	renderer->settings.EnvironmentBuffer->LoadFromFile("assets/HDR_blue_nebulae-1.hdr");
	renderer->CanSelectWorld = false;
	renderer->settings.DebugLines = false;

	Material explosionMaterial;
	explosionMaterial.roughness = 1.0f;
	explosionMaterial.emissionIntensity = 100.0f;
	explosionMaterial.emissionColor = float3(0.3f, 0.0f, 0.0f);
	MaterialList.push_back(explosionMaterial);
	explosionMaterialIndex = static_cast<int>(MaterialList.size()) - 1;
}

void FreeCam::Update(float deltaTime) {
	totalTime += deltaTime;

	if (rotateObjects) {

		float3 prevPos = wings->position;
		wings->position = float3(cos(totalTime), sin(totalTime), sin(totalTime)) * wingsOffset;
		wings->rotation.y = atan2(wings->position.x - prevPos.x, wings->position.z - prevPos.z);
		wings->rotation.y += PI / 2.0f;
		//wings->rotation.z = (PI / 4) * sin(totalTime);
		//wings->rotation.x = (PI / 4) * cos(totalTime) * 2.0f;
		wings->UpdateTransformCentered();

		float sunTime = -totalTime * sunSpeed;
		sun->position = float3(cos(sunTime), sin(sunTime), sin(sunTime)) * sunOffset;
		sun->UpdateTransformCentered();

		float3 direction = normalize(earth->position - sun->position);
		renderer->areaLight->position = sun->position + direction * 1.0f;
		renderer->areaLight->rotation = direction;

		float3 desiredDirection = normalize(renderer->camera.camPos - float3(0.0f));
		float3 desiredTarget = renderer->camera.camPos - desiredDirection;

		float targetSmoothTime = 0.1f; // Time in seconds to smooth target

		//interpolate between original target and the new target
		renderer->camera.camTarget = lerp(renderer->camera.camTarget, desiredTarget, 1.0f - exp(-deltaTime / targetSmoothTime) * renderer->camera.camSpeed);

	}

	HanldeInput();
	HandleExplosions(deltaTime);
	TweenManager::GetInstance().Update(deltaTime);
}

void FreeCam::Render() {
	ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove);
	ImGui::BeginChild("Scrolling");
	if (!ImGui::CollapsingHeader("Free Cam settings"))  return;

	//wingsOffset
	ImGui::SliderFloat3("Wings offset", &wingsOffset.x, -5, 5);
	ImGui::SliderFloat3("Sun offset", &sunOffset.x, -5, 5);
	ImGui::SliderFloat("Sun speed", &sunSpeed, 0.1f, 10.0f);
	ImGui::SliderFloat("Bullet speed", &bulletSpeed, 0.1f, 10.0f);
	ImGui::SliderFloat("Explosion max size", &explosionMaxSize, 0.1f, 50.0f);
	ImGui::SliderFloat("Explosion probabilty", &explosionProbabilty, 0.0f, 1.0f);
	ImGui::Checkbox("Can shoot", &canShoot);
	ImGui::Checkbox("Rotate objects", &rotateObjects);

	//waves float sliders
	ImGui::SliderFloat("Wave 1", &wave1, 0.0f, 1.0f);
	ImGui::SliderFloat("Wave 2", &wave2, 0.0f, 1.0f);
	ImGui::SliderFloat("Wave 3", &wave3, 0.0f, 1.0f);

	ImGui::EndChild();
	ImGui::End();
}

void FreeCam::Exit() {
	TweenManager::GetInstance().Stop();
	renderer->scene.CLearWorlds();
	renderer->areaLight->Intensity = 0.0f;
}

void FreeCam::HanldeInput() {
	if (InputManager::GetInstance().GetKeyDown(KeyValues::ESCAPE)) {
		MenuScene* menu = new MenuScene();
		menu->renderer = renderer;
		renderer->sceneManager.LoadScene(menu);
	}

	//if spacebar
	if (InputManager::GetInstance().GetMouseButton(0)) {
		if (!canShoot) return;
		Ray ray = renderer->GetMouseRay();
		renderer->scene.FindNearest(ray);
		int worldIndex = ray.worldIndex;
		if (worldIndex == -1 || worldIndex >= 2 || ray.GetMaterialIndex() == explosionMaterialIndex) return;

		VoxelWorld* bullet = new VoxelWorld(*bulletPrefab);
		bullet->SetActive(true);
		bullet->position = renderer->camera.camPos + renderer->camera.GetForward() * 0.1f;
		bullet->rotation = renderer->camera.GetRotation();
		bullet->scale = float3(0.5f);
		bullet->UpdateTransform();
		renderer->scene.worlds.push_back(bullet);
		VoxelWorld* owner = bullet;

		float3 endPos = ray.IntersectionPoint();
		float3 localEndPos = ray.LocalIntersectionPoint();

		//make the duration so that the bullet will move at a constant speed of 5 unit per second
		float duration = length(endPos - bullet->position) / bulletSpeed;

		std::unique_ptr<Tween<float3>> tween = std::make_unique<Tween<float3>>(bullet->position, endPos, duration);
		tween->OnUpdate([bullet](float3 pos, float) {
			bullet->position = pos;
			bullet->UpdateTransformCentered();
			});
		tween->OnFinish([this, bullet, worldIndex, localEndPos, owner](float3) {
			bullet->SetActive(false);
			//remove bullet from scene
			explosions.push_back({ make_int3(localEndPos * WORLDSIZE), worldIndex, owner });
			renderer->scene.worlds.erase(std::remove(renderer->scene.worlds.begin(), renderer->scene.worlds.end(), bullet), renderer->scene.worlds.end());
			delete bullet;
			});
		TweenManager::GetInstance().AddTween(std::move(tween));
	}
}

void FreeCam::HandleExplosions(const float deltaTime) {
	for (int i = 0; i < explosions.size(); i++) {
		ExplosionLocation& explosion = explosions[i];
		//lerp size from 0 to explosionMaxSize over wave3 seconds
		float size = lerp(0.0f, explosionMaxSize, explosion.duration / wave3);

		explosion.duration += deltaTime;
		DrawSphere(renderer->scene, explosion.position, size, 0xffffff, 0, explosion.worldIndex, explosionProbabilty);
		if (explosion.duration > wave1 && explosion.duration < wave2) {
			DrawSphere(renderer->scene, explosion.position, size, 0xffff00, explosionMaterialIndex, explosion.worldIndex, 1.0f);
		} else if (explosion.duration > wave2 && explosion.duration < wave3) {
			DrawSphere(renderer->scene, explosion.position, size, 0xff0000, explosionMaterialIndex, explosion.worldIndex, explosionProbabilty);
		} else if (explosion.duration > wave3) {
			DrawSphere(renderer->scene, explosion.position, size, 0, explosionMaterialIndex, explosion.worldIndex, explosionProbabilty);
		}
	}

	//loop through all explosions and remove the ones that are done
	explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [this](ExplosionLocation& explosion) {
		return explosion.duration > wave3;
		}), explosions.end());
}
