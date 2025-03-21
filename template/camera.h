#pragma once
#include "SvenUtils/InputManager.h"
// default screen resolution

//#define FULLSCREEN
// #define DOUBLESIZE

namespace Tmpl8 {


	class Camera {
	public:
		Camera() {
			// setup a basic view frustum
			camPos = float3(1, 1, 1);
			camTarget = float3(0, 0, -1);
			topLeft = float3(-aspect, 1, 0);
			topRight = float3(aspect, 1, 0);
			bottomLeft = float3(-aspect, -1, 0);
			bottomRight = float3(aspect, -1, 0);

			fov = 0.521f;
			UpdateProjection();
		}
		Ray Camera::GetPrimaryRay(const float x, const float y) const {
			if (paniniEffect) return GetPaniniEffectPrimaryRay(x, y);
			if (!depthOfField) return GetNoEffectPrimaryRay(x, y);

			// Calculate pixel position on virtual screen plane
			const float u = x * (1.0f / SCRWIDTH);
			const float v = y * (1.0f / SCRHEIGHT);

			const float3 pixelPosition = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);

			// Direction from camera to pixel position
			float3 initialDir = normalize(pixelPosition - camPos);

			// Calculate intersection with the focal plane
			float3 focalPlaneNormal = normalize(camTarget - camPos); // Assuming camTarget defines the viewing direction
			float intersectionDistance = focalDistance / dot(focalPlaneNormal, initialDir);
			float3 focalPoint = camPos + initialDir * intersectionDistance;

			// Random point on the lens
			float3 lensPoint = camPos + SampleHexagon() * lensRadius;

			// New ray direction from lens point to focal point
			float3 newDir = normalize(focalPoint - lensPoint);

			return Ray(lensPoint, newDir);
		}

		void GetPrimaryRaysSIMD(__m256 xValues, __m256 yValues, Ray* rays) const {
			//no effect only
			__m256 uValues = _mm256_mul_ps(xValues, invWidth);
			__m256 vValues = _mm256_mul_ps(yValues, invHeight);

			// topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft)
			__m256 pixelPositionXValues = _mm256_add_ps(topLeftX, _mm256_add_ps(_mm256_mul_ps(uValues, _mm256_sub_ps(topRightX, topLeftX)), _mm256_mul_ps(vValues, _mm256_sub_ps(bottomLeftX, topLeftX))));
			__m256 pixelPositionYValues = _mm256_add_ps(topLeftY, _mm256_add_ps(_mm256_mul_ps(uValues, _mm256_sub_ps(topRightY, topLeftY)), _mm256_mul_ps(vValues, _mm256_sub_ps(bottomLeftY, topLeftY))));
			__m256 pixelPositionZValues = _mm256_setzero_ps();

			// pixelPosition - camPos
			__m256 directionXValues = _mm256_sub_ps(pixelPositionXValues, camPosX);
			__m256 directionYValues = _mm256_sub_ps(pixelPositionYValues, camPosY);
			__m256 directionZValues = _mm256_sub_ps(pixelPositionZValues, camPosZ);

			// normalize(pixelPosition - camPos)
			__m256 invLengths = _mm256_rsqrt_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(directionXValues, directionXValues), _mm256_mul_ps(directionYValues, directionYValues)), _mm256_mul_ps(directionZValues, directionZValues)));
			directionXValues = _mm256_mul_ps(directionXValues, invLengths);
			directionYValues = _mm256_mul_ps(directionYValues, invLengths);
			directionZValues = _mm256_mul_ps(directionZValues, invLengths);

			// Store the rays in the output array
			for (int i = 0; i < 8; i++) {
				// Extract the ith components of direction vectors
				float dx = ((float*)&directionXValues)[i];
				float dy = ((float*)&directionYValues)[i];
				float dz = ((float*)&directionZValues)[i];

				// Construct the direction vector for the ray
				float3 direction = float3(dx, dy, dz);

				rays[i] = Ray(camPos, direction);
			}
		}

		Ray Camera::GetNoEffectPrimaryRay(const float x, const float y) const {
			// Calculate pixel position on virtual screen plane (as before)
			const float u = x * (1.0f / SCRWIDTH);
			const float v = y * (1.0f / SCRHEIGHT);
			const float3 pixelPosition = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
			const float3 direction = normalize(pixelPosition - camPos);
			return Ray(camPos, direction);
		}

		Ray Camera::GetPaniniEffectPrimaryRay(const float x, const float y) const {
			// Convert screen coordinates to normalized device coordinates (NDC) with origin at the center of the screen
			float ndcX = (x / SCRWIDTH) * 2.0f - 1.0f; // Range [-1, 1]
			float ndcY = 1.0f - (y / SCRHEIGHT) * 2.0f; // Range [1, -1], flipped Y-axis

			// Panini projection parameters
			float d = paniniDistance; // Distance from the viewer to the projection plane, usually 1.0
			float s = paniniSqueeze; // Squeeze parameter, 0 for cylindrical projection

			// Calculate the radial distance from the center of the NDC
			float radialDist = sqrt(ndcX * ndcX + ndcY * ndcY);

			// Apply the Panini projection formula
			float paniniFactor = d + s * (sqrt(1.0f + radialDist * radialDist) - 1.0f) / (d + 1.0f);
			float paniniX = ndcX * paniniFactor;
			float paniniY = ndcY * paniniFactor;

			// Calculate the camera's right and up vectors
			float3 ahead = normalize(camTarget - camPos);
			float3 tmpUp(0, 1, 0);
			float3 right = normalize(cross(tmpUp, ahead));
			float3 up = normalize(cross(ahead, right));

			// Calculate the direction for the primary ray after Panini projection
			// Ensure the direction calculation takes into account the camera orientation
			float3 dir = normalize((paniniX * right + paniniY * up + 2.0f * ahead));

			// Return the ray originating from the camera position in the direction calculated
			return Ray(camPos, dir);
		}


		void Camera::UpdateProjection() {
			float3 ahead = normalize(camTarget - camPos);
			float3 tmpUp(0, 1, 0);
			float3 right = normalize(cross(tmpUp, ahead));
			float3 up = normalize(cross(ahead, right));
			float fovScale = tan(fov * 0.5f);
			topLeft = camPos + 2 * ahead - fovScale * aspect * right + fovScale * up;
			topRight = camPos + 2 * ahead + fovScale * aspect * right + fovScale * up;
			bottomLeft = camPos + 2 * ahead - fovScale * aspect * right - fovScale * up;
			bottomRight = camPos + 2 * ahead + fovScale * aspect * right - fovScale * up;

			topLeftX = _mm256_set1_ps(topLeft.x);
			topLeftY = _mm256_set1_ps(topLeft.y);
			topRightX = _mm256_set1_ps(topRight.x);
			topRightY = _mm256_set1_ps(topRight.y);
			bottomLeftX = _mm256_set1_ps(bottomLeft.x);
			bottomLeftY = _mm256_set1_ps(bottomLeft.y);

			camPosX = _mm256_set1_ps(camPos.x);
			camPosY = _mm256_set1_ps(camPos.y);
			camPosZ = _mm256_set1_ps(camPos.z);
		}

		float HandleInput(const float t) {
			if (!WindowHasFocus()) return false;
			float speed = camSpeed * t;
			float3 ahead = normalize(camTarget - camPos);
			float3 tmpUp(0, 1, 0);
			float3 right = normalize(cross(tmpUp, ahead));
			float3 up = normalize(cross(ahead, right));

			bool changed = false;
			bool movedForward = false;
			bool forward = true;
			if (IsKeyDown(GLFW_KEY_A)) camPos -= speed * 2 * right, changed = true;
			if (IsKeyDown(GLFW_KEY_D)) camPos += speed * 2 * right, changed = true;
			if (IsKeyDown(GLFW_KEY_W)) camPos += speed * 2 * ahead, changed = true; movedForward = true;
			if (IsKeyDown(GLFW_KEY_S)) camPos -= speed * 2 * ahead, changed = true; movedForward = true; forward = false;
			if (IsKeyDown(GLFW_KEY_E)) camPos += speed * 2 * up, changed = true;
			if (IsKeyDown(GLFW_KEY_Q)) camPos -= speed * 2 * up, changed = true;
			camTarget = camPos + ahead;

			//insyead of using buttons we use the mouse delta to look around
			if (InputManager::GetInstance().GetMouseButton(1)) {
				Vector2 mouseDeltaVector = InputManager::GetInstance().GetMouseDelta();
				float2 mouseDelta = float2(mouseDeltaVector.x, mouseDeltaVector.y);

				// Calculate pitch based on mouse Y delta
				float pitchAngle = atan2f(ahead.y, sqrtf(ahead.x * ahead.x + ahead.z * ahead.z)) - mouseDelta.y * 0.005f;
				// Clamp pitch angle to prevent flipping
				const float maxPitch = PI / 2.0f - EPSILON; // Just under 90 degrees in radians
				pitchAngle = max(-maxPitch, min(maxPitch, pitchAngle));

				// Calculate new ahead vector based on clamped pitch and yaw
				float yawAngle = atan2f(ahead.x, ahead.z) + mouseDelta.x * 0.005f;
				ahead = float3(sinf(yawAngle) * cosf(pitchAngle), sinf(pitchAngle), cosf(yawAngle) * cosf(pitchAngle));
				ahead = normalize(ahead);

				// Update camera target based on new ahead vector
				camTarget = camPos + ahead;
				changed = true;
			}
			if (!changed) return false;
			fov = clamp(fov, 0.1f, PI - 0.1f);

			ahead = normalize(camTarget - camPos);
			up = normalize(cross(ahead, right));
			right = normalize(cross(up, ahead));
			float fovScale = tan(fov * 0.5f);
			topLeft = camPos + 2 * ahead - fovScale * aspect * right + fovScale * up;
			topRight = camPos + 2 * ahead + fovScale * aspect * right + fovScale * up;
			bottomLeft = camPos + 2 * ahead - fovScale * aspect * right - fovScale * up;
			bottomRight = camPos + 2 * ahead + fovScale * aspect * right - fovScale * up;

			topLeftX = _mm256_set1_ps(topLeft.x);
			topLeftY = _mm256_set1_ps(topLeft.y);
			topRightX = _mm256_set1_ps(topRight.x);
			topRightY = _mm256_set1_ps(topRight.y);
			bottomLeftX = _mm256_set1_ps(bottomLeft.x);
			bottomLeftY = _mm256_set1_ps(bottomLeft.y);

			camPosX = _mm256_set1_ps(camPos.x);
			camPosY = _mm256_set1_ps(camPos.y);
			camPosZ = _mm256_set1_ps(camPos.z);

			if (movedForward) return forward ? speed : -speed;
			return 0;
		}

		//Credit to Robin
		float2 Camera::WorldSpaceToScreenSpace(const float3 position_ws, const float3 camera_position, const float3 camera_target) {
			float3 to_point = position_ws - camera_position;
			float3 to_point_nrm = normalize(to_point);

			float3 camera_dir = normalize(camera_target - camera_position);
			float3 view_right = normalize(cross(float3(0.0, 1.0, 0.0), camera_dir));
			float3 view_up = cross(camera_dir, view_right);

			float3 fwd = camera_dir * 2.0f;

			float d = max(dot(camera_dir, to_point_nrm), EPSILON);
			d = 2.0f / d;

			to_point = to_point_nrm * d - fwd;

			float x = dot(to_point, view_right);
			float y = dot(to_point, view_up);

			// Incorporate FOV in the projection (fov in radians)
			float tanHalfFOV = tan(fov / 2.0f);
			x /= (tanHalfFOV * aspect);
			y /= tanHalfFOV;

			float2 reproj_uv01 = float2(x, y);
			reproj_uv01 = reproj_uv01 * 0.5f + 0.5f;

			reproj_uv01.y = 1.0f - reproj_uv01.y;

			return reproj_uv01;
		}

		//rotate function
		float3 rotate(float3 v, float a, float3 axis) {
			float s = sin(a);
			float c = cos(a);
			float3 result;
			result.x = (c + (1 - c) * axis.x * axis.x) * v.x + ((1 - c) * axis.x * axis.y - axis.z * s) * v.y + ((1 - c) * axis.x * axis.z + axis.y * s) * v.z;
			result.y = ((1 - c) * axis.x * axis.y + axis.z * s) * v.x + (c + (1 - c) * axis.y * axis.y) * v.y + ((1 - c) * axis.y * axis.z - axis.x * s) * v.z;
			result.z = ((1 - c) * axis.x * axis.z - axis.y * s) * v.x + ((1 - c) * axis.y * axis.z + axis.x * s) * v.y + (c + (1 - c) * axis.z * axis.z) * v.z;
			return result;
		}

		bool DrawImgui() {
			bool changed = false;
			if (!paniniEffect) {
				if (ImGui::DragFloat("FOV", &fov, 0.01f, 0, PI)) {
					changed = true;
					UpdateProjection();
				}
			}
			//near and far plane
			changed |= ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.0f, 10.0f);
			changed |= ImGui::SliderFloat("Far Plane", &farPlane, 10.0f, 1000.0f);

			changed |= ImGui::Checkbox("Panini Projection", &paniniEffect);
			if (paniniEffect) {
				changed |= ImGui::SliderFloat("Panini Squeeze", &paniniSqueeze, 0, 5);
				changed |= ImGui::SliderFloat("Panini Distance", &paniniDistance, 0, 2);
			}
			changed |= ImGui::Checkbox("Depth of Field", &depthOfField);
			if (!depthOfField) return changed;
			changed |= ImGui::SliderFloat("Lens Radius", &lensRadius, 0, 0.2f);
			changed |= ImGui::SliderFloat("Focal Distance", &focalDistance, 0, 100);
			return changed;
		}

		mat4 Camera::GetViewMatrix() {
			float3 up = float3(0, 1, 0); // World's up vector
			float3 zaxis = normalize(camPos - camTarget); // The "forward" vector.
			float3 xaxis = normalize(cross(up, zaxis)); // The "right" vector.
			float3 yaxis = cross(zaxis, xaxis);     // The "up" vector.

			// Create a 4x4 view matrix from the right, up, forward and eye position vectors
			mat4 viewMatrix = mat4(
				xaxis.x, yaxis.x, zaxis.x, 0,
				xaxis.y, yaxis.y, zaxis.y, 0,
				xaxis.z, yaxis.z, zaxis.z, 0,
				-dot(xaxis, camPos), -dot(yaxis, camPos), -dot(zaxis, camPos), 1
			);

			return viewMatrix;
		}

		mat4 Camera::GetProjectionMatrix() {
			float yScale = 1.0f / tan(fov / 2.0f); // Scale the y coordinates
			float xScale = yScale / aspect; // Scale the x coordinates to maintain aspect ratio
			float frustumLength = farPlane - nearPlane;

			mat4 projMatrix = mat4(
				xScale, 0, 0, 0,
				0, yScale, 0, 0,
				0, 0, -((farPlane + nearPlane) / frustumLength), -1,
				0, 0, -((2 * nearPlane * farPlane) / frustumLength), 0
			);

			return projMatrix;
		}

		float3 GetCameraRotation() {
			float3 ahead = normalize(camTarget - camPos);
			return float3(atan2(ahead.z, ahead.x), atan2(ahead.y, sqrt(ahead.x * ahead.x + ahead.z * ahead.z)), 0);
		}


		float3 GetCameraDirection() {
			return (camTarget - camPos);
		}

		//return the distance between the camera pos and the previous camera pos
		float GetCameraDelta() {
			return length(prevCamPos - camPos);
		}

		void UpdatePrevState() {
			prevCamPos = camPos;
			prevCamTarget = camTarget;
			prevTopLeft = topLeft;
			prevTopRight = topRight;
			prevBottomLeft = bottomLeft;
			prevBottomRight = bottomRight;
		}

		int2 Camera::Reproject(const float3& worldPos) const {
			float3 O_LU = prevTopLeft - prevCamPos;
			float3 O_RB = prevBottomRight - prevCamPos;

			float3 LFnormal = normalize(cross(O_LU, prevBottomLeft - prevTopLeft));

			float3 UFnormal = normalize(cross(O_LU, prevTopRight - prevTopLeft));

			float3 BFnormal = normalize(cross(O_RB, prevBottomLeft - prevBottomRight));

			float3 RFnormal = normalize(cross(O_RB, prevTopRight - prevBottomRight));

			float LF_d = dot(LFnormal, prevCamPos);
			float UF_d = dot(UFnormal, prevCamPos);
			float BF_d = dot(BFnormal, prevCamPos);
			float RF_d = dot(RFnormal, prevCamPos);

			float d1_x = dot(LFnormal, worldPos) - LF_d;
			float d2_x = dot(RFnormal, worldPos) - RF_d;

			float d1_y = dot(UFnormal, worldPos) - UF_d;
			float d2_y = dot(BFnormal, worldPos) - BF_d;

			float u = d1_x / (d1_x + d2_x);
			float v = d1_y / (d1_y + d2_y);

			return int2(static_cast<int>(roundf((u * SCRWIDTH))), static_cast<int>(roundf((v * SCRHEIGHT))));
		}

		float3 GetForward() {
			return normalize(camTarget - camPos);
		}

		float3 GetRotation() {
			float3 forward = normalize(camTarget - camPos);
			return float3(atan2(forward.z, forward.x), atan2(forward.y, sqrt(forward.x * forward.x + forward.z * forward.z)), 0);
		}


		float3 camPos, camTarget;
		float3 topLeft, topRight, bottomLeft, bottomRight;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;

		float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;

		float3 prevCamPos, prevCamTarget;
		float3 prevTopLeft, prevTopRight, prevBottomLeft, prevBottomRight;

		float camSpeed = 0.000545f;

		bool depthOfField = false;
		bool paniniEffect = false;
		char dummy[2];

		float lensRadius = 0.1f;
		float focalDistance = 1.0f;
		float fov = 0.521f;

		float paniniSqueeze = 0.0f;
		float paniniDistance = 1.0f;
		float2 dummy2;

		const __m256 invWidth = _mm256_set1_ps(1.0f / SCRWIDTH);
		const __m256 invHeight = _mm256_set1_ps(1.0f / SCRHEIGHT);
		__m256 topLeftX;
		__m256 topLeftY;
		__m256 topRightX;
		__m256 topRightY;
		__m256 bottomLeftX;
		__m256 bottomLeftY;

		__m256 camPosX;
		__m256 camPosY;
		__m256 camPosZ;
	};

}
