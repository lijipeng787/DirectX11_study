#pragma once

class PositionClass {
public:
	PositionClass() {}

	PositionClass(const PositionClass& rhs) = delete;

	~PositionClass() {}
public:
	void SetPosition(float, float, float);

	void SetRotation(float, float, float);

	void GetPosition(float&, float&, float&);

	void GetRotation(float&, float&, float&);

	void SetFrameTime(float);

	void MoveLeft(bool);

	void MoveRight(bool);
private:
	float position_x_ = 0.0f, position_y_ = 0.0f, position_z_ = 0.0f;

	float rotation_x_ = 0.0f, rotation_y_ = 0.0f, rotation_z_ = 0.0f;

	float frame_time_ = 0.0f, left_speed_ = 0.0f, right_speed_ = 0.0f;
};
