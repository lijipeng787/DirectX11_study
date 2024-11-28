#pragma once

class PositionClass {
public:
  PositionClass() {}

  PositionClass(const PositionClass &rhs) = delete;

  ~PositionClass() {}

public:
  inline void SetFrameTime(float time) { frame_time_ = time; }

  inline void GetRotation(float &y) { y = rotation_y_; }

  void TurnLeft(bool);

  void TurnRight(bool);

private:
  float frame_time_ = 0.0f, rotation_y_ = 0.0f;

  float left_turning_speed_ = 0.0f, right_turning_speed_ = 0.0f;
};