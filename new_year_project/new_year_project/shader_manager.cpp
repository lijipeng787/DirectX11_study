#include "shader_manager.h"

#define OBJECT_INITIALIZE_AND_TEST(obj_name,class_name)	\
		(obj_name)=new (class_name);					\
		if(!obj_name){return false;}					\
		if(!(obj_name->Initialize(hwnd)))				\
		return false;

ShaderManager::ShaderManager() {
}

ShaderManager::ShaderManager(const ShaderManager & copy){
}

ShaderManager::~ShaderManager(){
}

bool ShaderManager::Initialize(HWND hwnd){
	OBJECT_INITIALIZE_AND_TEST(color_shader_, ColorShader);

	OBJECT_INITIALIZE_AND_TEST(texture_shader_, TextureShader);

	OBJECT_INITIALIZE_AND_TEST(light_shader_, LightShader);

	OBJECT_INITIALIZE_AND_TEST(font_shader_, FontShader);

	OBJECT_INITIALIZE_AND_TEST(terrain_shader_, TerrainShader);

	OBJECT_INITIALIZE_AND_TEST(skybox_shader_, SkyboxShader);
	return true;
}

bool ShaderManager::ExecuteColorShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection){
	
	auto result = color_shader_->InitializeGraphicsPipeline(world, view, projection);
	color_shader_->DrawIndexed(index_count);
	return result;
}

bool ShaderManager::ExecuteTextureShader(int index_count,
	DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
	ID3D11ShaderResourceView * srv_texture) {

	auto result = texture_shader_->InitializeGraphicsPipeline(world, view, projection, srv_texture);
	texture_shader_->DrawIndexed(index_count);
	return result;
}

bool ShaderManager::ExecuteLightShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 diffuse_color){

	auto result = light_shader_->InitializeGraphicsPipeline(
		world, view, projection,
		srv_texture, light_direction, diffuse_color
		);
	light_shader_->DrawIndexed(index_count);
	return result;
}

bool ShaderManager::ExecuteFontShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, DirectX::XMFLOAT4 font_color){

	auto result = font_shader_->InitializeGraphicsPipeline(
		world, view, projection,
		srv_texture, font_color
		);
	font_shader_->DrawIndexed(index_count);
	return result;
}

bool ShaderManager::ExecuteSkyboxShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	DirectX::XMFLOAT4 skybox_apex_color, DirectX::XMFLOAT4 skybox_center_color){
	
	auto result = skybox_shader_->InitializeGraphicsPipeline(
		world, view, projection,
		skybox_apex_color, skybox_center_color
		);
	skybox_shader_->DrawIndexed(index_count);
	return result;
}

bool ShaderManager::ExecuteTerrainShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, ID3D11ShaderResourceView * srv_normalmap, 
	DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 diffuse_color){
	
	auto result = terrain_shader_->InitializeGraphicsPipeline(
		world, view, projection,
		srv_texture, srv_normalmap,
		light_direction, diffuse_color
		);
	terrain_shader_->DrawIndexed(index_count);
	return result;
}
