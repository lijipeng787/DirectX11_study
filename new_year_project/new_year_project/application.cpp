#include "application.h"

Application::Application() :
	input_(std::make_shared<Input>()),
	shader_manager_(std::make_shared<ShaderManager>()),
	texture_manager_(std::make_shared<TextureManager>()),
	timer_(std::make_shared<Timer>()),
	fps_(std::make_shared<Fps>()),
	zone_(std::make_shared<Zone>()) {}

Application::~Application(){
}

bool Application::Initialize(HINSTANCE hinstance, HWND hwnd, int screen_width, int screen_height){

	if (!(input_->Initialize(hinstance, hwnd, screen_width, screen_height))) {
		MessageBoxW(hwnd, L"could not initialize the input pbject", L"error", MB_OK);
		return false;
	}

	d3d11_device_ = D3d11Device::get_d3d_object();
	if (!d3d11_device_) {
		return false;
	}
	
	if (!(d3d11_device_->Initialize(screen_width, screen_height, false, hwnd, false, 1500.0f, 0.1f))) {
		MessageBoxW(hwnd, L"could not initialize the d3d object", L"error", MB_OK);
		return false;
	}

	if (!(shader_manager_->Initialize(hwnd))) {
		MessageBoxW(hwnd, L"could not initialize the shader manager object", L"error", MB_OK);
		return false;
	}
	
	if (!(texture_manager_->Initialize(10))) {
		MessageBoxW(hwnd, L"could not initialize the texture manager object", L"error", MB_OK);
		return false;
	}

	if (!(texture_manager_->LoadTexture("../Engine/data/textures/dirt01d.tga", 0))) {
		MessageBoxW(hwnd, L"could not initialize the texture object", L"error", MB_OK);
		return false;
	}
	
	if (!(texture_manager_->LoadTexture("../Engine/data/textures/dirt01n.tga", 1))) {
		MessageBoxW(hwnd, L"could not initialize the texture object", L"error", MB_OK);
		return false;
	}

	if (!(timer_->initialize())) {
		MessageBoxW(hwnd, L"could not initialize the timer object", L"error", MB_OK);
		return false;
	}

	fps_->Initialize();

	if (!(zone_->Initialize(hwnd, screen_width, screen_height, 1500.0f))) {
		MessageBoxW(hwnd, L"could not initialize the zone object", L"error", MB_OK);
		return false;
	}

	return true;
}

bool Application::Frame(){

	fps_->Frame();

	timer_->frame();

	if (!(input_->Frame())) {
		return false;
	}

	if (true == input_->IsEscapePressed())
		return false;

	auto frame_time = timer_->get_time();
	auto fps_value = fps_->GetFps();

	if (!(zone_->Frame(input_.get(),
		shader_manager_.get(),
		texture_manager_.get(),
		frame_time,
		fps_value))) {

		return false;
	}

	return true;
}
