#include "modellistclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include <stdlib.h>
#include <time.h>

using namespace DirectX;

struct ModelInfoType {
	XMFLOAT4 color;
	float positionX, positionY, positionZ;
};

bool ModelListClass::Initialize(int numModels) {
	float red, green, blue;

	model_count_ = numModels;

	model_info_list_ = new ModelInfoType[model_count_];
	if (!model_info_list_) {
		return false;
	}

	// Seed the random generator with the current time.
	srand((unsigned int)time(NULL));

	// Go through all the models and randomly generate the model color and position.
	for (int i = 0; i < model_count_; i++) {
		// Generate a random color for the model.
		red = (float)rand() / RAND_MAX;
		green = (float)rand() / RAND_MAX;
		blue = (float)rand() / RAND_MAX;

		model_info_list_[i].color = XMFLOAT4(red, green, blue, 1.0f);

		// Generate a random position in front of the viewer for the mode.
		model_info_list_[i].positionX = (((float)rand() - (float)rand()) / RAND_MAX) * 10.0f;
		model_info_list_[i].positionY = (((float)rand() - (float)rand()) / RAND_MAX) * 10.0f;
		model_info_list_[i].positionZ = ((((float)rand() - (float)rand()) / RAND_MAX) * 10.0f) + 5.0f;
	}

	return true;
}

void ModelListClass::Shutdown() {

	if (model_info_list_) {
		delete[] model_info_list_;
		model_info_list_ = 0;
	}

}

int ModelListClass::GetModelCount() {
	return model_count_;
}

void ModelListClass::GetData(int index, float& positionX, float& positionY, float& positionZ, XMFLOAT4& color) {
	positionX = model_info_list_[index].positionX;
	positionY = model_info_list_[index].positionY;
	positionZ = model_info_list_[index].positionZ;

	color = model_info_list_[index].color;
}