#pragma once

#ifndef APPLICATION_HEADER_POSITION_H

#define APPLICATION_HEADER_POSITION_H

#include "application_common.h"

class Position {
public:
	Position();

	Position(const Position& copy);

	~Position();

public:
	void SetPosition(float x, float y, float z);

	void SetRotation(float x, float y, float z);

	void GetPosition(float& x, float& y, float& z);

	void GetRotation(float& x, float& y, float& z);

	void SetFrameTime(float frame_time);

	void MoveForward(bool key_down);

	void MoveBackward(bool key_down);

	void MoveUpward(bool key_down);

	void MoveDownward(bool key_down);

	void TurnLeft(bool key_down);

	void TurnRight(bool key_down);

	void LookUpward(bool key_down);

	void LookDownward(bool key_down);

private:
	float position_x_, position_y_, position_z_;

	float rotation_x_, rotation_y_, rotation_z_;

	float frame_time_;

	float forward_speed_, backward_speed_;

	float upward_speed_, downward_speed_;

	float turn_left_speed_, turn_right_speed_;

	float look_up_speed_, look_down_speed_;
};

#endif // !APPLICATION_HEADER_POSITION_H
