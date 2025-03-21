#pragma once
enum ToneMappingType {
	Reinhard,
	ReinhardModified,
	Uncharted2,
	ACES,
	None
};

// Function to convert ToneMappingType to a string for display
inline const char* ToneMappingTypeToString(ToneMappingType type) {
	switch (type) {
	case Reinhard: return "Reinhard";
	case ReinhardModified: return "Reinhard Modified";
	case Uncharted2: return "Uncharted 2";
	case ACES: return "ACES";
	case None: return "None";
	default: return "Unknown";
	}
}

// Function to display the dropdown and allow the user to select a ToneMappingType
inline bool DisplayToneMappingDropdown(const char* label, ToneMappingType& currentType) {
	bool changed = false;
	if (ImGui::BeginCombo(label, ToneMappingTypeToString(currentType))) {
		for (int i = Reinhard; i <= None; ++i) {
			ToneMappingType type = static_cast<ToneMappingType>(i);
			bool isSelected = (currentType == type);
			if (ImGui::Selectable(ToneMappingTypeToString(type), isSelected)) {
				currentType = type;
				changed |= true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return changed;
}

static float3 ApplyReinhard(const float3 color, const float exposure) {
	return color / (color + 1.0f) * exposure;
}

static float3 ApplyReinhardModified(const float3 color, const  float exposure) {
	return color / (color + 1.0f) * exposure / (exposure + 1.0f);
}

static float3 ApplyUncharted2(const float3 color, const  float exposure) {
#if 0
	__m256 A = _mm256_set1_ps(0.15f);
	__m256 B = _mm256_set1_ps(0.50f);
	__m256 C = _mm256_set1_ps(0.10f);
	__m256 D = _mm256_set1_ps(0.20f);
	__m256 E = _mm256_set1_ps(0.02f);
	__m256 F = _mm256_set1_ps(0.30f);
	__m256 Exp = _mm256_set1_ps(exposure);

	__m256 col = _mm256_set_ps(color.z, color.z, color.z, color.y, color.y, color.y, color.x, color.x);
	__m256 term1 = _mm256_add_ps(_mm256_mul_ps(A, col), _mm256_mul_ps(C, B));
	__m256 term2 = _mm256_add_ps(_mm256_mul_ps(A, col), B);
	__m256 term3 = _mm256_add_ps(_mm256_mul_ps(col, _mm256_add_ps(_mm256_mul_ps(E, col), _mm256_mul_ps(F, D))), _mm256_mul_ps(col, _mm256_add_ps(_mm256_mul_ps(E, col), D)) + F * B);

	__m256 result = _mm256_mul_ps(_mm256_div_ps(_mm256_mul_ps(col, term1), _mm256_add_ps(_mm256_mul_ps(col, term2), _mm256_mul_ps(C, D))), _mm256_add_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(Exp, term3)));

	// Extract the results back to float3
	float3 res;
	res.x = _mm256_cvtss_f32(result); // Extract the lowest element
	result = _mm256_permute4x64_ps(result, _MM_SHUFFLE(3, 2, 3, 2)); // Shuffle to bring the next element to the lowest
	res.y = _mm256_cvtss_f32(result); // Extract the now lowest element
	result = _mm256_permute4x64_ps(result, _MM_SHUFFLE(3, 2, 3, 2)); // Shuffle to bring the final element to the lowest
	res.z = _mm256_cvtss_f32(result); // Extract the now lowest element

	return res;
#else
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	return (color * (A * color + C * B)) / (color * (A * color + B) + C * D) * (1 + (color * (E * color + F * D)) / (color * (E * color + D) + F * B)) * exposure;
#endif
}

static float3 ApplyACES(const float3 color, const  float exposure) {
	float3 a = color * (color * 2.51f + 0.03f);
	float3 b = color * (color * 3.31f + 0.00f);
	float3 c = color * (color * 0.21f + 0.00f);
	return (a / (a + b + c)) * exposure;
}

static float3 ApplyNone(const float3 color, const  float exposure) {
	return color * exposure;
}

inline float3 ToneMapping(const float3 color, const float exposure, const ToneMappingType type) {
	switch (type) {
	case Reinhard:
		return ApplyReinhard(color, exposure);
	case ReinhardModified:
		return ApplyReinhardModified(color, exposure);
	case Uncharted2:
		return ApplyUncharted2(color, exposure);
	case ACES:
		return ApplyACES(color, exposure);
	case None:
		return ApplyNone(color, exposure);
	default:
		return color;
	}
}