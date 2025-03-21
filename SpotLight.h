#pragma once
#include "Light.h" // Make sure to include the base Light class

namespace Tmpl8 {
	class SpotLight : public Light {
	public:
		// Constructor: Initializes a SpotLight object with position, direction, color, intensity, angle, and falloff
		SpotLight(const float3& position, const float3& direction, const float3& color, float intensity, float angle, float falloff)
			: Light(color, intensity), position(position), direction(normalize(direction)), angle(angle), falloff(falloff) {
			printf("SpotLight created\n");
		}

		// GetContribution: Calculates the light contribution at a given point in the scene
		float3 GetContribution(const Scene& scene, const Ray& ray, const float3& point, const float3& normal) override {
			float3 lightVec = position - point;
			float distanceSquared = dot(lightVec, lightVec);
			float3 lightDir = normalize(lightVec);
			float NdotL = dot(normal, lightDir);
			if (NdotL <= 0.0f) return float3(0); // No contribution if surface is facing away

			float spotEffect = dot(-lightDir, direction);
			float cosfValue = cosf(angle * 0.5f);
			if (spotEffect < cosfValue) return float3(0); // Outside of the spotlight cone

			// Adjust shadow ray origin based on fastRender option
			float3 offset = normal * EPSILON;
			float3 shadowOrigin = point + offset;
			Ray shadowRay(shadowOrigin, lightDir, length(lightVec));

			// Perform shadow check
			if (scene.IsOccluded(shadowRay)) {
				return float3(0); // In shadow
			}

			// Access the material's metallic property
			const Material& mat = ray.GetMaterial(); // Assuming GetMaterial() is a method that retrieves the material from the ray
			float metallicFactor = mat.metallic;

			// Calculate the diffuse contribution
			float3 diffuseContribution = (1.0f - metallicFactor) * Color * GetIntensity() * NdotL;

			// Calculate the specular contribution for metallic surfaces
			float3 V = normalize(-ray.D); // View direction
			float3 H = normalize(V + lightDir); // Halfway vector between view direction and light direction
			float NdotH = max(dot(normal, H), 0.0f);
			float3 specularContribution = metallicFactor * Color * GetIntensity() * pow(NdotH, mat.roughness * 128.0f); // Using Blinn-Phong for simplicity

			// Calculate attenuation and falloff
			float attenuation = 1.0f / distanceSquared;
			float falloffFactor = powf(spotEffect, falloff);

			// Combine diffuse and specular contributions and apply attenuation and falloff
			return (diffuseContribution + specularContribution) * attenuation * falloffFactor;
		}


		// DrawImgui: Provides an interface for adjusting the light's properties using ImGui
		bool DrawImgui(int index) override {
			std::string positionLabel = "Position##" + std::to_string(index);
			std::string directionLabel = "Direction##" + std::to_string(index);
			std::string angleLabel = "Angle##" + std::to_string(index);
			std::string falloffLabel = "Falloff##" + std::to_string(index);

			bool changed = false;
			changed |= ImGui::SliderFloat3(positionLabel.c_str(), &position.x, 0.0f, 1.0f);
			ImGui::Text("Direction");
			changed |= DirectionProperties(direction, index);
			changed |= ImGui::SliderFloat(angleLabel.c_str(), &angle, 0, 2 * TWEEN_PI);
			changed |= ImGui::SliderFloat(falloffLabel.c_str(), &falloff, 0.0f, 128.0f);
			changed |= Light::DrawImgui(index);
			return changed;
		}

		float SpotLight::GetContribution(const float3 pos) {
			float3 lightVec = position - pos;
			float distanceSquared = dot(lightVec, lightVec);
			float3 lightDir = normalize(lightVec);
			float spotEffect = dot(-lightDir, direction);
			float cosfValue = cosf(angle * 0.5f);
			if (spotEffect < cosfValue) return 0; // Outside of the spotlight cone
			float attenuation = 1.0f / distanceSquared;
			float falloffFactor = powf(spotEffect, falloff);
			return attenuation * falloffFactor;
		}

		// GetVisualizer: Returns a vector of lines to visualize the light
		std::vector<Line> GetVisualizer() const override {
			std::vector<Line> visualizer;

			const int segments = 12; // Number of segments to approximate the cone base
			float radius = tanf(angle * 0.5f); // Radius of the cone base at a unit distance
			float3 coneBaseCenter = position + direction; // Center of the cone base at a unit distance from the spotlight position
			float4 color = float4(Color, 1); // Color of the visualizer lines

			std::vector<float3> coneBasePoints;
			coneBasePoints.reserve(segments);

			// Calculate points around the cone base
			for (int i = 0; i < segments; ++i) {
				float newAngle = 2.0f * PI * i / segments; // Angle for this segment
				float3 localDir = cosf(newAngle) * normalize(cross(direction, float3(0, 1, 0))) + sinf(newAngle) * normalize(cross(direction, cross(direction, float3(0, 1, 0))));
				float3 point = coneBaseCenter + localDir * radius;
				coneBasePoints.push_back(point);

				// Draw line from spotlight position to this point
				visualizer.push_back(Line(position, point, color));
			}

			// Draw lines between adjacent points around the cone base to form the base circle
			for (int i = 0; i < segments; ++i) {
				int nextIndex = (i + 1) % segments; // Ensures the last point connects to the first
				visualizer.push_back(Line(coneBasePoints[i], coneBasePoints[nextIndex], color));
			}

			return visualizer;
		}


		float3 position; // Position of the spotlight in the scene
		float3 direction; // Direction in which the spotlight is aiming
		float angle; // Cutoff angle of the spotlight cone in radians
		float falloff; // Rate at which the light intensity decreases towards the edges of the cone
	};
} // namespace Tmpl8
