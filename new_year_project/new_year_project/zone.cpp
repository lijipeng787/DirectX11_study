#include "zone.h"

#define OBJECT_INITIALIZE_AND_TEST(obj_name,class_name)	\
		(obj_name)=new (class_name);					\
		if(!obj_name){return false;}					\

Zone::Zone() :user_interface_(nullptr), camera_(nullptr),
light_(nullptr), position_(nullptr),
skybox_(nullptr), terrain_(nullptr) {
}

Zone::Zone(const Zone & copy){
}

Zone::~Zone(){
}

bool Zone::Initialize(HWND hwnd, int screen_width, int screen_height, float screen_depth){
	OBJECT_INITIALIZE_AND_TEST(user_interface_, UserInterface);

	if (!(user_interface_->Initialize(screen_width, screen_height))) {
		MessageBoxW(hwnd, L"could not initialize the user interface object", L"error", MB_OK);
		return false;
	}

	OBJECT_INITIALIZE_AND_TEST(camera_, Camera);

	camera_->SetPosition(0.0f, 0.0f, -10.0f);
	camera_->Render();
	camera_->RenderBaseViewMatrix();

	OBJECT_INITIALIZE_AND_TEST(light_, Light);

	light_->set_diffuse_color(1.0f, 1.0f, 1.0f, 1.0f);
	light_->set_direction(-0.5f, -1.0f, -0.5f);

	OBJECT_INITIALIZE_AND_TEST(position_, Position);

	position_->SetPosition(512.0f, 30.0f, -10.0f);
	position_->SetRotation(0.0f, 0.0f, 0.0f);

	OBJECT_INITIALIZE_AND_TEST(skybox_, SkyBox);

	if (!(skybox_->Initialize())) {
		MessageBoxW(hwnd, L"could not initialize the skybox object", L"error", MB_OK);
		return false;
	}

	OBJECT_INITIALIZE_AND_TEST(terrain_, Terrain);

	if (!(terrain_->Initialize("../Engine/data/setup.txt"))) {
		MessageBoxW(hwnd, L"could not initialize the terrain object", L"error", MB_OK);
		return false;
	}

	return true;
}

void Zone::Shutdown(){

	if (camera_) {
		delete camera_;
		camera_ = nullptr;
	}

	if (light_) {
		delete light_;
		light_ = nullptr;
	}

	if (position_) {
		delete position_;
		position_ = nullptr;
	}
}

bool Zone::Frame(
	Input * input_object, 
	ShaderManager * shader_manager_object, 
	TextureManager * texture_manager_object, 
	float frame_time, 
	int fps
	){

	HandleMovementInput(input_object, frame_time);

	float pos_x, pos_y, pos_z;
	position_->GetPosition(pos_x, pos_y, pos_z);
	float rot_x, rot_y, rot_z;
	position_->GetRotation(rot_x, rot_y, rot_z);

	if (!(user_interface_->Frame(fps,pos_x, pos_y, pos_z, rot_x, rot_y, rot_z))) {
		return false;
	}

	if (!(Render(shader_manager_object, texture_manager_object))) {
		return false;
	}

	return true;
}

void Zone::HandleMovementInput(Input * input_object, float frame_tiem){
	position_->SetFrameTime(frame_tiem);
	auto key_down = false;

	key_down = input_object->IsLeftPressed();
	position_->TurnLeft(key_down);

	key_down = input_object->IsRightPressed();
	position_->TurnRight(key_down);

	key_down = input_object->IsUpPressed();
	position_->MoveForward(key_down);

	key_down = input_object->IsDownPressed();
	position_->MoveBackward(key_down);

	key_down = input_object->IsAPressed();
	position_->MoveUpward(key_down);

	key_down = input_object->IsZPressed();
	position_->MoveDownward(key_down);

	key_down = input_object->IsPageUpPressed();
	position_->LookUpward(key_down);

	key_down = input_object->IsPageDownPressed();
	position_->LookDownward(key_down);

	float pos_x, pos_y, pos_z;
	position_->GetPosition(pos_x, pos_y, pos_z);
	float rot_x, rot_y, rot_z;
	position_->GetRotation(rot_x, rot_y, rot_z);

	camera_->SetPosition(pos_x, pos_y, pos_z);
	camera_->SetRotation(rot_x, rot_y, rot_z);

	if (input_object->IsF1Toggled())
		display_ui_ = !display_ui_;
	
	if (input_object->IsF2Toggled())
		wire_frame_ = !wire_frame_;
	
	if (input_object->IsF3Toggled())
		cell_lines_ = !cell_lines_;
}

bool Zone::Render(ShaderManager * shader_manager_object, TextureManager * texture_manager_object){
	camera_->Render();

	auto d3dclass = D3d11Device::get_d3d_object();

	auto world = DirectX::XMMATRIX();
	d3dclass->get_world_matrix(world);

	auto projection = DirectX::XMMATRIX();
	d3dclass->get_projection_matrix(projection);

	auto view = DirectX::XMMATRIX();
	camera_->GetViewMatrix(view);

	auto base_view = DirectX::XMMATRIX();
	camera_->GetBaseViewMatrix(base_view);

	auto orthogonality = DirectX::XMMATRIX();
	d3dclass->get_orthogonality_matrix(orthogonality);

	auto camera_position = camera_->GetPosition();

	d3dclass->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	d3dclass->TurnCullingOff();
	d3dclass->TurnZBufferOff();

	world = DirectX::XMMatrixTranslation(camera_position.x, camera_position.y, camera_position.z);

	skybox_->Render();

	if (!(shader_manager_object->ExecuteSkyboxShader(
		skybox_->GetIndexCount(),
		world, view, projection,
		skybox_->get_skybox_apex_color(),
		skybox_->get_skybox_center_color()))
		) {
		return false;
	}

	d3dclass->get_world_matrix(world);

	d3dclass->TurnZBufferOn();
	d3dclass->TurnCullingOn();

	if (wire_frame_)
		d3dclass->TurnWireFrameOn();

	for (int i = 0; i < terrain_->get_terrain_cell_count(); ++i) {
		if (!(terrain_->RenderCell(i)))
			return false;

		if (!(shader_manager_object->ExecuteTerrainShader(
			terrain_->GetCellIndexCount(i),
			world, view, projection,
			texture_manager_object->GetTexture(0), 
			texture_manager_object->GetTexture(1),
			light_->get_direction(),
			light_->get_diffuse_color()))
			) {
			return false;
		}

		if (cell_lines_) {
			terrain_->RenderCellLines(i);
			if (!(shader_manager_object->ExecuteColorShader(
				terrain_->GetCellLinesIndexCount(i),
				world, view, projection))
				) {
				return false;
			}
		}
	}

	if (wire_frame_)
		d3dclass->TurnWireFrameOff();

	if (display_ui_) {
		if (!(user_interface_->Render(
			shader_manager_object, 
			world, 
			base_view, 
			orthogonality))
			) {
			return false;
		}
	}

	d3dclass->EndScene();

	return true;
}
