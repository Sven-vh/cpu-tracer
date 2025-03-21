#pragma once

namespace Tmpl8 {
	class Scene;
}

void DrawSphere(Scene& scene, const int3& center, float radius, const uint color, const int materialIndex = 0, const int worldIndex = 0, float probability = 1.0f);

void DrawHollowSphere(Scene& scene, const float3& center, const float radius, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawCube(Scene& scene, const float3& center, float size, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawHollowCube(Scene& scene, const float3& center, float size, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawBox(Scene& scene, const float3& center, const float3& size, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawHollowBox(Scene& scene, const float3& center, const float3& size, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawLine(Scene& scene, const float3& start, const float3& end, const uint color, const int materialIndex = 0, const int worldIndex = 0);

void DrawVoxFile(Scene& scene, const char* filename, float3 position, const int worldIndex = 0);

void LoadVoxFile(Scene& scene, const char* filename);

std::vector<Line> VisualizeSphere(const float radius, const float3 center, const float3 color, const int latitudeLines, const int longitudeLines);
std::vector<Line> VisualizeCube(const float3& center, const float3& size, const float3 color);
