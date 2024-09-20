#include "Frustum.h"

using namespace DirectX;

void FrustumClass::ConstructFrustum(float screenDepth, const XMMATRIX& projectionMatrix, const XMMATRIX& viewMatrix) {

	XMMATRIX projectionMatrixCopy = projectionMatrix;

	// Calculate the minimum Z distance in the frustum.
	float zMinimum = -projectionMatrixCopy.r[3].m128_f32[2] / projectionMatrixCopy.r[2].m128_f32[2];
	float r = screenDepth / (screenDepth - zMinimum);
	projectionMatrixCopy.r[2].m128_f32[2] = r;
	projectionMatrixCopy.r[3].m128_f32[2] = -r * zMinimum;

	// Create the frustum matrix from the view matrix and updated projection matrix.
	auto matrix = XMMatrixMultiply(viewMatrix, projectionMatrixCopy);

	// Calculate near plane of frustum.
	planes_[0].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[2];
	planes_[0].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[2];
	planes_[0].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[2];
	planes_[0].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[2];
	planes_[0] = XMPlaneNormalize(planes_[0]);

	// Calculate far plane of frustum.
	planes_[1].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[2];
	planes_[1].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[2];
	planes_[1].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[2];
	planes_[1].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[2];
	planes_[1] = XMPlaneNormalize(planes_[1]);

	// Calculate left plane of frustum.
	planes_[2].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[0];
	planes_[2].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[0];
	planes_[2].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[0];
	planes_[2].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[0];
	planes_[2] = XMPlaneNormalize(planes_[2]);

	// Calculate right plane of frustum.
	planes_[3].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[0];
	planes_[3].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[0];
	planes_[3].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[0];
	planes_[3].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[0];
	planes_[3] = XMPlaneNormalize(planes_[3]);

	// Calculate top plane of frustum.
	planes_[4].m128_f32[0] = matrix.r[0].m128_f32[3] - matrix.r[0].m128_f32[1];
	planes_[4].m128_f32[1] = matrix.r[1].m128_f32[3] - matrix.r[1].m128_f32[1];
	planes_[4].m128_f32[2] = matrix.r[2].m128_f32[3] - matrix.r[2].m128_f32[1];
	planes_[4].m128_f32[3] = matrix.r[3].m128_f32[3] - matrix.r[3].m128_f32[1];
	planes_[4] = XMPlaneNormalize(planes_[4]);

	// Calculate bottom plane of frustum.
	planes_[5].m128_f32[0] = matrix.r[0].m128_f32[3] + matrix.r[0].m128_f32[1];
	planes_[5].m128_f32[1] = matrix.r[1].m128_f32[3] + matrix.r[1].m128_f32[1];
	planes_[5].m128_f32[2] = matrix.r[2].m128_f32[3] + matrix.r[2].m128_f32[1];
	planes_[5].m128_f32[3] = matrix.r[3].m128_f32[3] + matrix.r[3].m128_f32[1];
	planes_[5] = XMPlaneNormalize(planes_[5]);
}

bool FrustumClass::CheckPoint(float x, float y, float z) {

	XMVECTOR tempVector{};
	XMFLOAT3 temp(x, y, z);

	// Check if the point is inside all six planes of the view frustum.
	for (int i = 0; i < 6; i++) {
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] < 0.0f) {
			return false;
		}
	}

	return true;
}

bool FrustumClass::CheckCube(float xCenter, float yCenter, float zCenter, float radius) {

	XMVECTOR tempVector{};
	XMFLOAT3 temp{};

	// Check if any one point of the cube is in the view frustum.
	for (int i = 0; i < 6; i++) {

		temp.x = xCenter - radius;
		temp.y = yCenter - radius;
		temp.z = zCenter - radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + radius;
		temp.y = yCenter - radius;
		temp.z = zCenter - radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - radius;
		temp.y = yCenter + radius;
		temp.z = zCenter - radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + radius;
		temp.y = yCenter + radius;
		temp.z = zCenter - radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - radius;
		temp.y = yCenter - radius;
		temp.z = zCenter + radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + radius;
		temp.y = yCenter - radius;
		temp.z = zCenter + radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - radius;
		temp.y = yCenter + radius;
		temp.z = zCenter + radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + radius;
		temp.y = yCenter + radius;
		temp.z = zCenter + radius;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		return false;
	}

	return true;
}

bool FrustumClass::CheckSphere(float xCenter, float yCenter, float zCenter, float radius) {

	XMVECTOR tempVector{};
	XMFLOAT3 temp(xCenter, yCenter, zCenter);

	// Check if the radius of the sphere is inside the view frustum.
	for (int i = 0; i < 6; i++) {
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] < -radius) {
			return false;
		}
	}

	return true;
}

bool FrustumClass::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize) {

	XMVECTOR tempVector{};
	XMFLOAT3 temp{};

	// Check if any of the 6 planes of the rectangle are inside the view frustum.
	for (int i = 0; i < 6; i++) {

		temp.x = xCenter - xSize;
		temp.y = yCenter - ySize;
		temp.z = zCenter - zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + xSize;
		temp.y = yCenter - ySize;
		temp.z = zCenter - zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - xSize;
		temp.y = yCenter + ySize;
		temp.z = zCenter - zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - xSize;
		temp.y = yCenter - ySize;
		temp.z = zCenter + zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + xSize;
		temp.y = yCenter + ySize;
		temp.z = zCenter - zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + xSize;
		temp.y = yCenter - ySize;
		temp.z = zCenter + zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter - xSize;
		temp.y = yCenter + ySize;
		temp.z = zCenter + zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		temp.x = xCenter + xSize;
		temp.y = yCenter + ySize;
		temp.z = zCenter + zSize;
		tempVector = XMPlaneDotCoord(planes_[i], XMLoadFloat3(&temp));

		if (tempVector.m128_f32[0] >= 0.0f) {
			continue;
		}

		return false;
	}

	return true;
}
