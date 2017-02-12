////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"
#include <new>


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_ParticleShader = 0;
	m_ParticleSystem = 0;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Create the Direct3D object.
	m_D3D = ( D3DClass* )_aligned_malloc( sizeof( D3DClass ), 16 );
	new ( m_D3D )D3DClass();
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = ( CameraClass* )_aligned_malloc( sizeof( CameraClass ), 16 );
	new ( m_Camera )CameraClass();
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, -2.0f, -10.0f);
	
	// Create the particle shader object.
	m_ParticleShader = ( ParticleShaderClass* )_aligned_malloc( sizeof( ParticleShaderClass ), 16 );
	new ( m_ParticleShader )ParticleShaderClass();
	if(!m_ParticleShader)
	{
		return false;
	}

	// Initialize the particle shader object.
	result = m_ParticleShader->Initialize(m_D3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the particle shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the particle system object.
	m_ParticleSystem = ( ParticleSystemClass* )_aligned_malloc( sizeof( ParticleSystemClass ), 16 );
	new ( m_ParticleSystem )ParticleSystemClass();
	if(!m_ParticleSystem)
	{
		return false;
	}

	// Initialize the particle system object.
	result = m_ParticleSystem->Initialize(m_D3D->GetDevice(), L"../../tut39/data/star.dds");
	if(!result)
	{
		return false;
	}

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the particle system object.
	if(m_ParticleSystem)
	{
		m_ParticleSystem->Shutdown();
		m_ParticleSystem->~ParticleSystemClass();
		_aligned_free( m_ParticleSystem );
		m_ParticleSystem = 0;
	}

	// Release the particle shader object.
	if(m_ParticleShader)
	{
		m_ParticleShader->Shutdown();
		m_ParticleShader->~ParticleShaderClass();
		_aligned_free( m_ParticleShader );
		m_ParticleShader = 0;
	}

	// Release the camera object.
	if( m_Camera )
	{
		m_Camera->~CameraClass();
		_aligned_free( m_Camera );
		m_Camera = 0;
	}

	// Release the D3D object.
	if( m_D3D )
	{
		m_D3D->Shutdown();
		m_D3D->~D3DClass();
		_aligned_free( m_D3D );
		m_D3D = 0;
	}

	return;
}


bool GraphicsClass::Frame(float frameTime)
{
	bool result;


	// Run the frame processing for the particle system.
	m_ParticleSystem->Frame(frameTime, m_D3D->GetDeviceContext());

	// Render the graphics scene.
	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	// Turn on alpha blending.
	m_D3D->EnableAlphaBlending();

	// Put the particle system vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_ParticleSystem->Render(m_D3D->GetDeviceContext());

	// Render the model using the texture shader.
	result = m_ParticleShader->Render(m_D3D->GetDeviceContext(), m_ParticleSystem->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
									  m_ParticleSystem->GetTexture());
	if(!result)
	{
		return false;
	}

	// Turn off alpha blending.
	m_D3D->DisableAlphaBlending();

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}