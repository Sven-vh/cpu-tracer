#pragma once
struct Plane {
	float3 position;
	float3 color;

	Plane(const float3& position, const float3& color) : position(position), color(color) {}

	bool DrawImgui(int index) {
		std::string colorLabel = "Color##" + std::to_string(index);
		bool changed = false;
		changed |= ImGui::ColorEdit3(colorLabel.c_str(), &color.x);

		std::string positionLabel = "Position##" + std::to_string(index);
		changed |= ImGui::DragFloat3(positionLabel.c_str(), &position.x, 0.001f);

		return changed;
	}
};

enum NeighbourHood {
	Moore,
	VonNeumann
};

class Settings {
public:

	bool PathTracing = true;
	bool StepThrough = false;
	bool Normals = false;
	bool UV = false;
	bool DebugDraw = true;

	bool Accumulate = false;
	bool Reprojection = true;

	bool AntiAliasing = false;
	bool Jitter = false;

	bool FocusCenter = true;
	bool FocusMouse = false;

	int AntiAliasingSamples = 4;

	float VignetteIntensity = 0.2f;
	float VignetteRadius = 0.6f;
	float ChromaIntensity = 0.0f;
	float Sigma = 0.5f;
	float KSigma = 1.0f;
	float Threshold = 0.5f;

	int PathTracingMaxDepth = 2;
	float RussianRouletteThreshold = 0.5f;
	int MinDepthRussiaRoulette = 3;

	float repoDepthThreshold = 0.05f;
	float repoNormalThreshold = 0.85f;
	float repoBlendFactor = 0.85f;

	bool DrawMode = false;
	bool SubtractionMode = false;
	bool RandomColor = true;
	int DrawMaterial = 0;
	int SphereSize = 10;
	uint DrawColor = 0xffffff;

	float3 VoxelFilePosition = float3(0.0f);

	float Exposure = 1.0f;
	ToneMappingType ToneMapping = ToneMappingType::Reinhard;

	bool EnvironmentLight = false;
	bool UseHDR = true;

	bool RenderFloor = false;
	Plane Floor = Plane(float3(0, 0, 0), float3(0.5f, 0.5f, 0.5f));

	float SunSize = 100.0f;
	float3 SkyColorZenith = float3(0.08f, 0.35f, 0.75f);
	float3 SkyColorHorizon = float3(1.0f, 1.0f, 1.0f);
	float3 GroundColor = float3(0.35f, 0.3f, 0.35f);
	FLoatSurface* EnvironmentBuffer;

	//cellular automata settings
	bool Survival[26] = { 0, 0, 0, 0, 1, 1};
	bool Spawn[26] = { 0, 0, 0, 1};
	int StartState = 6;
	int NeighbourHood = NeighbourHood::Moore;
	float probablity = 0.5f;
	float fps = 20;
	int radius = 15;

	bool DebugLines = false;

	bool DrawSettingsWindow();

	bool HandleRenderOptions();
	bool HandleToneMappingOptions();
	bool HandleAliasingOptions();
	bool AccumulationImguiWindow();
	bool HandlePathTracingImguiWindow();
	bool HandleMaterialsImguiWindow();
	bool HandleDrawingImguiWindow();
	bool HandleSkyLightImguiWindow();
	bool DrawRepojectionImguiWindow();
};