#pragma once
#include "Light.h"
namespace Tmpl8 {
	class DirectionalLight : public Light {
	public:
		DirectionalLight(const float3& direction, const float3& color, float intensity)
			: Light(color, intensity), direction(normalize(direction)) {
			printf("DirectionalLight created\n");
		}

		float3 DirectionalLight::GetContribution(const Scene& scene, const Ray& ray, const float3& intersectionPoint, const float3& normal) {
			// for Lambert's cosine law
			float NdotL = dot(normal, -direction);
			if (NdotL <= EPSILON) return float3(0); // No light contribution if surface is facing away


			Ray shadowRay(intersectionPoint + normal * EPSILON, -direction);

			// Perform shadow check
			if (scene.IsOccluded(shadowRay)) {
				return float3(0); // In shadow
			}

			// Access the material's metallic property
			const Material& mat = ray.GetMaterial();
			float metallicFactor = mat.metallic;

			// Calculate the diffuse contribution
			float3 diffuseContribution = (1.0f - metallicFactor) * Color * GetIntensity() * NdotL;

			// Calculate the specular contribution for metallic surfaces
			// Assuming a simple model where the specular color is influenced by the light's color and the material's albedo
			float3 H = (-ray.D - direction);
			float NdotH = max(dot(normal, H), 0.0f);
			float3 specularContribution = metallicFactor * Color * GetIntensity() * pow(NdotH, mat.roughness * 128.0f); // Using Blinn-Phong for simplicity

			// Combine diffuse and specular contributions
			return diffuseContribution + specularContribution;
		}

		// GetVisualizer: Returns a vector of lines to visualize the light
		std::vector<Line> GetVisualizer() const override {
			std::vector<Line> visualizer;
			visualizer.push_back(Line(float3(0), direction, float4(Color, 1)));
			return visualizer;
		}

		bool DirectionalLight::DrawImgui(int index) {
			ImGui::Text("Direction");
			if (DirectionProperties(direction, index)) return true;
			if (Light::DrawImgui(index)) return true;
			//direction text
			ImGui::Text("Direction: ");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(direction.x, direction.y, direction.z, 1), "X: %.2f Y: %.2f Z: %.2f", direction.x, direction.y, direction.z);
			return false;
		}

		float DirectionalLight::GetContribution(const float3) {
			return 1.0f;
		}

		float3 direction;
	};
} // namespace Tmpl8