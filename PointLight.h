#pragma once
#include "Light.h" // Make sure to include the base Light class

namespace Tmpl8 {
	class PointLight : public Light {
	public:
		// Constructor: Initializes a PointLight object with position, color, and intensity
		PointLight(const float3& position, const float3& color, float intensity)
			: Light(color, intensity), position(position) {
			printf("PointLight created\n");
		}

		// GetContribution: Calculates the light contribution at a given point in the scene
		float3 PointLight::GetContribution(const Scene& scene, const Ray&, const float3& intersectionPoint, const float3& normal) override {
			float3 distanceVec = position - intersectionPoint;
			float distanceSquared = dot(distanceVec, distanceVec);
			float3 lightDir = normalize(distanceVec);

			// for Lambert's cosine law
			float NdotL = dot(normal, lightDir);
			// No light contribution if surface is facing away
			if (NdotL <= 0.0f) return float3(0);

			// Adjust shadow ray origin
			float3 shadowRayOrigin = intersectionPoint + normal * EPSILON;
			Ray shadowRay(shadowRayOrigin, lightDir, length(distanceVec));

			// Perform shadow check
			if (scene.IsOccluded(shadowRay)) {
				return float3(0); // In shadow
			}/* else {
				return Color * GetIntensity() * NdotL / distanceSquared;
			}*/

			// Access the material's metallic property
			//const Material& mat = ray.GetMaterial(); // Assuming GetMaterial() is a method that retrieves the material from the ray
			//float metallicFactor = mat.metallic;

			//// Calculate the diffuse contribution
			//float3 diffuseContribution = (1.0f - metallicFactor) * Color * GetIntensity() * NdotL;

			//// Calculate the specular contribution for metallic surfaces
			//float3 V = normalize(-ray.D); // View direction
			//float3 H = normalize(V + lightDir); // Halfway vector between view direction and light direction
			//float NdotH = max(dot(normal, H), 0.0f);
			//float3 specularContribution = metallicFactor * Color * GetIntensity() * pow(NdotH, mat.roughness * 128.0f); // Using Blinn-Phong for simplicity

			// Fall off calculation
			float fallOff = constantTerm + linearTerm * sqrt(distanceSquared) + quadraticTerm * distanceSquared;


			// Calculate light attenuation including fall off
			float attenuation = 1.0f / (fallOff * distanceSquared);

			// Combine diffuse and specular contributions and apply attenuation
			return Color * GetIntensity() * NdotL * attenuation;
		}

		bool PointLight::DrawImgui(int index) {
			std::string positionLabel = "Position##" + std::to_string(index);
			std::string constantLabel = "Constant Term##" + std::to_string(index);
			std::string linearLabel = "Linear Term##" + std::to_string(index);
			std::string quadraticLabel = "Quadratic Term##" + std::to_string(index);

			bool changed = false;
			changed |= ImGui::DragFloat3(positionLabel.c_str(), &position.x, 0.001f);
			changed |= ImGui::DragFloat(constantLabel.c_str(), &constantTerm, 0.001f, 0.0f, 10.0f, "%.3f");
			changed |= ImGui::DragFloat(linearLabel.c_str(), &linearTerm, 0.001f, 0.0f, 1.0f, "%.3f");
			changed |= ImGui::DragFloat(quadraticLabel.c_str(), &quadraticTerm, 0.001f, 0.0f, 1.0f, "%.3f");

			changed |= Light::DrawImgui(index);
			return changed;
		}


		float PointLight::GetContribution(const float3 pos) {
			float3 distance = position - pos;
			float distanceSquared = dot(distance, distance);
			float3 lightDir = normalize(distance);
			return 1.0f / distanceSquared;
		}

		// GetVisualizer: Returns a vector of lines to visualize the light
		std::vector<Line> GetVisualizer() const override {
			return VisualizeSphere(VOXELSIZE / 2.0f, position, Color, 10, 10);
		}

		float3 position; // Position of the point light in the scene
		// Fall-off properties
		float constantTerm = 1.0f;
		float linearTerm = 0.09f;
		float quadraticTerm = 0.032f;
	private:
	};
} // namespace Tmpl8
