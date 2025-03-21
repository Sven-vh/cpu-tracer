#pragma once
#include "Light.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "AreaLight.h"
#include "Settings.h"


struct PixelInfo {
	uint x, y; // 8 bytes
	float depth, dummy; // 8 bytes
	float4 color; // 16 bytes
	Ray ray; // 196 bytes

	PixelInfo() : x(0), y(0), color(float4(0)), depth(0) {}
	PixelInfo(uint x, uint y) : x(x), y(y) {}
};


struct Sphere {
	float3 position;
	float3 velocity;
	float radius;
	float mass;
	float3 color;
};

enum RenderState {
	PUZZLE,
	CELLULAR_AUTOMATA,
	FREE_CAMERA
};

namespace Tmpl8 {

	class Renderer : public TheApp {
	public:
		// game flow methods
		void Init();
		void Trace(PixelInfo& currentPixel) const;
		void Tick(float deltaTime);
		void UpdateBallPhysics(float deltaTime);
		void PerformanceReport(Timer& t);
		void RenderScreen(const float cameraDistance);
		void ProcessScreen();
		void ProcessScreenSIMD();

		inline float4 ApplyReprojection(PixelInfo& currentPixel, const float& cameraDepthDelta, const float& depthThreshold, const float& blendFactor) const;

		void NoEffect(PixelInfo& currentPixel) const;
		void ApplyAntiAliasing(PixelInfo& currentPixel) const;
		void ApplyJitter(PixelInfo& currentPixel) const;
		void DrawLine(const float3& from, const float3& to, const float4 color = float4(1, 0, 0, 0), const bool override = false, const float duration = 0.0f);


		void UI();
		void Shutdown();

		// input handling
		void MouseUp(int button);
		void MouseDown(int button);
		void MouseMove(int x, int y);
		void MouseWheel(float y);
		void KeyUp(int key);
		void KeyDown(int key);

		void LoadLevel(const int levelIndex);
		void LoadCellularAutomata();
		void LoadFreeCamera();

		void UpdateSHaderSettings();

		Ray GetMouseRay() const;

		// data members
		float4* reprojection;
		float4* prevReprojection;

		Scene scene;
		Camera camera;

		std::shared_ptr<PointLight> pointLight;
		std::shared_ptr<Light> ambientLight;
		std::shared_ptr<DirectionalLight> sunLight;
		std::shared_ptr<AreaLight> areaLight;
		Settings settings;
		SceneManager sceneManager;

		bool IsInMenu = false;
		bool CanSelectWorld = true;
		RenderState renderState = PUZZLE;
	private:

		int frameIndex = 0;

		int selectedWorld = 0;

		std::vector<std::shared_ptr<Light>> lights;
		std::vector<Line> lines;

		float fps = 0.0f;
		float ms = 0.0f;
		float time = 0.0f;

		Sphere ball;

		int* cellStates;
		int* newCellStates;
		void HandleUserInput();
		void PlaceShapeAtMouse();
		void SelectWorld();
		void HandleCamera();

		void HandleCellularAutomata();
		void RandomizeCellularAutomata();
		void ClearCellularAutomata();

		void DrawSidebar();
		void DrawBasicInfoImguiWindow();
		void DrawCameraImguiWindow();
		void DrawShaderImguiWindow();
		void DrawWorldImguiWindow();
		void DrawCellularAutomataImguiWindow();
		void DrawLightsImguiWindow();
		void DrawWorldSettingsImguiWindow();

		void ResetAccumulation();

		void SetCamSettings();

		float3 PerformPathTracing(PixelInfo& ray, int depth = 0) const;
		float3 PerformSimpleRendering(Ray& ray) const;
		float3 CalculateDirectLighting(const Ray& ray, const float3& I, const float3& N)const;

		float3 GetEnvironmentLight(const Ray& ray) const;
		float3 GetEnviromentLightFromTexture(const Ray& ray) const;

		bool IsLookingAtFloor(Ray& ray, float3& color) const;

		void DebugDraw();
		void FlushLines(const float deltaTime);

		inline void SetScreen(const float4& color, const int index) {
#if HDR
			screen->pixels[index] = color;
#else
			screen->pixels[index] = RGBF32_to_RGB8(&color);
#endif
		}
	};

} // namespace Tmpl8