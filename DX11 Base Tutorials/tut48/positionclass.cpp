


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

	m_forwardSpeed   = 0.0f;
	m_backwardSpeed  = 0.0f;
	m_upwardSpeed    = 0.0f;
	m_downwardSpeed  = 0.0f;
	left_turning_speed_  = 0.0f;
	right_turning_speed_ = 0.0f;
	m_lookUpSpeed    = 0.0f;
	m_lookDownSpeed  = 0.0f;
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


void PositionClass::MoveForward(bool keydown)
{
	float radians;


	// Update the forward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_forwardSpeed += frame_time_ * 0.001f;

		if(m_forwardSpeed > (frame_time_ * 0.03f))
		{
			m_forwardSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_forwardSpeed -= frame_time_ * 0.0007f;

		if(m_forwardSpeed < 0.0f)
		{
			m_forwardSpeed = 0.0f;
		}
	}

	// Convert degrees to radians.
	radians = rotation_y_ * 0.0174532925f;

	// Update the position.
	m_positionX += sinf(radians) * m_forwardSpeed;
	m_positionZ += cosf(radians) * m_forwardSpeed;

	
}


void PositionClass::MoveBackward(bool keydown)
{
	float radians;


	// Update the backward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_backwardSpeed += frame_time_ * 0.001f;

		if(m_backwardSpeed > (frame_time_ * 0.03f))
		{
			m_backwardSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_backwardSpeed -= frame_time_ * 0.0007f;
		
		if(m_backwardSpeed < 0.0f)
		{
			m_backwardSpeed = 0.0f;
		}
	}

	// Convert degrees to radians.
	radians = rotation_y_ * 0.0174532925f;

	// Update the position.
	m_positionX -= sinf(radians) * m_backwardSpeed;
	m_positionZ -= cosf(radians) * m_backwardSpeed;

	
}


void PositionClass::MoveUpward(bool keydown)
{
	// Update the upward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_upwardSpeed += frame_time_ * 0.003f;

		if(m_upwardSpeed > (frame_time_ * 0.03f))
		{
			m_upwardSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_upwardSpeed -= frame_time_ * 0.002f;

		if(m_upwardSpeed < 0.0f)
		{
			m_upwardSpeed = 0.0f;
		}
	}

	// Update the height position.
	m_positionY += m_upwardSpeed;

	
}


void PositionClass::MoveDownward(bool keydown)
{
	// Update the downward speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_downwardSpeed += frame_time_ * 0.003f;

		if(m_downwardSpeed > (frame_time_ * 0.03f))
		{
			m_downwardSpeed = frame_time_ * 0.03f;
		}
	}
	else
	{
		m_downwardSpeed -= frame_time_ * 0.002f;

		if(m_downwardSpeed < 0.0f)
		{
			m_downwardSpeed = 0.0f;
		}
	}

	// Update the height position.
	m_positionY -= m_downwardSpeed;

	
}


void PositionClass::TurnLeft(bool keydown)
{
	// Update the left turn speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		left_turning_speed_ += frame_time_ * 0.01f;

		if(left_turning_speed_ > (frame_time_ * 0.15f))
		{
			left_turning_speed_ = frame_time_ * 0.15f;
		}
	}
	else
	{
		left_turning_speed_ -= frame_time_* 0.005f;

		if(left_turning_speed_ < 0.0f)
		{
			left_turning_speed_ = 0.0f;
		}
	}

	// Update the rotation_.
	rotation_y_ -= left_turning_speed_;

	// Keep the rotation_ in the 0 to 360 range.
	if(rotation_y_ < 0.0f)
	{
		rotation_y_ += 360.0f;
	}

	
}


void PositionClass::TurnRight(bool keydown)
{
	// Update the right turn speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		right_turning_speed_ += frame_time_ * 0.01f;

		if(right_turning_speed_ > (frame_time_ * 0.15f))
		{
			right_turning_speed_ = frame_time_ * 0.15f;
		}
	}
	else
	{
		right_turning_speed_ -= frame_time_* 0.005f;

		if(right_turning_speed_ < 0.0f)
		{
			right_turning_speed_ = 0.0f;
		}
	}

	// Update the rotation_.
	rotation_y_ += right_turning_speed_;

	// Keep the rotation_ in the 0 to 360 range.
	if(rotation_y_ > 360.0f)
	{
		rotation_y_ -= 360.0f;
	}

	
}


void PositionClass::LookUpward(bool keydown)
{
	// Update the upward rotation_ speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_lookUpSpeed += frame_time_ * 0.01f;

		if(m_lookUpSpeed > (frame_time_ * 0.15f))
		{
			m_lookUpSpeed = frame_time_ * 0.15f;
		}
	}
	else
	{
		m_lookUpSpeed -= frame_time_* 0.005f;

		if(m_lookUpSpeed < 0.0f)
		{
			m_lookUpSpeed = 0.0f;
		}
	}

	// Update the rotation_.
	m_rotationX -= m_lookUpSpeed;

	// Keep the rotation_ maximum 90 degrees.
	if(m_rotationX > 90.0f)
	{
		m_rotationX = 90.0f;
	}

	
}


void PositionClass::LookDownward(bool keydown)
{
	// Update the downward rotation_ speed movement based on the frame time and whether the user is holding the key down or not.
	if(keydown)
	{
		m_lookDownSpeed += frame_time_ * 0.01f;

		if(m_lookDownSpeed > (frame_time_ * 0.15f))
		{
			m_lookDownSpeed = frame_time_ * 0.15f;
		}
	}
	else
	{
		m_lookDownSpeed -= frame_time_* 0.005f;

		if(m_lookDownSpeed < 0.0f)
		{
			m_lookDownSpeed = 0.0f;
		}
	}

	// Update the rotation_.
	m_rotationX += m_lookDownSpeed;

	// Keep the rotation_ maximum 90 degrees.
	if(m_rotationX < -90.0f)
	{
		m_rotationX = -90.0f;
	}

	
}