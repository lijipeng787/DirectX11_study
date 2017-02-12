////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"
#include <new>


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
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
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);

	return true;
}


void GraphicsClass::Shutdown()
{
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


void GraphicsClass::Frame()
{
	// Render the scene.
	Render();

	return;
}


void GraphicsClass::Render()
{
	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();


	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return;
}