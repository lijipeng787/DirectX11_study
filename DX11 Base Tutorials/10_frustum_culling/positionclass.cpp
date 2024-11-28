#include "positionclass.h"

#include <math.h>

void PositionClass::TurnLeft(bool keydown) {

  // If the key is pressed increase the speed at which the camera turns left. If
  // not slow down the turn speed.
  if (keydown) {
    left_turning_speed_ += frame_time_ * 0.01f;

    if (left_turning_speed_ > (frame_time_ * 0.15f)) {
      left_turning_speed_ = frame_time_ * 0.15f;
    }
  } else {
    left_turning_speed_ -= frame_time_ * 0.005f;

    if (left_turning_speed_ < 0.0f) {
      left_turning_speed_ = 0.0f;
    }
  }

  // Update the rotation_ using the turning speed.
  rotation_y_ -= left_turning_speed_;
  if (rotation_y_ < 0.0f) {
    rotation_y_ += 360.0f;
  }
}

void PositionClass::TurnRight(bool keydown) {

  // If the key is pressed increase the speed at which the camera turns right.
  // If not slow down the turn speed.
  if (keydown) {
    right_turning_speed_ += frame_time_ * 0.01f;

    if (right_turning_speed_ > (frame_time_ * 0.15f)) {
      right_turning_speed_ = frame_time_ * 0.15f;
    }
  } else {
    right_turning_speed_ -= frame_time_ * 0.005f;

    if (right_turning_speed_ < 0.0f) {
      right_turning_speed_ = 0.0f;
    }
  }

  // Update the rotation_ using the turning speed.
  rotation_y_ += right_turning_speed_;
  if (rotation_y_ > 360.0f) {
    rotation_y_ -= 360.0f;
  }
}