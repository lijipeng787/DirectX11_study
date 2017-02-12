#include "user_interface.h"

#define OBJECT_INITIALIZE_AND_TEST(obj_name,class_name)	\
		(obj_name)=new (class_name);					\
		if(!obj_name){return false;}					\

#define OBJECT_ARR_CREATE_AND_TEST(obj_name,class_name,size)	\
		auto obj_name=new class_name[size];						\
		if(!obj_name){return false;}							\
		ZeroMemory(obj_name,sizeof(obj_name));					\

UserInterface::UserInterface() :font_object_(nullptr), fps_string_(nullptr),
video_string_(nullptr), position_string_(nullptr), previous_fps_(-1) {
}

UserInterface::UserInterface(const UserInterface & copy){
}

UserInterface::~UserInterface(){
}

bool UserInterface::Initialize(int screen_width, int screen_height){
	OBJECT_INITIALIZE_AND_TEST(font_object_, Font);

	auto d3dclass = D3d11Device::get_d3d_object();

	if (!(font_object_->Initialize("../Engine/data/font/font01.txt", "../Engine/data/font/font01.tga", 32.0f, 3))) {
		return false;
	}

	OBJECT_INITIALIZE_AND_TEST(fps_string_, Text);

	if (!(fps_string_->Initialize(screen_width, screen_height, 16, false, font_object_, "FPS:0", 10, 50, 0.0f, 1.0f, 0.0f))) {
		return false;
	}

	char video_card_info[128] = { 0 };
	auto video_card_memory_capacity = 0;
	d3dclass->get_videocard_info(video_card_info, video_card_memory_capacity);

	char video_card_info_string[144] = { 0 };
	strcpy_s(video_card_info_string, "video card: ");
	strcat_s(video_card_info_string, video_card_info);

	char temp_char[16] = { 0 };
	_itoa_s(video_card_memory_capacity, temp_char, 10);
	char video_card_memory_string[32] = { 0 };
	strcpy_s(video_card_memory_string, "video memory: ");
	strcat_s(video_card_memory_string, temp_char);
	strcat_s(video_card_memory_string, "MB");

	video_string_ = new Text[2];
	if (!video_string_) {
		return false;
	}

	if (!(video_string_[0].Initialize(screen_width, screen_height, 256, false, font_object_,
		video_card_info_string, 10, 10, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(video_string_[1].Initialize(screen_width, screen_height, 32, false, font_object_,
		video_card_memory_string, 10, 30, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	position_string_ = new Text[6];
	if (!position_string_) {
		return false;
	}

	if (!(position_string_[0].Initialize(screen_width, screen_height, 16, false, font_object_,
		"x: 0", 10, 310, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(position_string_[1].Initialize(screen_width, screen_height, 16, false, font_object_,
		"y: 0", 10, 330, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(position_string_[2].Initialize(screen_width, screen_height, 16, false, font_object_,
		"z: 0", 10, 350, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(position_string_[3].Initialize(screen_width, screen_height, 16, false, font_object_,
		"rx: 0", 10, 370, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(position_string_[4].Initialize(screen_width, screen_height, 16, false, font_object_,
		"ry: 0", 10, 390, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	if (!(position_string_[5].Initialize(screen_width, screen_height, 16, false, font_object_,
		"rz: 0", 10, 410, 1.0f, 1.0f, 1.0f))) {
		return false;
	}

	return true;
}

void UserInterface::Shutdown() {

	if (position_string_) {
		position_string_[0].Shutdown();
		position_string_[1].Shutdown();
		position_string_[2].Shutdown();
		position_string_[3].Shutdown();
		position_string_[4].Shutdown();
		position_string_[5].Shutdown();

		delete[] position_string_;
		position_string_ = 0;
	}

	if (video_string_) {
		video_string_[0].Shutdown();
		video_string_[1].Shutdown();

		delete[] video_string_;
		video_string_ = nullptr;
	}

	if (fps_string_) {
		fps_string_->Shutdown();
		delete fps_string_;
		fps_string_ = nullptr;
	}

	if (font_object_) {
		font_object_->Shutdown();
		delete font_object_;
		font_object_ = nullptr;
	}
}

bool UserInterface::Frame(int fps, float pos_x, float pos_y, float pos_z,
	float rot_x, float rot_y, float rot_z){

	if (!(UpdateFpsString(fps)))
		return false;

	if (!(UpdatePosisitonString(pos_x, pos_y, pos_z, rot_x, rot_y, rot_z)))
		return false;

	return true;
}

bool UserInterface::Render(ShaderManager * shader_manager_object, 
	DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX orhthgonality){

	auto d3dclass = D3d11Device::get_d3d_object();

	d3dclass->TurnZBufferOff();
	d3dclass->TurnAlphaBlendingOn();

	fps_string_->Render(shader_manager_object, world, view, orhthgonality, font_object_->GetTexture());

	video_string_[0].Render(shader_manager_object, world, view, orhthgonality, font_object_->GetTexture());
	video_string_[1].Render(shader_manager_object, world, view, orhthgonality, font_object_->GetTexture());

	for (int i = 0; i < 6; ++i) {
		position_string_[i].Render(shader_manager_object, world, view, orhthgonality, font_object_->GetTexture());
	}

	d3dclass->TurnAlphaBlendingOff();

	d3dclass->TurnZBufferOn();

	return true;
}

bool UserInterface::UpdateFpsString(int fps_value){
	if (fps_value == previous_fps_)
		return true;

	previous_fps_ = fps_value;

	if (fps_value > 9999)
		fps_value = 9999;

	char temp_char[16] = { 0 };
	_itoa_s(fps_value, temp_char, 10);
	char fps_char[16] = { 0 };
	strcpy_s(fps_char, "fps:");
	strcat_s(fps_char, temp_char);

	auto red = 0.0f;
	auto green = 0.0f;
	auto blue = 0.0f;

	if (60 <= fps_value) {
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}
	else if (60 > fps_value && fps_value >= 30) {
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}
	else if (30 > fps_value) {
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	if (!(fps_string_->UpdateSentence(font_object_, fps_char, 10, 50, red, green, blue))) {
		return false;
	}

	return true;
}

bool UserInterface::UpdatePosisitonString(float pos_x, float pos_y, float pos_z, 
	float rot_x, float rot_y, float rot_z){
	char temp_string[16] = { 0 };
	char final_string[16] = { 0 };

	if (previous_position_[0] != static_cast<int>(pos_x)) {
		previous_position_[0] = static_cast<int>(pos_x);
		_itoa_s(static_cast<int>(pos_x), temp_string, 10);
		strcpy_s(final_string, "x:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[0].UpdateSentence(font_object_, final_string,
			10, 100, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	if (previous_position_[1] != static_cast<int>(pos_y)) {
		previous_position_[1] = static_cast<int>(pos_y);
		_itoa_s(static_cast<int>(pos_y), temp_string, 10);
		strcpy_s(final_string, "y:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[1].UpdateSentence(font_object_, final_string,
			10, 120, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	if (previous_position_[2] != static_cast<int>(pos_z)) {
		previous_position_[2] = static_cast<int>(pos_z);
		_itoa_s(static_cast<int>(pos_z), temp_string, 10);
		strcpy_s(final_string, "z:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[2].UpdateSentence(font_object_, final_string,
			10, 140, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	if (previous_position_[3] != static_cast<int>(rot_x)) {
		previous_position_[3] = static_cast<int>(rot_x);
		_itoa_s(static_cast<int>(rot_x), temp_string, 10);
		strcpy_s(final_string, "rx:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[3].UpdateSentence(font_object_, final_string,
			10, 180, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	if (previous_position_[4] != static_cast<int>(rot_y)) {
		previous_position_[4] = static_cast<int>(rot_y);
		_itoa_s(static_cast<int>(rot_y), temp_string, 10);
		strcpy_s(final_string, "ry:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[4].UpdateSentence(font_object_, final_string,
			10, 200, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	if (previous_position_[5] != static_cast<int>(rot_z)) {
		previous_position_[5] = static_cast<int>(rot_z);
		_itoa_s(static_cast<int>(rot_z), temp_string, 10);
		strcpy_s(final_string, "rz:");
		strcat_s(final_string, temp_string);
		if (!(position_string_[5].UpdateSentence(font_object_, final_string,
			10, 220, 1.0f, 1.0f, 1.0f))) {
			return false;
		}
	}

	return true;
}
