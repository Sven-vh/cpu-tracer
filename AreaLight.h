#pragma once
#include "Light.h" // Include the base Light class
namespace Tmpl8 {
	class AreaLight : public Light {
	public:
		// Constructor: Initializes an AreaLight object with position, size, rotation, color, and intensity
		AreaLight(const float3& position, const float2& size, const float3& rotation, const int& numSamples, const float3& color, float intensity)
			: Light(color, intensity), position(position), size(size), rotation(rotation), numSamples(numSamples) {
			printf("AreaLight created\n");
		}
		// GetContribution: Calculates the light contribution at a given point in the scene
		float3 GetContribution(const Scene& scene, const Ray&, const float3& intersectionPoint, const float3& normal) override {
			float3 lightContribution = float3(0);
			for (int i = 0; i < numSamples; i++) {
				float3 samplePoint = RandomPointOnPlane(position, rotation, size);
				float3 distance = samplePoint - intersectionPoint;
				float3 lightDir = normalize(distance);
				Ray lightRay(intersectionPoint + lightDir * 0.001f, lightDir, length(distance));
				if (!scene.IsOccluded(lightRay)) {
					float NdotL = max(dot(normal, lightDir), 0.0f);
					if (NdotL > 0.0f) {
						float3 diffuseContribution = Color * GetIntensity() * NdotL;
						lightContribution += diffuseContribution;
					}
				}
			}
			return lightContribution / static_cast<float>(numSamples);
		}
		bool DrawImgui(int index) override {
			std::string positionLabel = "Position##" + std::to_string(index);
			std::string sizeLabel = "Size##" + std::to_string(index);
			std::string rotationLabel = "Rotation##" + std::to_string(index);
			std::string numSamplesLabel = "Num Samples##" + std::to_string(index);
			bool changed = false;
			changed |= ImGui::SliderFloat3(positionLabel.c_str(), &position.x, 0.0f, 1.0f);
			changed |= ImGui::SliderFloat2(sizeLabel.c_str(), &size.x, 0.0f, 1.0f);
			changed |= DirectionProperties(rotation, index);
			changed |= Light::DrawImgui(index);
			changed |= ImGui::SliderInt(numSamplesLabel.c_str(), &numSamples, 1, 64);
			//rotation text
			ImGui::Text("Rotation: %f, %f, %f", rotation.x, rotation.y, rotation.z);
			return changed;
		}

		// GetVisualizer: Returns a vector of lines to visualize the area light in the scene
		std::vector<Line> GetVisualizer() const override {
			std::vector<Line> visualizer;
			float3 right = normalize(cross(float3(0, 1, 0), rotation));
			float3 up = normalize(cross(rotation, right));
			float3 halfSize = float3(size.x / 2, size.y / 2, 0);
			float3 p0 = position - right * halfSize.x - up * halfSize.y;
			float3 p1 = position + right * halfSize.x - up * halfSize.y;
			float3 p2 = position + right * halfSize.x + up * halfSize.y;
			float3 p3 = position - right * halfSize.x + up * halfSize.y;
			visualizer.push_back(Line(p0, p1, float4(Color, 1)));
			visualizer.push_back(Line(p1, p2, float4(Color, 1)));
			visualizer.push_back(Line(p2, p3, float4(Color, 1)));
			visualizer.push_back(Line(p3, p0, float4(Color, 1)));
			return visualizer;
		}

		float3 position; // Position of the area light in the scene
		float dummy;
		float2 size; // Size of the area light (width and height)
		float3 rotation; // Rotation of the area light in Euler angles (pitch, yaw, roll)
		int numSamples = 8; // Number of samples to use for the area light
	};
} // namespace Tmpl8