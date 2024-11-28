#include "positionclass.h"

#include <math.h>

void PositionClass::SetPosition(float x, float y, float z) {

  position_x_ = x;
  position_y_ = y;
  position_z_ = z;
}

void PositionClass::SetRotation(float x, float y, float z) {

  rotation_x_ = x;
  rotation_y_ = y;
  rotation_z_ = z;
}

void PositionClass::GetPosition(float &x, float &y, float &z) {

  x = position_x_;
  y = position_y_;
  z = position_z_;
}

void PositionClass::GetRotation(float &x, float &y, float &z) {
  x = rotation_x_;
  y = rotation_y_;
  z = rotation_z_;
}

void PositionClass::SetFrameTime(float time) { frame_time_ = time; }

void PositionClass::MoveLeft(bool keydown) {

  float radians;

  // Update the forward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    left_speed_ += frame_time_ * 0.001f;

    if (left_speed_ > (frame_time_ * 0.03f)) {
      left_speed_ = frame_time_ * 0.03f;
    }
  } else {
    left_speed_ -= frame_time_ * 0.0007f;

    if (left_speed_ < 0.0f) {
      left_speed_ = 0.0f;
    }
  }

  // Convert degrees to radians.
  radians = rotation_y_ * 0.0174532925f;

  // Update the position.
  position_x_ -= cosf(radians) * left_speed_;
  position_z_ -= sinf(radians) * left_speed_;
}

void PositionClass::MoveRight(bool keydown) {

  float radians;

  // Update the backward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    right_speed_ += frame_time_ * 0.001f;

    if (right_speed_ > (frame_time_ * 0.03f)) {
      right_speed_ = frame_time_ * 0.03f;
    }
  } else {
    right_speed_ -= frame_time_ * 0.0007f;

    if (right_speed_ < 0.0f) {
      right_speed_ = 0.0f;
    }
  }

  // Convert degrees to radians.
  radians = rotation_y_ * 0.0174532925f;

  // Update the position.
  position_x_ += cosf(radians) * right_speed_;
  position_z_ += sinf(radians) * right_speed_;
}