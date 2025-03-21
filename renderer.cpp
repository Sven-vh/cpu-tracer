#include "precomp.h"
#include <SvenUtils/InputManager.h>
#include <SvenUtils/TweenUtils.h>
#include "DirectionalLight.h"
#include <PointLight.h>
#include <SpotLight.h>
#include "AreaLight.h"
#include <filesystem> // For directory iteration

// YOU GET:
// 1. A fast voxel renderer in plain C/C++
// 2. Normals and voxel colors
// FROM HERE, TASKS COULD BE:							FOR SUFFICIENT
// * Materials:
//   - Reflections and diffuse reflections				<===
//   - Transmission with Snell, Fresnel					<===
//   - Textures, Minecraft-style						<===
//   - Beer's Law
//   - Normal maps
//   - Emissive materials with postproc bloom
//   - Glossy reflections (BASIC)
//   - Glossy reflections (microfacet)
// * Light transport:
//   - Point lights										<===
//   - Spot lights										<===
//   - Area lights										<===
//	 - Sampling multiple lights with 1 ray
//   - Importance-sampling
//   - Image based lighting: sky
// * Camera:
//   - Depth of field									<===
//   - Anti-aliasing									<===
//   - Panini, fish-eye etc.
//   - Post-processing: now also chromatic				<===
//   - Spline cam, follow cam, fixed look-at cam
//   - Low-res cam with CRT shader
// * Scene:
//   - HDR skydome										<===
//   - Spheres											<===
//   - Smoke & trilinear interpolation
//   - Signed Distance Fields
//   - Voxel instances with transform
//   - Triangle meshes (with a BVH)
//   - High-res: nested grid
//   - Procedural art: shapes & colors
//   - Multi-threaded Perlin / Voronoi
// * Various:
//   - Object picking
//   - Ray-traced physics
//   - Profiling & optimization
// * GPU:
//   - GPU-side Perlin / Voronoi
//   - GPU rendering *not* allowed!
// * Advanced:
//   - Ambient occlusion
//   - Denoising for soft shadows
//   - Reprojection for AO / soft shadows
//   - Line lights, tube lights, ...
//   - Bilinear interpolation and MIP-mapping
// * Simple game:										
//   - 3D Arkanoid										<===
//   - 3D Snake?
//   - 3D Tank Wars for two players
//   - Chess
// REFERENCE IMAGES:
// https://www.rockpapershotgun.com/minecraft-ray-tracing
// https://assetsio.reedpopcdn.com/javaw_2019_04_20_23_52_16_879.png
// https://www.pcworld.com/wp-content/uploads/2023/04/618525e8fa47b149230.56951356-imagination-island-1-on-100838323-orig.jpg

extern Shader* shader;

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init() {
	// create fp32 rgb pixel buffer to render to
	reprojection = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	prevReprojection = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	memset(reprojection, 0, SCRWIDTH * SCRHEIGHT * 16);
	memset(prevReprojection, 0, SCRWIDTH * SCRHEIGHT * 16);
	// try to load a camera
	FILE* f = fopen("camera.bin", "rb");
	if (f) {
		fread(&camera, 1, sizeof(Camera), f);
		fclose(f);
	}
	//try to load a settings file
	FILE* settingsFile = fopen("settings.bin", "rb");
	if (settingsFile) {
		fread(&settings, 1, sizeof(Settings), settingsFile);
		fclose(settingsFile);
	}
	camera.aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	camera.UpdateProjection();

	//scene.GenerateGrid();

	sunLight = std::make_shared<DirectionalLight>(normalize(float3(0.58f, -0.63f, -0.53f)), float3(1, 1, 1), 0.2f);
	lights.push_back(sunLight);
	pointLight = std::make_shared<PointLight>(float3(0.13f, 0.08f, 0.2f), float3(1, 1, 1), 0.032f);
	pointLight->constantTerm = 5.0f;
	pointLight->linearTerm = 0.09f;
	pointLight->quadraticTerm = 0.032f;
	lights.push_back(pointLight);

	//const float3& position, const float2& size, const float3& rotation, const int& numSamples, const float3& color, float intensity
	areaLight = std::make_shared<AreaLight>(float3(0.0f, 2.0f, 0.0f), float2(1.0f, 1.0f), float3(0.0f, 0.0f, 0.0f), 1, float3(1.0f), 0.0f);
	areaLight->position = float3(-2.8f, 2.6f,-2.3f);
	areaLight->size = float2(0.5f, 0.5f);
	areaLight->rotation = float3(-2.09f, 1.78f, -1.37f);
	lights.push_back(areaLight);


	ambientLight = std::make_shared<Light>(float3(1.0f), 0.01f);
	lights.push_back(ambientLight);

	settings.EnvironmentBuffer = new FLoatSurface("assets/lonely_road_afternoon_8k.hdr");

	ball.position = float3(0.5, 2.0f, 0.5f);
	ball.velocity = float3(0, 0, 0);
	ball.radius = 0.1f;

	shader->Bind();
	shader->SetFloat("vignetteIntensity", settings.VignetteIntensity);
	shader->SetFloat("vignetteRadius", settings.VignetteRadius);
	shader->SetFloat("chromaAmount", settings.ChromaIntensity);
	shader->SetFloat("sigma", settings.Sigma);
	shader->SetFloat("kSigma", settings.KSigma);
	shader->SetFloat("threshold", settings.Threshold);
	shader->Unbind();
	scene.Clear(0);
#if 1
	MenuScene* puzzleScene = new MenuScene();
	puzzleScene->renderer = this;
	sceneManager.LoadScene(puzzleScene);
	sceneManager.Init();
#else
	PuzzleScene* puzzleScene = new PuzzleScene();
	puzzleScene->renderer = this;
	sceneManager.LoadScene(puzzleScene);
	sceneManager.Init();
#endif

	//Were going to do some perforamce testing here.
#if USE_SIMD
#endif

#if 0
	//randomize screen buffer
	for (int i = 0; i < SCRWIDTH * SCRHEIGHT; i++) {
		screen->pixels[i] = float4(RandomFloat(), RandomFloat(), RandomFloat(), 0);
	}

	const int numTests = 10'000;
	Timer t;
	for (int i = 0; i < numTests; i++) {
		float4 color = screen->pixels[0];
		ProcessScreen();
	}
	printf("Tonemapping 10k times took (without SIMD): %f seconds\n", t.elapsed());

	t.reset();
	for (int i = 0; i < numTests; i++) {
		ProcessScreenSIMD();
	}
	printf("Tonemapping 10k times took (with SIMD): %f seconds\n", t.elapsed());
#endif
#if 0
	printf("\n\n\n\n");

	const int numTests = 100'000'000;

	Ray ray;
	Timer t;
	for (int i = 0; i < numTests; i++) {
		ray = camera.GetNoEffectPrimaryRay(i, i);
	}
	printf("GetPrimaryRay took: %f seconds\n", t.elapsed());
	//print ray origin to avoid compiler optimization
	printf("Ray origin: %f, %f, %f\n", ray.O.x, ray.O.y, ray.O.z);

	printf("\n");

	Ray rays[8];
	t.reset();
	for (int i = 0; i < numTests; i += 8) {
		__m256 xVals = _mm256_set_ps(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
		__m256 yVals = _mm256_set_ps(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
		camera.GetPrimaryRaysSIMD(xVals, yVals, rays);
	}
	printf("GetPrimaryRays (With SIMD) took: %f seconds\n", t.elapsed());
	//print ray origin to avoid compiler optimization
	printf("Ray origin: %f, %f, %f\n", rays[0].O.x, rays[0].O.y, rays[0].O.z);


	printf("\n\n\n\n");
#endif
#if 0
	//cube intersection test
	float3 cubeMin = float3(0, 0, 0);
	float3 cubeMax = float3(1, 1, 1);
	float3 rayOrigin = float3(0.5, 0.5, 0.5);
	float3 rayDirection = float3(0, 0, -1);
	Ray r = Ray(rayOrigin, rayDirection);
	Cube cube = Cube(cubeMin, cubeMax);

	const int numTests = 100'000'000;

	float result = 0;
	Timer t;
	for (int i = 0; i < numTests; i++) {
		result = cube.Intersect(r);
	}
	printf("100M Cube intersections took: %f seconds\n", t.elapsed());
	//print result to avoid compiler optimization
	//printf("Result: %f\n", result);

	t.reset();
	for (int i = 0; i < numTests; i++) {
		result = cube.IntersectSIMD(r);
	}
	printf("100M Cube intersections (With SIMD) took: %f seconds\n", t.elapsed());
	//print result to avoid compiler optimization
	//printf("Result: %f\n", result);

#endif
#if 0
	// isn't actually faster
	//transform point test
	mat4 transform = mat4::Translate(float3(1, 1, 1));
	float3 point = float3(0, 0, 0);

	const int numTests = 100'000'000;

	float3 result = float3(0, 0, 0);
	Timer t;
	for (int i = 0; i < numTests; i++) {
		result = transform.TransformPoint(point);
	}
	printf("100M TransformPoint took: %f seconds\n", t.elapsed());
	//print result to avoid compiler optimization
	printf("Result: %f, %f, %f\n", result.x, result.y, result.z);

	t.reset();
	for (int i = 0; i < numTests; i++) {
		result = transform.TransformPointSIMD(point);
	}
	printf("100M TransformPoint (With SIMD) took: %f seconds\n", t.elapsed());
	//print result to avoid compiler optimization
	printf("Result: %f, %f, %f\n", result.x, result.y, result.z);
#endif
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime) {
	deltaTime /= 1000.0f;
	// pixel loop
	Timer t;


	//settings.fps is how many times per second we want to update the cellular automata
	if (settings.fps > 0) {
		const float targetFrameTime = 1.0f / settings.fps;
		if (time > targetFrameTime) {
			HandleCellularAutomata();
			time = 0;
		}
	}

	sceneManager.Update(deltaTime);

	if (IsInMenu) {
		screen->Clear(float3(0));
		InputManager::GetInstance().Update(deltaTime);
		return;
	}

	float deltaDistance = 0.0f;
	switch (renderState) {
	case PUZZLE:
		break;
	case CELLULAR_AUTOMATA:
		deltaDistance = camera.HandleInput(deltaTime);
		if (deltaDistance == 0.0f) {
			ResetAccumulation();
		}
		HandleCamera();
		HandleUserInput();

		break;
	case FREE_CAMERA:
		deltaDistance = camera.HandleInput(deltaTime);
		if (deltaDistance == 0.0f) {
			ResetAccumulation();
		}
		HandleCamera();
		HandleUserInput();
		break;
	default: break;
	}
	RenderScreen(deltaDistance);

	if (!CanSelectWorld) {
		selectedWorld = -1;
	}

	DebugDraw();
	FlushLines(deltaTime);

	InputManager::GetInstance().Update(deltaTime);

	camera.UpdatePrevState();
	//PerformanceReport(t);
	time += deltaTime;
}

void Tmpl8::Renderer::UpdateBallPhysics(float deltaTime) {
	float3 ballPosition = ball.position;
	float3 totalForce = float3(0, 0, 0); // Initialize a vector to accumulate forces from all directions

	int rays = 100; // Total number of rays
	float deltaTheta = 2.0f * PI / rays; // Azimuth angle step (in radians)
	float deltaPhi = PI / (rays - 1.0f); // Inclination angle step (in radians), adjusted for full coverage

	for (int i = 0; i < rays; ++i) {
		for (int j = 0; j < rays; ++j) {
			// Calculate spherical coordinates
			float theta = i * deltaTheta;
			float phi = j * deltaPhi - (PI / 2.0f); // Offset by PI/2 to start from the top

			// Convert spherical coordinates to Cartesian coordinates for the ray direction
			float3 rayDir = float3(
				cos(phi) * sin(theta), // x
				sin(phi),              // y
				cos(phi) * cos(theta)  // z
			);

			// Create and cast the ray
			Ray r = Ray(ballPosition, rayDir);
			scene.FindNearest(r);
			if (r.voxel != 0) {
				float3 intersectionPoint = r.IntersectionPoint();
				float distance = length(intersectionPoint - ballPosition);

				// Check if the ray intersected within the ball's radius
				if (distance < ball.radius) {
					// Reflect the ray direction based on the surface normal at the intersection point
					float3 normal = r.GetNormal();
					float3 reflection = reflect(rayDir, normal);

					// Accumulate the reflection force
					totalForce += reflection * (ball.radius - distance);
				}
			}
		}
	}
	// Gravity vector (assuming the negative y-direction is downward)
	const float3 gravity = float3(0, -9.81f, 0); // Adjust the magnitude as necessary

	// Apply gravity to the ball's velocity
	ball.velocity += gravity * (deltaTime / 1000.0f);

	// Calculate reflection forces from rays (as previously described)
	// Ensure totalForce is not a zero vector before normalizing
	if (length(totalForce) > 0.0f) {
		float3 normalizedForce = normalize(totalForce);
		float magnitude = 0.01f; // Adjust this magnitude as necessary
		ball.velocity += normalizedForce * magnitude;
	}

	// Update the ball's position based on its velocity
	ball.position += ball.velocity * (deltaTime / 1000.0f);

	// Collision detection with the ground
	float groundLevel = 0.0f; // Adjust this to your scene's ground level
	if (ball.position.y < groundLevel + ball.radius) {
		ball.position.y = groundLevel + ball.radius; // Adjust the ball's position to sit on the ground
		ball.velocity.y *= -0.9f; // Reflect the y-velocity to simulate a bounce, adjust the coefficient as necessary
	}

	// Optionally, apply damping to the velocity
	ball.velocity *= 0.99f; // Adjust the damping factor as necessary
}

void Renderer::RenderScreen(const float cameraDistance) {

	if (frameIndex == 1) memset(reprojection, 0, SCRWIDTH * SCRHEIGHT * 16);


	SetCamSettings();
	bool wantsToDebug = false;
	if (InputManager::GetInstance().GetKeyDown(KeyValues::I)) {
		wantsToDebug = true;
	}
	Vector2 mousePos = InputManager::GetInstance().GetMousePosition();
	int2 mousePosInt = int2(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));

	float floatFrameIndex = static_cast<float>(frameIndex);

	const ToneMappingType toneMapping = settings.ToneMapping;
	const float exposure = settings.Exposure;
	const float blendFactor = settings.repoBlendFactor;
	const float depthThreshold = settings.repoDepthThreshold;
	printf("Camera depth delta: %f\n", cameraDistance);

	const bool accumulation = settings.Accumulate;
	const bool reprojectionEnabled = settings.Reprojection;
	const bool antiAliasing = settings.AntiAliasing;
	const bool jitter = settings.Jitter;

#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			//using above bools to determine what to do with the pixel

			const int index = x + y * SCRWIDTH;

			if (wantsToDebug) {
				if (x == mousePosInt.x && y == mousePosInt.y) {
					printf("break here\n");
				}
			}

			PixelInfo currentPixel(x, y);

			if (reprojectionEnabled) {
				NoEffect(currentPixel);
				// far away pixels are not reprojected to avoid ghosting/motion blur with the reprojection
				if (currentPixel.ray.t > 5.0f) {
					float4 color = ToneMapping(float4(currentPixel.color, 0), exposure, toneMapping);
					SetScreen(color, index);
					continue;
				}
			} else if (antiAliasing) {
				ApplyAntiAliasing(currentPixel);
			} else if (jitter) {
				ApplyJitter(currentPixel);
			} else {
				NoEffect(currentPixel);
			}

			//clamp the color to avoid fireflies
			if (dot(currentPixel.color, currentPixel.color) > 9) {
				currentPixel.color = 3.0f * normalize(currentPixel.color);
			}

			float4 avg;
			if (reprojectionEnabled) {
				avg = ApplyReprojection(currentPixel, cameraDistance, depthThreshold, blendFactor);
				reprojection[index] = float4(avg, currentPixel.depth);
			} else if (accumulation) {
				reprojection[index] += float4(currentPixel.color, currentPixel.depth);
				avg = reprojection[index] / floatFrameIndex;
			} else {
				avg = float4(currentPixel.color, currentPixel.depth);
			}

			float4 color = ToneMapping(avg, exposure, toneMapping);
			SetScreen(color, index);
		}
	}

	//ProcessScreenSIMD();

	if (settings.Accumulate) {
		frameIndex++;
	} else {
		ResetAccumulation();
	}

	if (reprojectionEnabled) {
		//swap the buffers
		float4* temp = prevReprojection;
		prevReprojection = reprojection;
		reprojection = temp;
	}
}

void Tmpl8::Renderer::ProcessScreen() {
#if HDR
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			int index = x + y * SCRWIDTH;
			float4 color = screen->pixels[index];
			color = color / (color + float4(1.0f)) * settings.Exposure;
			screen->pixels[index] = color;
		}
	}
#endif
}

void Tmpl8::Renderer::ProcessScreenSIMD() {
#if HDR
	const float exposure = settings.Exposure;
	__m256 one_vec = _mm256_set1_ps(1.0f);
	__m256 exposure_vec = _mm256_set1_ps(exposure);

#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x += 8) {
			const int index = x + y * SCRWIDTH;

			// Load pixel data into temporary arrays because of AoS format
			float tempR[8], tempG[8], tempB[8];
			for (int i = 0; i < 8; i++) {
				tempR[i] = screen->pixels[index + i].x;
				tempG[i] = screen->pixels[index + i].y;
				tempB[i] = screen->pixels[index + i].z;
			}

			// Load data into __m256 registers (SoA format now)
			__m256 color_r = _mm256_load_ps(tempR);
			__m256 color_g = _mm256_load_ps(tempG);
			__m256 color_b = _mm256_load_ps(tempB);

			//reinhard tonemapping
			__m256 blend_factor_r = _mm256_div_ps(exposure_vec, _mm256_add_ps(color_r, one_vec));
			color_r = _mm256_fmadd_ps(color_r, blend_factor_r, color_r);
			__m256 blend_factor_g = _mm256_div_ps(exposure_vec, _mm256_add_ps(color_g, one_vec));
			color_g = _mm256_fmadd_ps(color_g, blend_factor_g, color_g);
			__m256 blend_factor_b = _mm256_div_ps(exposure_vec, _mm256_add_ps(color_b, one_vec));
			color_b = _mm256_fmadd_ps(color_b, blend_factor_b, color_b);

			// Store the data back to temporary arrays
			_mm256_store_ps(tempR, color_r);
			_mm256_store_ps(tempG, color_g);
			_mm256_store_ps(tempB, color_b);

			// Store back from temporary arrays to the pixel data
			for (int i = 0; i < 8; i++) {
				screen->pixels[index + i].x = tempR[i];
				screen->pixels[index + i].y = tempG[i];
				screen->pixels[index + i].z = tempB[i];
			}
		}
	}
#endif
}


inline float4 Tmpl8::Renderer::ApplyReprojection(PixelInfo& currentPixel, const float& cameraDepthDelta, const float& depthThreshold, const float& blendFactor) const {
	float3 worldPos = currentPixel.ray.IntersectionPoint();
	int2 reprojected = camera.Reproject(worldPos);

	bool keepSample = reprojected.x < SCRWIDTH && reprojected.y < SCRHEIGHT && reprojected.x > 0 && reprojected.y > 0;

	if (keepSample) {
		// Clamp the base reprojected coordinates to the framebuffer bounds
		int2 clampedBase = make_int2(max(0, min(reprojected.x, SCRWIDTH - 2)), max(0, min(reprojected.y, SCRHEIGHT - 2)));

		// Calculate the fractional part of the reprojected coordinates
		float2 fractional = make_float2(static_cast<float>(reprojected.x) - floorf(static_cast<float>(reprojected.x)), static_cast<float>(reprojected.y) - floorf(static_cast<float>(reprojected.y)));

		// Determine the indices of the four nearest pixels using clamped base coordinates
		int2 base = clampedBase;
		int2 baseTopRight = make_int2(base.x + 1, base.y);
		int2 baseBottomLeft = make_int2(base.x, base.y + 1);
		int2 baseBottomRight = make_int2(base.x + 1, base.y + 1);

		// Fetch the colors of the four nearest pixels
		float4 colorBottomLeft = prevReprojection[baseBottomLeft.x + baseBottomLeft.y * SCRWIDTH];
		float4 colorBottomRight = prevReprojection[baseBottomRight.x + baseBottomRight.y * SCRWIDTH];
		float4 colorTopLeft = prevReprojection[base.x + base.y * SCRWIDTH];
		float4 colorTopRight = prevReprojection[baseTopRight.x + baseTopRight.y * SCRWIDTH];

		// Calculate the weights for bilinear interpolation
		float weightTopLeft = (1 - fractional.x) * (1 - fractional.y);
		float weightTopRight = fractional.x * (1 - fractional.y);
		float weightBottomLeft = (1 - fractional.x) * fractional.y;
		float weightBottomRight = fractional.x * fractional.y;

		// Interpolate the colors based on the weights
		float4 interpolatedColor = weightTopLeft * colorTopLeft +
			weightTopRight * colorTopRight +
			weightBottomLeft * colorBottomLeft +
			weightBottomRight * colorBottomRight;

		// Use the interpolated color instead of directly using prev
		float depthDifference = fabs(interpolatedColor.w - (currentPixel.depth + cameraDepthDelta));
		if (depthDifference < depthThreshold) {
			float confidence = max(blendFactor - (depthDifference * 3.0f), 0.0f);
			return confidence * interpolatedColor + (1.0f - confidence) * currentPixel.color;
			//return depthDifference;
		}
	}

	return currentPixel.color;
}

void Tmpl8::Renderer::NoEffect(PixelInfo& currentPixel) const {
	currentPixel.ray = camera.GetPrimaryRay((float)currentPixel.x, (float)currentPixel.y);
	Trace(currentPixel);
	return;
}

void Renderer::ApplyAntiAliasing(PixelInfo& currentPixel) const {
	// Supersampling anti-aliasing with 2x2 grid
	int antiAliasingSamples = settings.AntiAliasingSamples;
	int gridSide = static_cast<int>(sqrt(antiAliasingSamples));
	float4 pixel = float4(0, 0, 0, 0);
	for (int sy = 0; sy < gridSide; sy++) {
		for (int sx = 0; sx < gridSide; sx++) {
			float offsetX = (sx + 0.5f) / 2 - 0.5f;
			float offsetY = (sy + 0.5f) / 2 - 0.5f;
			currentPixel.ray = camera.GetPrimaryRay((float)currentPixel.x + offsetX, (float)currentPixel.y + offsetY);
			Trace(currentPixel);
			pixel += currentPixel.color;
		}
	}
	currentPixel.color = pixel / static_cast<float>(antiAliasingSamples);
}

void Renderer::ApplyJitter(PixelInfo& currentPixel) const {
	float jitterX = (RandomFloat() - 0.5f);
	float jitterY = (RandomFloat() - 0.5f);

	float subPixelX = currentPixel.x + jitterX;
	float subPixelY = currentPixel.y + jitterY;

	currentPixel.ray = camera.GetPrimaryRay(subPixelX, subPixelY);

	Trace(currentPixel);
	return;
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
void Renderer::Trace(PixelInfo& currentPixel) const {
	if (settings.PathTracing) {
		currentPixel.color = PerformPathTracing(currentPixel, 0);
		return;
	}
	if (settings.StepThrough) {
		scene.FindNearest(currentPixel.ray);

		if (currentPixel.ray.steps == 0) {
			currentPixel.color = GetEnvironmentLight(currentPixel.ray);
			return;
		}
		static const float maxSteps = sqrtf(WORLDSIZE2 + WORLDSIZE2 + WORLDSIZE2) * 2.0f;

		currentPixel.color = float3(currentPixel.ray.steps / maxSteps);
		return;
	}

	if (settings.Normals) {
		scene.FindNearest(currentPixel.ray);
		if (currentPixel.ray.voxel == 0) {
			currentPixel.color = GetEnvironmentLight(currentPixel.ray);
			return;
		}
		currentPixel.color = (currentPixel.ray.GetNormal() + 1) * 0.5f;
		return;
	}

	if (settings.UV) {
		scene.FindNearest(currentPixel.ray);
		if (currentPixel.ray.voxel == 0) {
			currentPixel.color = GetEnvironmentLight(currentPixel.ray);
		}

		float2 uv = currentPixel.ray.GetUV();
		currentPixel.color = float3(uv.x, uv.y, 0);
		return;
	}
	currentPixel.color = PerformSimpleRendering(currentPixel.ray);
	return;
}


float3 Renderer::PerformPathTracing(PixelInfo& currentPixel, int depth) const {
	scene.FindNearest(currentPixel.ray);

	float3 floorColor;
	if (settings.RenderFloor && IsLookingAtFloor(currentPixel.ray, floorColor)) {
		return floorColor;
	}

	if (currentPixel.ray.voxel == 0) return GetEnvironmentLight(currentPixel.ray);
	if (depth > settings.PathTracingMaxDepth) return float3(0);

	// Russian Roulette termination
	if (depth > settings.MinDepthRussiaRoulette && RandomFloat() < settings.RussianRouletteThreshold) {
		return float3(0); // Terminate the path early
	}

	const Material& material = currentPixel.ray.GetMaterial();
	const float3 intersectionPoint = currentPixel.ray.IntersectionPoint();
	const float3 normal = currentPixel.ray.GetNormal();
	currentPixel.depth = length(intersectionPoint - camera.camPos);
	const float3 albedo = currentPixel.ray.GetAlbedo();
	const float3 emission = material.GetEmission();
	const float3 viewDir = -currentPixel.ray.D;

	float reflectivity = material.GetReflectivity(viewDir, normal);
	const float transparency = material.transparency;
	const float diffuseness = 1 - reflectivity - transparency;
	const float metallic = material.metallic;

	float3 outRadiance = emission;

	// Adjust reflectivity for metallic materials
	if (metallic > 0) {
		reflectivity = lerp(reflectivity, 1.0f, metallic); // Linearly interpolate reflectivity based on metallic value
	}

	// Reflection
	if (reflectivity > 0) {
		const float3 reflectedDir = reflect(currentPixel.ray.D, normal);
		const float3 reflectionColor = lerp(float3(1), albedo, metallic); // Reflective color is influenced by albedo for metals

		PixelInfo reflectedPixel;
		reflectedPixel.ray = Ray(intersectionPoint + normal * EPSILON, reflectedDir);

		outRadiance += reflectivity * reflectionColor * PerformPathTracing(reflectedPixel, depth + 1);
	}

	// Refraction/Transmission
	if (transparency > 0) {
		const float3 transmittedColor = material.GetTransmittedColor(albedo, currentPixel.ray.t);
		//if (!transmittedColor.isZero()) { // Check if transmission is possible
		const float3 refractedDir = refract(currentPixel.ray.D, normal, material.ior);

		//find the position where we want to start the next ray
		Ray refractedRay(intersectionPoint + refractedDir * EPSILON, refractedDir);
		scene.FindNearestEmpty(refractedRay);

		const float3 nextPos = refractedRay.IntersectionPoint();

		PixelInfo nextPixel;
		nextPixel.ray = Ray(nextPos + refractedDir * EPSILON, refractedDir);
		outRadiance += transmittedColor * PerformPathTracing(nextPixel, depth + 1);
		//}
	}

	// Diffuse
	if (diffuseness > 0 && metallic < 1) { // Reduce or eliminate diffuse component for metals
		const float3 irradiance = CalculateDirectLighting(currentPixel.ray, intersectionPoint, normal);
		const float3 brdf = albedo * INVPI;
		outRadiance += (1 - metallic) * diffuseness * brdf * irradiance; // Scale diffuse contribution by (1 - metallic)

		// Indirect diffuse lighting, also scaled by (1 - metallic)
		if (depth < settings.PathTracingMaxDepth - 1) {
			const float3 sampleDirection = RandomDirectionInHemisphere(normal);

			PixelInfo indirectPixel;
			indirectPixel.ray = Ray(intersectionPoint + sampleDirection * EPSILON, sampleDirection);

			outRadiance += (1 - metallic) * diffuseness * brdf * PerformPathTracing(indirectPixel, depth + 1);
		}
	}
	// Apply Russian Roulette continuation probability adjustment if needed
	if (depth > settings.MinDepthRussiaRoulette) {
		outRadiance /= (1 - settings.RussianRouletteThreshold);
	}

	return outRadiance;
}

float3 Renderer::PerformSimpleRendering(Ray& ray) const {
	scene.FindNearest(ray);
	if (ray.voxel == 0) {
		return GetEnvironmentLight(ray);
	}

	float3 I = ray.IntersectionPoint();
	float3 N = ray.GetNormal();
	float3 albedo = ray.GetAlbedo();

	float3 directLighting = CalculateDirectLighting(ray, I, N);
	return directLighting * albedo;
}


float3 Renderer::CalculateDirectLighting(const Ray& ray, const float3& I, const float3& N) const {
	float3 color(0);

#if 0

	const int totalLights = lights.size();
	float totalLightIntensity = 0;

	// Compute total light intensity
	for (const auto& light : lights) {
		totalLightIntensity += light->GetIntensity(I);
	}

	int effectiveLights = std::min(5, totalLights); // Ensure we do not exceed the total number of lights

	// Stratified sampling: divide the total intensity range into 'effectiveLights' strata
	float stratumSize = totalLightIntensity / effectiveLights;

	// Vector to store selected lights for contribution calculation
	std::vector<std::shared_ptr<Light>> selectedLights;
	selectedLights.reserve(effectiveLights);

	// Perform stratified sampling
	for (int i = 0; i < effectiveLights; ++i) {
		float stratumStart = i * stratumSize;
		float stratumEnd = stratumStart + stratumSize;
		float r = RandomFloat() * stratumSize + stratumStart; // Random value within the stratum
		float cumulativeIntensity = 0.0f;

		for (const auto& light : lights) {
			cumulativeIntensity += light->GetIntensity(I);
			if (cumulativeIntensity >= r && cumulativeIntensity <= stratumEnd) {
				selectedLights.push_back(light);
				break; // Break the inner loop once the light is selected
			}
		}
	}

	// Compute contribution of selected lights
	for (const auto& light : selectedLights) {
		color += light->GetContribution(scene, ray, I, N);
	}

	return color;

#else

	for (const auto& light : lights) {
		if (light->GetIntensity() == 0) continue;
		color += light->GetContribution(scene, ray, I, N);
	}
	return color;
#endif
}

// -----------------------------------------------------------
// Update user interface (img1)
// -----------------------------------------------------------
void Renderer::UI() {
	sceneManager.Render();
	if (IsInMenu) return;
	DrawSidebar();
	DrawWorldSettingsImguiWindow();
}

void Tmpl8::Renderer::DrawSidebar() {
	// Set the sidebar to be at the left edge of the window and span the full height
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImVec2 window_pos = ImVec2(0, 0); // Top left corner of the application window
	ImVec2 window_pos_pivot = ImVec2(0, 0);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver, window_pos_pivot);

	// Set only the height of the window, allowing the width to be adjustable
	ImVec2 window_size = ImVec2(0, ImGui::GetIO().DisplaySize.y); // Width is 0, which means it will not be fixed
	ImGui::SetNextWindowSizeConstraints(ImVec2(100, window_size.y), ImVec2(FLT_MAX, window_size.y)); // Minimum width can be set here, and height is fixed

	// Begin the sidebar window with NoResize and NoMove flags to keep it fixed
	ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove);

	// Scrollable region
	ImGui::BeginChild("Scrolling");
	if (ImGui::CollapsingHeader("Renderer settings")) {
		bool changed = false;
		DrawBasicInfoImguiWindow();
		DrawCameraImguiWindow();
		DrawShaderImguiWindow();
		DrawLightsImguiWindow();
		changed |= settings.DrawSettingsWindow();
		DrawWorldImguiWindow();
		DrawCellularAutomataImguiWindow();
		if (changed) ResetAccumulation();
	}

	// End scrollable region
	ImGui::EndChild();
	ImGui::End();
}

void Renderer::DrawBasicInfoImguiWindow() {
	// collasping header for basic info
	if (!ImGui::CollapsingHeader("Basic Info")) return;
	Ray r = GetMouseRay();
	scene.FindNearest(r);
	//fps and ms
	ImGui::Text("FPS: %f", fps);
	ImGui::Text("MS: %f", ms);
	ImGui::Text("voxel: %i", r.voxel);
	ImGui::Text("voxel: %i", r.index);
	ImGui::Text("world: %i", r.worldIndex);
	int materialIndex = r.voxel >> 24;
	ImGui::Text("Material Index: %i", materialIndex);
	ImGui::Text("Steps: %i", r.steps);
	ImGui::Text("Distance: %f", r.t);

	//intersection point
	float3 intersectionPoint = r.IntersectionPoint();
	ImGui::Text("Intersection Point: %f, %f, %f", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);

	Material& currentMaterial = MaterialList[materialIndex];
	//imgu itext for all the material properties
	ImGui::Text("Material Properties");
	ImGui::Text("Emission: %f, %f, %f", currentMaterial.GetEmission().x, currentMaterial.GetEmission().y, currentMaterial.GetEmission().z);
	ImGui::Text("Roughness: %f", currentMaterial.roughness);
	ImGui::Text("Metallic: %f", currentMaterial.metallic);
	ImGui::Text("Transparency: %f", currentMaterial.transparency);
	ImGui::Text("IOR: %f", currentMaterial.ior);
	//refractivity and reflectivity
	ImGui::Text("Refractivity: %f", currentMaterial.GetRefractivity());
	ImGui::Text("Reflectivity: %f", currentMaterial.GetReflectivity(r.D, r.GetNormal()));
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void Tmpl8::Renderer::DrawCameraImguiWindow() {
	//collapsing header for camera
	if (!ImGui::CollapsingHeader("Camera")) return;
	bool changed = false;
	changed |= ImGui::DragFloat("Camera Speed", &camera.camSpeed, 0.1f);
	changed |= ImGui::DragFloat3("Camera Position", &camera.camPos.x);
	changed |= ImGui::DragFloat3("Camera Target", &camera.camTarget.x);
	if (ImGui::CollapsingHeader("viewMatrix")) {
		changed |= MatrixProperties("View Matrix", camera.GetViewMatrix());
	}
	if (ImGui::CollapsingHeader("projectionMatrix")) {
		changed |= MatrixProperties("Projection Matrix", camera.GetProjectionMatrix());
	}
	changed |= camera.DrawImgui();
	if (camera.depthOfField) {
		if (ImGui::Checkbox("Focus Center", &settings.FocusCenter)) {
			if (settings.FocusCenter) settings.FocusMouse = false;
			changed = true;
		}
		if (ImGui::Checkbox("Focus Mouse", &settings.FocusMouse)) {
			if (settings.FocusMouse) settings.FocusCenter = false;
			changed = true;
		}
	}
	if (ImGui::Button("Reset position")) {
		camera.camPos = float3(0, 0, 0);
		camera.camTarget = float3(0, 0, 1);
		changed = true;
	}

	if (changed) ResetAccumulation();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void Renderer::DrawShaderImguiWindow() {
	//settings.VignetteIntensity = 0.5f;
	//settings.VignetteRadius = 0.5f;
	//settings.ChromaAmount = 0;
	//make sliders for all the settings
	//collapsing header for shader
	if (!ImGui::CollapsingHeader("Shader")) return;
	bool changed = false;
	changed |= ImGui::SliderFloat("Vignette Intensity", &settings.VignetteIntensity, 0, 1);
	changed |= ImGui::SliderFloat("Vignette Radius", &settings.VignetteRadius, 0, 1);
	changed |= ImGui::SliderFloat("Chroma Amount", &settings.ChromaIntensity, 0, 0.1f);
	changed |= ImGui::DragFloat("Sigma", &settings.Sigma, 0.01f);
	changed |= ImGui::DragFloat("KSigma", &settings.KSigma, 0.01f);
	changed |= ImGui::DragFloat("Threshold", &settings.Threshold, 0.01f);


	if (changed) {
		shader->Bind();
		shader->SetFloat("vignetteIntensity", settings.VignetteIntensity);
		shader->SetFloat("vignetteRadius", settings.VignetteRadius);
		shader->SetFloat("chromaAmount", settings.ChromaIntensity);
		shader->SetFloat("sigma", settings.Sigma);
		shader->SetFloat("kSigma", settings.KSigma);
		shader->SetFloat("threshold", settings.Threshold);
		shader->Unbind();
	}
}

void Renderer::DrawWorldImguiWindow() {
	// Collapsing header for world
	if (!ImGui::CollapsingHeader("World")) return;
	ImGui::Checkbox("Can select World", &CanSelectWorld);
	bool changed = false;
	changed |= ImGui::Checkbox("Render Floor", &settings.RenderFloor);
	changed |= settings.Floor.DrawImgui(1234);
	changed |= scene.DrawImGui();

	if (changed) ResetAccumulation();
}

void Tmpl8::Renderer::DrawCellularAutomataImguiWindow() {
	if (!ImGui::CollapsingHeader("Cellular Automata")) return;
	bool changed = false;

	ImGui::SliderFloat("probability", &settings.probablity, 0, 1);
	//radius
	ImGui::DragInt("Radius", &settings.radius, 1, 0, WORLDSIZE);
	if (ImGui::Button("Randomize")) {
		changed = true;
		RandomizeCellularAutomata();
	}

	if (ImGui::Button("iterate")) {
		changed = true;
		HandleCellularAutomata();
	}

	if (ImGui::Button("Clear")) {
		changed = true;
		ClearCellularAutomata();
	}

	//dropdown for checkboxes

	if (ImGui::CollapsingHeader("Survival Settings")) {
		for (int i = 0; i < 26; i++) {
			std::string label = "Survival Rule " + std::to_string(i + 1);
			changed |= ImGui::Checkbox(label.c_str(), &settings.Survival[i]);
		}
	}

	if (ImGui::CollapsingHeader("Spawn Settings")) {
		for (int i = 0; i < 26; i++) {
			std::string label = "SPawn Rule " + std::to_string(i + 1);
			changed |= ImGui::Checkbox(label.c_str(), &settings.Spawn[i]);
		}
	}

	ImGui::DragInt("StartState", &settings.StartState, 1, 1, 10);
	ImGui::DragFloat("ticks per second", &settings.fps, 1, 0, 100);

	return;
}

void Renderer::DrawLightsImguiWindow() {
	//collapsing header for lights
	if (!ImGui::CollapsingHeader("Lights")) return;
	bool changed = false;
	if (ImGui::Button("Add Directional Light")) {
		float3 direction = normalize(camera.camTarget - camera.camPos);
		lights.push_back(std::make_shared<DirectionalLight>(direction, float3(1), 1.0f));
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Ambient Light")) {
		lights.push_back(std::make_shared<Light>(float3(1), 0.25f));
		changed = true;
	}

	//point light
	if (ImGui::Button("Add Point Light")) {
		lights.push_back(std::make_shared<PointLight>(camera.camPos, float3(1), 1.0f));
		changed = true;
	}
	ImGui::SameLine();
	//spot light
	if (ImGui::Button("Add Spot Light")) {
		float3 direction = normalize(camera.camTarget - camera.camPos);
		lights.push_back(std::make_shared<SpotLight>(camera.camPos, direction, float3(1), 1.0f, 1.0f, 30.0f));
		changed = true;
	}

	//area light
	if (ImGui::Button("Add Area Light")) {
		lights.push_back(std::make_shared<AreaLight>(camera.camPos, float2(1.0f, 1.0f), camera.camTarget, 1, float3(1.0f), 1.0f));
		changed = true;
	}

	ImGui::Separator();
	int i = 0;
	int indexToRemove = -1;
	for (auto& light : lights) {
		bool isSunLight = light == sunLight;
		if (isSunLight) {
			ImGui::Text("Sun Light");
		}
		if (light->DrawImgui(i++)) {
			changed = true;
		}

		//button to remove light
		if (!isSunLight) {
			std::string removeLabel = "Remove##" + std::to_string(i);
			if (ImGui::Button(removeLabel.c_str())) {
				indexToRemove = i;
			}
		}
		ImGui::Separator();
	}
	if (indexToRemove != -1) {
		lights.erase(lights.begin() + indexToRemove - 1);
		changed = true;
	}

	if (changed) ResetAccumulation();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

void Renderer::DrawWorldSettingsImguiWindow() {
	if (selectedWorld == -1) return;

	//Draw a window at the top right of the screen that shows the settings for the selected world
	float padding = 0;
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - padding, padding);
	ImVec2 pivot = ImVec2(1, 0);
	ImGui::SetNextWindowPos(window_pos, 0, pivot);

	//dont save the size of the window
	//ImGui::SetNextWindowSizeConstraints(ImVec2(300, 500), ImVec2(300, 500));

	std::string title = "World Settings##" + std::to_string(selectedWorld);
	ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	//get the world
	VoxelWorld* world = scene.worlds[selectedWorld];
	bool changed = false;


	changed |= world->DrawImGui(0);
	std::string deleteLabel = "Delete World##" + std::to_string(selectedWorld);
	if (ImGui::Button(deleteLabel.c_str())) {
		scene.worlds.erase(scene.worlds.begin() + selectedWorld);
		selectedWorld = -1;
		changed = true;
	}
	if (ImGui::BeginTabBar("##TabBar", ImGuiTabBarFlags_FittingPolicyScroll)) {

		if (ImGui::BeginTabItem("Models")) {


			// Read all the .vox files in the assets folder and store them in a vector
			std::vector<std::string> voxFiles;
			for (const auto& entry : std::filesystem::directory_iterator("assets")) {
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

				ImGui::Combo("Vox Files", &selectedItem, &cStrVoxFiles[0], static_cast<int>(cStrVoxFiles.size()) - 1);
			}

			// Position input for the .vox file
			ImGui::DragFloat3("Vox Position", &settings.VoxelFilePosition.x, VOXELSIZE);

			if (ImGui::Button("Draw Vox File") && selectedItem >= 0) {
				// Load the selected .vox file
				std::string path = "assets/" + voxFiles[selectedItem];
				DrawVoxFile(scene, path.c_str(), settings.VoxelFilePosition, selectedWorld);
				changed = true;
			}

			if (ImGui::Button("Load Vox File") && selectedItem >= 0) {
				// Load the selected .vox file
				std::string path = "assets/" + voxFiles[selectedItem];
				LoadVoxFile(scene, path.c_str());
				changed = true;
			}

		}
	}
	if (changed) ResetAccumulation();
	ImGui::End();
}

// -----------------------------------------------------------
// User wants to close down
// -----------------------------------------------------------
void Renderer::Shutdown() {

	sceneManager.Destroy();

	// save current camera
	FILE* f = fopen("camera.bin", "wb");
	fwrite(&camera, 1, sizeof(Camera), f);
	fclose(f);

	//save settings
	FILE* settingsFile = fopen("settings.bin", "wb");
	fwrite(&settings, 1, sizeof(Settings), settingsFile);
	fclose(settingsFile);
}

void Renderer::HandleUserInput() {

	if (InputManager::GetInstance().GetMouseButton(0)) {
		//shoot ray and call a trace function
		Ray r = GetMouseRay();
		PixelInfo currentPixel;
		currentPixel.ray = r;

		scene.FindNearest(r);
		float4 color = PerformPathTracing(currentPixel, 0);
	}

	if (!InputManager::GetInstance().GetMouseButton(0)) return;
	if (settings.DrawMode) {
		PlaceShapeAtMouse();
		return;
	}
	SelectWorld();
}

Ray Renderer::GetMouseRay() const {
	Vector2 mousePos = InputManager::GetInstance().GetMousePosition();
	Ray r = camera.GetNoEffectPrimaryRay((float)mousePos.x, (float)mousePos.y);
	return r;
}

void Renderer::DebugDraw() {
	//if (!settings.DebugDraw) return;

	int index = 0;
	for (auto& world : scene.worlds) {

		if (index++ != selectedWorld) continue;
		std::vector<float3> corners = world->GetCorners();
		DrawLine(corners[0], corners[1], float4(1, 0, 0, 1));
		DrawLine(corners[0], corners[2], float4(1, 0, 0, 1));
		DrawLine(corners[0], corners[4], float4(1, 0, 0, 1));
		DrawLine(corners[1], corners[3], float4(1, 0, 0, 1));
		DrawLine(corners[1], corners[5], float4(1, 0, 0, 1));
		DrawLine(corners[2], corners[3], float4(1, 0, 0, 1));
		DrawLine(corners[2], corners[6], float4(1, 0, 0, 1));
		DrawLine(corners[3], corners[7], float4(1, 0, 0, 1));
		DrawLine(corners[4], corners[5], float4(1, 0, 0, 1));
		DrawLine(corners[4], corners[6], float4(1, 0, 0, 1));
		DrawLine(corners[5], corners[7], float4(1, 0, 0, 1));
		DrawLine(corners[6], corners[7], float4(1, 0, 0, 1));
	}

	//draw xyz axis
	DrawLine(float3(0), float3(1, 0, 0), float4(1, 0, 0, 1));
	DrawLine(float3(0), float3(0, 1, 0), float4(0, 1, 0, 1));
	DrawLine(float3(0), float3(0, 0, 1), float4(0, 0, 1, 1));

	for (const auto& light : lights) {
		for (const auto& line : light->GetVisualizer()) {
			DrawLine(line.start, line.end, line.color);
		}
	}
}

void Renderer::DrawLine(const float3& from, const float3& to, const float4 color, const bool override, const float duration) {
	if (!settings.DebugLines && !override) return;
	lines.push_back(Line(from, to, color, duration));
}

void Renderer::FlushLines(const float deltaTime) {
	std::vector<Line> toRemove;
	for (auto& line : lines) {
		float2 posFrom = camera.WorldSpaceToScreenSpace(
			line.start,
			camera.camPos,
			camera.camTarget);
		float2 posTo = camera.WorldSpaceToScreenSpace(
			line.end,
			camera.camPos,
			camera.camTarget);

		int2 from = int2(static_cast<int>(posFrom.x * SCRWIDTH), static_cast<int>(posFrom.y * SCRHEIGHT));
		int2 to = int2(static_cast<int>(posTo.x * SCRWIDTH), static_cast<int>(posTo.y * SCRHEIGHT));

#if HDR
		screen->LineWithAlpha(static_cast<float>(from.x), static_cast<float>(from.y), static_cast<float>(to.x), static_cast<float>(to.y), line.color);
#else
		screen->LineWithAlpha(static_cast<float>(from.x), static_cast<float>(from.y), static_cast<float>(to.x), static_cast<float>(to.y), RGBF32_to_RGB8(&line.color));
#endif

		line.duration -= deltaTime;
		if (line.duration <= 0) {
			toRemove.push_back(line);
		}
	}

	for (auto& line : toRemove) {
		lines.erase(std::remove(lines.begin(), lines.end(), line), lines.end());
	}
}

void Renderer::PlaceShapeAtMouse() {
	if (ImGui::GetIO().WantCaptureMouse) return;

	Ray r = GetMouseRay();
	scene.FindNearest(r);

	if (settings.SubtractionMode) {
		const int3 pos = make_int3(r.LocalIntersectionPoint() * WORLDSIZE);
		DrawSphere(scene, pos, static_cast<float>(settings.SphereSize), 0);
	} else {
		uint color = settings.RandomColor ? RandomColor() : settings.DrawColor;
		const int3 pos = make_int3(r.LocalIntersectionPoint() * WORLDSIZE);
		DrawSphere(scene, pos, static_cast<float>(settings.SphereSize), color, settings.DrawMaterial, r.worldIndex);
	}

	ResetAccumulation();
}

void Tmpl8::Renderer::SelectWorld() {
	if (ImGui::GetIO().WantCaptureMouse) return;
	Ray r = GetMouseRay();
	scene.FindNearest(r);
	if (r.voxel == 0) {
		selectedWorld = -1;
		return;
	}
	selectedWorld = r.worldIndex;
}

void Renderer::PerformanceReport(Timer& t) {
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if (alpha > 0.05f) alpha *= 0.5f;
	fps = 1000.0f / avg;
	ms = avg;
	float rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
}

float3 Renderer::GetEnvironmentLight(const Ray& ray) const {
	if (!settings.EnvironmentLight) return float3(0);
	if (settings.UseHDR) {
		return GetEnviromentLightFromTexture(ray);
	}
	float skyGradientT = pow(smoothstep(0.0f, 0.4f, ray.D.y), 0.35f);
	float groundToSkyT = smoothstep(-0.01f, 0.0f, ray.D.y);
	float3 skyGradient = lerp(settings.SkyColorHorizon, settings.SkyColorZenith, skyGradientT);

	float sun = pow(max(0.0f, dot(ray.D, -sunLight->direction)), settings.SunSize) * sunLight->GetIntensity();
	float clampedSun = min(1.0f, sun);

	// Combine ground, sky, and sun color
	float sunMask = groundToSkyT >= 1;
	float3 result = lerp(settings.GroundColor, skyGradient, groundToSkyT) + clampedSun * sunMask;
	return result;
}

float3 Renderer::GetEnviromentLightFromTexture(const Ray& ray) const {
	// Calculate the spherical coordinates of the direction
	float3 direction = normalize(ray.D);
	float theta = acosf(direction.y); // Ensure D.y is clamped between -1 and 1
	float phi = atan2f(direction.z, direction.x);
	if (phi < 0) phi += 2 * PI; // Ensure phi is in the range [0, 2*PI]

	// Convert spherical coordinates to normalized texture coordinates
	float u = phi / (2 * PI);
	float v = (theta / PI);

	//clamp u and v
	u = clamp(u, 0.0f, 1.0f);
	v = clamp(v, 0.0f, 1.0f);

	return settings.EnvironmentBuffer->GetPixel(u, v);
}

bool Tmpl8::Renderer::IsLookingAtFloor(Ray& ray, float3& color) const {
	if (ray.D.y != 0) { // Avoid division by zero
		// Calculate intersection distance with the floor at y = floorY
		float tFloor = (settings.Floor.position.y - ray.O.y) / ray.D.y;

		if (tFloor > 0 && tFloor < ray.t) { // If closer than any existing hit
			ray.t = tFloor; // Update ray distance to intersection
			color = settings.Floor.color; // Return floor color, which could be a specific shade of gray

			//get direct lighting
			float3 intersectionPoint = ray.IntersectionPoint();
			float3 normal = float3(0, 1, 0);
			color *= CalculateDirectLighting(ray, intersectionPoint, normal);


			return true;
		}
	}
	return false;
}


void Renderer::HandleCamera() {

	if (ImGui::GetIO().WantCaptureMouse) return;
	if (InputManager::GetInstance().GetMouseWheelDelta() != 0) {

		camera.camSpeed = InputManager::GetInstance().GetMouseWheelDelta() > 0 ? camera.camSpeed * 2.0f : camera.camSpeed * 0.5f;
	}
	camera.camSpeed = max(0.0f, camera.camSpeed);
}

void Tmpl8::Renderer::HandleCellularAutomata() {
	if (scene.worlds.empty()) return;
	VoxelWorld* world = scene.worlds[0];
	if (world == nullptr) return;

	std::vector<int3> NeighbourOffsets;
	if (settings.NeighbourHood == NeighbourHood::VonNeumann) {
		NeighbourOffsets = std::vector<int3>(
			{ int3(0, 0, 1), int3(0, 0, -1), int3(1, 0, 0), int3(-1, 0, 0), int3(0, 1, 0), int3(0, -1, 0) }
		);
	} else {
		NeighbourOffsets = std::vector<int3>(
			{ int3(0, 0, 1), int3(0, 0, -1), int3(1, 0, 0), int3(-1, 0, 0), int3(0, 1, 0), int3(0, -1, 0),
						int3(1, 1, 0), int3(1, -1, 0), int3(-1, 1, 0), int3(-1, -1, 0),
						int3(1, 1, 1), int3(1, 1, -1), int3(1, -1, 1), int3(1, -1, -1),
						int3(-1, 1, 1), int3(-1, 1, -1), int3(-1, -1, 1), int3(-1, -1, -1) }
		);
	}

	bool survival[26];
	memcpy(survival, settings.Survival, 26 * sizeof(bool));
	bool spawn[26];
	memcpy(spawn, settings.Spawn, 26 * sizeof(bool));
	const int startState = settings.StartState;

	int3 dimensions = world->gridDimensions * BRICKSIZE;

	memset(newCellStates, 0, dimensions.x * dimensions.y * dimensions.z * sizeof(int));

#pragma omp parallel for schedule(dynamic)
	for (int x = 0; x < dimensions.x; x++) {
		for (int y = 0; y < dimensions.y; y++) {
			for (int z = 0; z < dimensions.z; z++) {

				int index = x + y * dimensions.x + z * dimensions.x * dimensions.y;
				int state = cellStates[index];

				int count = 0;
				const int3 pos = int3(x, y, z);
				for (const auto& offset : NeighbourOffsets) {
					int3 neighbourPos = pos + offset;
					//if out of bounds, go to the other side
					if (neighbourPos.x < 0) neighbourPos.x = dimensions.x - 1;
					if (neighbourPos.y < 0) neighbourPos.y = dimensions.y - 1;
					if (neighbourPos.z < 0) neighbourPos.z = dimensions.z - 1;
					if (neighbourPos.x >= dimensions.x) neighbourPos.x = 0;
					if (neighbourPos.y >= dimensions.y) neighbourPos.y = 0;
					if (neighbourPos.z >= dimensions.z) neighbourPos.z = 0;


					int neighbourIndex = neighbourPos.x + neighbourPos.y * dimensions.x + neighbourPos.z * dimensions.x * dimensions.y;
					if (cellStates[neighbourIndex] == startState - 1) {
						count++;
					}
				}

				if (state == 0) {
					if (spawn[count]) {
						newCellStates[index] = startState - 1;
					}
				} else {
					if (survival[count] && state == startState - 1) {
						newCellStates[index] = state;
					} else {
						newCellStates[index] = state - 1;
					}

				}
				//set color gradient based on state, red is alive, yellow is dying
				uint color;
				if (newCellStates[index] == 0) {
					color = 0;
				} else {
					float t = (float)newCellStates[index] / (float)startState;
					float4 c = lerp(float4(1, 0, 0, 1), float4(1, 1, 0, 1), t);
					color = RGBF32_to_RGB8(&c);
				}

				world->Set(x, y, z, color);
			}
		}
	}

	//swap the arrays
	std::swap(cellStates, newCellStates);
}


void Tmpl8::Renderer::RandomizeCellularAutomata() {
	cellStates = (int*)MALLOC64(WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);
	newCellStates = (int*)MALLOC64(WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);
	memset(cellStates, 0, WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);
	memset(newCellStates, 0, WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);

	//resize world
	scene.CLearWorlds();
	int3 gridSize = int3(WORLDSIZE / BRICKSIZE);
	//scene.worlds[0]->Resize(gridSize);
	//scene.worlds[0]->Clear(0);
	VoxelWorld* world = new VoxelWorld();
	world->Resize(gridSize);
	scene.worlds.push_back(world);

	const int radius = settings.radius;
	const int3 center = WORLDSIZE / 2;
	const int3 start = center - radius;
	const int3 end = center + radius;
	//randomize the cell states
	for (int x = start.x; x < end.x; x++) {
		for (int y = start.y; y < end.y; y++) {
			for (int z = start.z; z < end.z; z++) {
				const int index = x + y * WORLDSIZE + z * WORLDSIZE * WORLDSIZE;
				const int random = RandomFloat() > settings.probablity ? 0 : 1;

				cellStates[index] = random == 0 ? 0 : settings.StartState;
				uint color = random == 0 ? 0x000000 : 0xff0000;
				scene.worlds[0]->Set(x, y, z, color);
			}
		}
	}
}

void Tmpl8::Renderer::ClearCellularAutomata() {
	memset(cellStates, 0, WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);
	memset(newCellStates, 0, WORLDSIZE * WORLDSIZE * WORLDSIZE * 4);

	scene.worlds[0]->Clear(0);
}

void Renderer::ResetAccumulation() {
	if (settings.Reprojection) return;
	frameIndex = 1;
	memset(reprojection, 0, SCRWIDTH * SCRHEIGHT * 16);
}

void Tmpl8::Renderer::SetCamSettings() {
	if (settings.FocusCenter) {
		//send a ray from the center of the screen and find the intersection point and check the distance
		Ray r = camera.GetNoEffectPrimaryRay(SCRWIDTH / 2, SCRHEIGHT / 2);
		scene.FindNearest(r);
		if (r.voxel != 0) {
			camera.focalDistance = r.t;
		} else {
			camera.focalDistance = 100;
		}
	} else if (settings.FocusMouse) {
		//send a ray from the mouse position and find the intersection point and check the distance
		Ray r = GetMouseRay();
		scene.FindNearest(r);
		if (r.voxel != 0) {
			camera.focalDistance = r.t;
		} else {
			camera.focalDistance = 100;
		}
	}

	if (settings.FocusMouse) {
		Vector2 mouseDelta = InputManager::GetInstance().GetMouseDelta();
		if (mouseDelta.x != 0 || mouseDelta.y != 0) {
			ResetAccumulation();
		}
	}
}

#pragma region InputStuff
void Renderer::MouseUp(int button) {
	InputManager::GetInstance().MouseUp(button);
}

void Renderer::MouseDown(int button) {
	InputManager::GetInstance().MouseDown(button);
}

void Renderer::MouseMove(int x, int y) {
	InputManager::GetInstance().MouseMove(x, y);
}

void Renderer::MouseWheel(float y) {
	InputManager::GetInstance().MouseWheel(y);
}

void Renderer::KeyUp(int key) {
	InputManager::GetInstance().KeyUp(key);
}

void Renderer::KeyDown(int key) {
	InputManager::GetInstance().KeyDown(key);
}
void Tmpl8::Renderer::LoadLevel(const int levelIndex) {
	renderState = PUZZLE;
	PuzzleScene* puzzleScene = new PuzzleScene();
	puzzleScene->renderer = this;
	puzzleScene->StartLevel = levelIndex;
	sceneManager.LoadScene(puzzleScene);

	settings.VignetteIntensity = 0.2f;
	settings.VignetteRadius = 0.6f;
	shader->Bind();
	shader->SetFloat("vignetteIntensity", settings.VignetteIntensity);
	shader->SetFloat("vignetteRadius", settings.VignetteRadius);
	shader->Unbind();
}
void Tmpl8::Renderer::LoadCellularAutomata() {
	renderState = CELLULAR_AUTOMATA;
	CellularAutomata* gameOfLifeScene = new CellularAutomata();
	gameOfLifeScene->renderer = this;
	sceneManager.LoadScene(gameOfLifeScene);

	RandomizeCellularAutomata();

	camera.camPos = float3(1.57f, 1.5f, -0.65f);
	camera.camTarget = normalize(float3(0.993f, 0.918f, -0.05f));
	camera.camSpeed = 0.3f;
	camera.fov = 1.75f;
	settings.EnvironmentLight = true;
	settings.UseHDR = false;
	settings.VignetteRadius = 1.0f;
	shader->Bind();
	shader->SetFloat("vignetteRadius", settings.VignetteRadius);
	shader->Unbind();
	pointLight->Intensity = 0.0f;
	settings.PathTracing = false;
	settings.Reprojection = false;
	settings.DebugLines = true;
	selectedWorld = 0;
	sunLight->Intensity = 0.2f;
	settings.fps = 25.0f;
	sunLight->direction = normalize(float3(0.61f, -0.55f, 0.57f));
	sunLight->Intensity = 0.2f;
	areaLight->Intensity = 0.0f;
	camera.UpdateProjection();
}
void Tmpl8::Renderer::LoadFreeCamera() {
	renderState = FREE_CAMERA;
	FreeCam* gameOfLifeScene = new FreeCam();
	gameOfLifeScene->renderer = this;
	sceneManager.LoadScene(gameOfLifeScene);


	camera.camPos = float3(1.77f, 1.75f, -2.65f);
	camera.camTarget = normalize(float3(1.230f, 1.317f, -1.899f));
	camera.camSpeed = 0.3f;
	camera.fov = 1.75f;
	settings.EnvironmentLight = true;
	settings.UseHDR = true;
	settings.VignetteRadius = 1.0f;
	shader->Bind();
	shader->SetFloat("vignetteRadius", settings.VignetteRadius);
	shader->Unbind();
	pointLight->Intensity = 0.0f;
	settings.PathTracing = true;
	settings.Reprojection = true;
	settings.DebugLines = true;
	selectedWorld = 0;
	sunLight->Intensity = 0.2f;
	settings.fps = 0;
	sunLight->direction = normalize(float3(0.61f, -0.55f, 0.57f));
	scene.GenerateGrid();
	camera.UpdateProjection();
}
void Tmpl8::Renderer::UpdateSHaderSettings() {
	shader->Bind();
	shader->SetFloat("vignetteIntensity", settings.VignetteIntensity);
	shader->SetFloat("vignetteRadius", settings.VignetteRadius);
	shader->SetFloat("chromaAmount", settings.ChromaIntensity);
	shader->SetFloat("sigma", settings.Sigma);
	shader->SetFloat("kSigma", settings.KSigma);
	shader->SetFloat("threshold", settings.Threshold);
	shader->Unbind();
}
#pragma endregion