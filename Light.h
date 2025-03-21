#pragma once

namespace Tmpl8 {
	class Light {
	public:
		Light(const float3& color, float intensity) :
			Color(color), Intensity(intensity) {
		}
		virtual ~Light() = default;
		virtual float3 GetContribution(
			const Scene&,
			const Ray&,
			const float3&,
			const float3&) {
			return Color * GetIntensity();
		}
		virtual bool DrawImgui(int index) {
			std::string colorLabel = "Color##" + std::to_string(index);
			std::string intensityLabel = "Intensity##" + std::to_string(index);

			bool changed = false;
			changed |= ImGui::ColorEdit3(colorLabel.c_str(), &Color.x);
			changed |= ImGui::DragFloat(intensityLabel.c_str(), &Intensity, 0.001f, 0.0f);
			return changed;
		}

		float GetIntensity() const { return Intensity * PI; }
		virtual float GetIntensity(const float3&) const { return Intensity; }
		virtual std::vector<Line> GetVisualizer() const { return std::vector<Line>(); }

		//although this is not the best encapsulation
		// I could create getters and setters for color and intensity
		//but decided to make them public for simplicity
		float3 Color;
		float Intensity;
	};
}
