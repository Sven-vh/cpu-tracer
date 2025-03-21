#include "precomp.h"
#include "Settings.h"

bool Settings::DrawSettingsWindow() {
	bool changed = false;
	changed |= HandleRenderOptions();
	changed |= HandleToneMappingOptions();
	changed |= HandleAliasingOptions();
	changed |= DrawRepojectionImguiWindow();
	changed |= AccumulationImguiWindow();
	changed |= HandleSkyLightImguiWindow();
	changed |= HandlePathTracingImguiWindow();
	changed |= HandleMaterialsImguiWindow();
	changed |= HandleDrawingImguiWindow();
	return changed;
}


bool Settings::HandleRenderOptions() {
	//collapsing header for render options
	if (!ImGui::CollapsingHeader("Render Options")) return false;
	bool changed = false;
	ImGui::Checkbox("Debug Lines", &DebugLines);
	changed |= ImGui::Checkbox("Path Tracing", &PathTracing);
	if (ImGui::Checkbox("Step Through", &StepThrough)) {
		if (StepThrough) Normals = false;
		changed = true;
	}
	if (ImGui::Checkbox("Normals", &Normals)) {
		if (Normals) StepThrough = false;
		changed = true;
	}
	if (ImGui::Checkbox("UV", &UV)) {
		if (UV) StepThrough = false;
		changed = true;
	}


	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}

bool Settings::HandleToneMappingOptions() {
	//collapsing header for tone mapping
	if (!ImGui::CollapsingHeader("Tone Mapping")) return false;
	bool changed = false;
	//imgui enum dropdown for tone mapping
	changed |= ImGui::DragFloat("Exposure", &Exposure, 0.01f);
	changed |= DisplayToneMappingDropdown("Type", ToneMapping);

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}

bool Settings::HandleAliasingOptions() {
	//collapsing header for anti-aliasing
	if (!ImGui::CollapsingHeader("Anti-Aliasing")) return false;
	bool changed = false;
	if (ImGui::Checkbox("Jitter", &Jitter)) {
		if (Jitter) AntiAliasing = false;
		changed = true;
	}
	if (ImGui::Checkbox("Anti Aliasing", &AntiAliasing)) {
		if (AntiAliasing) Jitter = false;
		changed = true;
	}
	changed |= ImGui::SliderInt("Samples", &AntiAliasingSamples, 1, 16);

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}

bool Settings::HandleSkyLightImguiWindow() {
	//collapsing header for sky light
	if (!ImGui::CollapsingHeader("HDR")) return false;
	bool changed = false;
	changed |= ImGui::Checkbox("Environment Light", &EnvironmentLight);

	changed |= ImGui::Checkbox("Use HDR", &UseHDR);
	if (UseHDR) {
		changed |= ImGui::Checkbox("Billinear Interpolation", &bilinearTextures);
		changed |= SurfaceDropDown(EnvironmentBuffer, 999);
	} else {
		changed |= ImGui::SliderFloat("Sun Focus", &SunSize, 0, 500.0f);
		changed |= ImGui::ColorEdit3("Zenith", (float*)&SkyColorZenith);
		changed |= ImGui::ColorEdit3("Horizon", (float*)&SkyColorHorizon);
		changed |= ImGui::ColorEdit3("Void", (float*)&GroundColor);
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}
bool Settings::DrawRepojectionImguiWindow() {
	//collapsing header for reprojection
	if (!ImGui::CollapsingHeader("Reprojection Settings")) return false;
	bool changed = false;
	if (ImGui::Checkbox("Reprojection", &Reprojection)) {
		if (Reprojection) Accumulate = false;
		changed = true;
	}
	changed |= ImGui::SliderFloat("Depth Threshold", &repoDepthThreshold, 0, 1);
	changed |= ImGui::SliderFloat("Normal Threshold", &repoNormalThreshold, 0, 1);
	changed |= ImGui::SliderFloat("Blend Factor", &repoBlendFactor, 0, 1);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}
bool Settings::AccumulationImguiWindow() {
	//collapsing header for accumulation
	if (!ImGui::CollapsingHeader("Accumulation")) return false;
	bool changed = false;
	if (ImGui::Checkbox("Accumulate", &Accumulate)) {
		if (Accumulate) Reprojection = false;
		changed = true;
	}
	if (ImGui::Button("Reset")) {
		changed = true;
	}
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}


bool Settings::HandleDrawingImguiWindow() {
	//collapsing header for drawing
	if (!ImGui::CollapsingHeader("Drawing")) return false;
	ImGui::Checkbox("Draw Mode", &DrawMode);
	ImGui::Checkbox("Subtraction Mode", &SubtractionMode);
	ImGui::Checkbox("Random Color", &RandomColor);
	uintColorPicker("Color", DrawColor);
	ImGui::SliderInt("Material", &DrawMaterial, 0, static_cast<int>(MaterialList.size() - 1));
	ImGui::SliderInt("Sphere Size", &SphereSize, 1, WORLDSIZE * 2);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return false;
}

bool Settings::HandlePathTracingImguiWindow() {
	//collapsing header for path tracing
	if (!ImGui::CollapsingHeader("Path Tracing")) return false;
	bool changed = false;
	changed |= ImGui::Checkbox("Toggle Tracing", &PathTracing);
	changed |= ImGui::SliderInt("Max Depth", &PathTracingMaxDepth, 1, 100);
	changed |= ImGui::SliderFloat("Roulette Treshold", &RussianRouletteThreshold, 0.0f, 1.0f);
	changed |= ImGui::SliderInt("Min Depth for Roulette", &MinDepthRussiaRoulette, 1, PathTracingMaxDepth);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}

bool Settings::HandleMaterialsImguiWindow() {
	//collapsing header for materials
	if (!ImGui::CollapsingHeader("Materials")) return false;
	bool changed = false;
	if (ImGui::Button("Add Material")) {
		MaterialList.push_back(MaterialList.back());
		changed = true;
	}

	changed |= ImGui::Checkbox("Bilinear Textures", &bilinearTextures);

	int i = 0;
	int indexToRemove = -1;
	for (Material& material : MaterialList) {
		ImGui::Text("Material %i", i);
		if (material.DrawImGui(i++)) {
			changed = true;
		}

		//button to remove material
		std::string removeLabel = "Remove##" + std::to_string(i);
		if (ImGui::Button(removeLabel.c_str())) {
			indexToRemove = i;
		}
		ImGui::Separator();
	}

	if (indexToRemove != -1) {
		MaterialList.erase(MaterialList.begin() + indexToRemove - 1);
		changed = true;
	}


	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	return changed;
}
#pragma endregion