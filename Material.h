#pragma once

class Material {
public:
	Material() = default;
	Material(const int index, const float roughness, const float metallic, const float transparency, const float ior)
		: index(index), roughness(roughness), metallic(metallic), transparency(transparency), ior(ior) {
		hasTexture = false;
	}

	int index = 0;                    // index of the material in the material list
	float roughness = 0.0f;           // 0.0 = mirror, 1.0 = lambert
	float metallic = 0.0f;            // 0.0 = dielectric, 1.0 = metal

	float3 emissionColor = float3(1); // Emission color
	float emissionIntensity = 0.0f;   // Emission intensity

	float transparency = 0.0f;        // 0.0 = opaque, 1.0 = fully transparent
	float ior = 1.0f;; 			  // Index of refraction

	float3 absorptionCoefficient = float3(1); // Absorption coefficient for transmitted light
	bool hasTexture = false;          // True if the material has a texture
	bool combineTexture = false;      // True if the texture should be combined with the base color
	std::shared_ptr<FLoatSurface> texture = nullptr; // Texture data

	//GetEmission: Returns the emission color and intensity of the material
	float3 GetEmission() const {
		return emissionColor * emissionIntensity;
	}

	float3 GetTextureColor(const float2& uv) const {
		if (texture == nullptr) {
			//return pink
			return float3(1.0f, 0.0f, 1.0f);
		}
		float3 color = texture->GetPixel(uv.x, uv.y);
		//convert the color to 0-1 range
		color /= 255.0f;

		return color;
	}

	// Returns an estimated reflectivity value based on roughness and metallic properties.
	float GetReflectivity(const float3& viewDir, const float3& normal) const {
		// Calculate half-way vector and normalize
		float3 halfDir = normalize(viewDir + reflect(viewDir, normal));

		// Calculate dot products
		float NdotH = dot(normal, halfDir);
		float VdotH = dot(viewDir, halfDir);

		// Check for grazing angles to avoid division by zero
		if (NdotH == 0.0f || VdotH == 0.0f) {
			return 0.0f;
		}

		// Calculate GGX distribution term
		float alpha = roughness * roughness;
		float D = (NdotH * NdotH) / ((NdotH * NdotH) + alpha * (1.0f - VdotH * VdotH));
		float G = NdotH / (8.0f * alpha * VdotH) * exp(-alpha * tan(acos(NdotH)) * tan(acos(NdotH)));

		// Calculate Fresnel term (Schlick approximation)
		float F0 = pow(1.0f - ior, 5.0f);
		float F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);

		// Calculate final reflectivity
		float result = D * G * F / (4.0f * NdotH * VdotH);

		// Clamp to valid range
		return clamp(result, 0.0f, 1.0f);
	}

	// Returns an estimated refractivity value based on the index of refraction and transparency.
	float GetRefractivity() const {
		float refactivity = ior;
		refactivity *= transparency;
		refactivity = max(0.0f, min(1.0f, refactivity));
		return refactivity;
	}

	// Assuming absorptionCoefficient is defined as a property of the material,
	// representing how much the material absorbs light (RGB) as it passes through.
	float3 GetTransmittedColor(const float3& incidentColor, float distance) const {
		if (transparency <= 0.0f) {
			return float3(0.0f); // Opaque material, no transmission
		}

		// Beer-Lambert Law for light attenuation through the material (expf is expensive)
		float3 attenuation = expf(-absorptionCoefficient * distance);

		// Apply transparency and attenuation to the incident light
		float3 transmittedColor = incidentColor * attenuation * transparency;
		return transmittedColor;
	}



	bool DrawImGui(int ImGuiIndex) {
		std::string reflectivityLabel = "Roughness##" + std::to_string(ImGuiIndex);
		std::string metallicityLabel = "Metallic##" + std::to_string(ImGuiIndex);
		std::string transparencyLabel = "Transparency##" + std::to_string(ImGuiIndex);
		std::string iorLabel = "IOR##" + std::to_string(ImGuiIndex);
		std::string emissionColorLabel = "Emission Color##" + std::to_string(ImGuiIndex);
		std::string emissionIntensityLabel = "Emission Intensity##" + std::to_string(ImGuiIndex);
		std::string absorptionCoefficientLabel = "Absorption Coefficient##" + std::to_string(ImGuiIndex);
		std::string hasTextureLabel = "Has Texture##" + std::to_string(ImGuiIndex);
		std::string combineTextureLabel = "Combine Colors##" + std::to_string(ImGuiIndex);

		bool modified = false;
		modified |= ImGui::SliderFloat(reflectivityLabel.c_str(), &roughness, 0.0f, 1.0f);
		modified |= ImGui::SliderFloat(metallicityLabel.c_str(), &metallic, 0.0f, 1.0f);
		modified |= ImGui::SliderFloat(transparencyLabel.c_str(), &transparency, 0.0f, 1.0f);
		modified |= ImGui::SliderFloat(iorLabel.c_str(), &ior, 1.0f, 2.5f); // Adjust max value as needed
		modified |= ImGui::ColorEdit3(emissionColorLabel.c_str(), &emissionColor.x);
		modified |= ImGui::DragFloat(emissionIntensityLabel.c_str(), &emissionIntensity, 0.01f);
		modified |= ImGui::ColorEdit3(absorptionCoefficientLabel.c_str(), &absorptionCoefficient.x);
		modified |= ImGui::Checkbox(hasTextureLabel.c_str(), &hasTexture);
		if (hasTexture) {
			modified |= ImGui::Checkbox(combineTextureLabel.c_str(), &combineTexture);

			//Read all the .png files in the assets folder and store them in a vector
			std::vector<std::string> hdrFiles;
			for (const auto& entry : std::filesystem::directory_iterator("assets")) {
				if (entry.path().extension() == ".png") {
					hdrFiles.push_back(entry.path().filename().string());
				}
			}

			// Dropdown for selecting .png file
			static int selectedItem = -1; // Index of the selected item in the combo box
			std::string label = "Files##" + std::to_string(ImGuiIndex);
			if (!hdrFiles.empty()) {
				std::vector<const char*> cStrHdrFiles; // ImGui needs const char* array
				for (const auto& file : hdrFiles) {
					cStrHdrFiles.push_back(file.c_str());
				}

				ImGui::Combo(label.c_str(), &selectedItem, &cStrHdrFiles[0], static_cast<int>(cStrHdrFiles.size()));
			}

			std::string loadLabel = "Load File##" + std::to_string(ImGuiIndex);
			if (ImGui::Button(loadLabel.c_str()) && selectedItem >= 0) {
				// Load the selected .png file
				std::string path = "assets/" + hdrFiles[selectedItem];
				if (texture == nullptr) {
					texture = std::make_unique<FLoatSurface>();
				} else {
					if (texture->ownBuffer) FREE64(texture->pixels);
				}
				texture->LoadFromFile(path.c_str());
				//print the size of the pixels buffer
				std::cout << "Size of the pixels buffer: " << texture->width << "x" << texture->height << std::endl;
				modified = true;
			}
		}
		return modified;
	}

	//Create an == operator for the Material class
	bool operator==(const Material& other) const {
		return roughness == other.roughness && metallic == other.metallic &&
			transparency == other.transparency && ior == other.ior;
	}
};
