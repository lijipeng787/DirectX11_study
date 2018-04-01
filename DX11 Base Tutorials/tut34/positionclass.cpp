


#include "positionclass.h"


PositionClass::PositionClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;
	
	m_rotationX = 0.0f;
	rotation_y_ = 0.0f;
	m_rotationZ = 0.0f;

	frame_time_ = 0.0f;

	m_leftSpeed   = 0.0f;
	m_rightSpeed  = 0.0f;
}


PositionClass::PositionClass(const PositionClass& other)
{
}


PositionClass::~PositionClass()
{
}


void PositionClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	
}


void PositionClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	rotation_y_ = y;
	m_rotationZ = z;
	
}


void PositionClass::GetPosition(float& x, float& y, float& z)
{
	x = m_positionX;
	y = m_positionY;
	z = m_positionZ;
	
}


void PositionClass::GetRotation(float& x, float& y, float& z)
{
	x = m_rotationX;
	y = rotation_y_;
	z = m_rotationZ;
	
}


void PositionClass::SetFrameTime(float time)
{
	frame_time_ = time;
	
}


void PositionClass::MoveLeft(bool keydown)
{
	float radians;


	// Update the forward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_leftSpeed += frame_time_ * 0.001f;

		if(m_leftSpeed > (frame_time_ * 0.03f))
		{
			m_leftSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_leftSpeed -= frame_time_ * 0.0007f;

		if(m_leftSpeed < 0.0f)
		{
			m_leftSpeed = 0.0f;
		}
	}

	// Convert degrees to radians.
	radians = rotation_y_ * 0.0174532925f;

	// Update the position.
	m_positionX -= cosf(radians) * m_leftSpeed;
	m_positionZ -= sinf(radians) * m_leftSpeed;

	
}


void PositionClass::MoveRight(bool keydown)
{
	float radians;


	// Update the backward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_rightSpeed += frame_time_ * 0.001f;

		if(m_rightSpeed > (frame_time_ * 0.03f))
		{
			m_rightSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_rightSpeed -= frame_time_ * 0.0007f;
		
		if(m_rightSpeed < 0.0f)
		{
			m_rightSpeed = 0.0f;
		}
	}

	// Convert degrees to radians.
	radians = rotation_y_ * 0.0174532925f;

	// Update the position.
	m_positionX += cosf(radians) * m_rightSpeed;
	m_positionZ += sinf(radians) * m_rightSpeed;

	
}