#pragma once

class PositionClass {
public:
  PositionClass();

  PositionClass(const PositionClass &);

  ~PositionClass();

public:
  void SetPosition(float, float, float);

  void SetRotation(float, float, float);

  void GetPosition(float &, float &, float &);

  void GetRotation(float &, float &, float &);

  void SetFrameTime(float);

  void MoveForward(bool);

  void MoveBackward(bool);

  void MoveUpward(bool);

  void MoveDownward(bool);

  void TurnLeft(bool);

  void TurnRight(bool);

  void LookUpward(bool);

  void LookDownward(bool);

private:
  float position_x_, position_y_, position_z_;

  float rotation_x_, rotation_y_, rotation_z_;

  float frame_time_;

  float m_forwardSpeed, m_backwardSpeed;

  float m_upwardSpeed, m_downwardSpeed;

  float left_turning_speed_, right_turning_speed_;

  float m_lookUpSpeed, m_lookDownSpeed;
};
