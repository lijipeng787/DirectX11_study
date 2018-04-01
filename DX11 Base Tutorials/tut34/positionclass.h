









#include <math.h>





class PositionClass
{
public:
	PositionClass();
	PositionClass(const PositionClass&);
	~PositionClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void GetPosition(float&, float&, float&);
	void GetRotation(float&, float&, float&);

	void SetFrameTime(float);

	void MoveLeft(bool);
	void MoveRight(bool);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, rotation_y_, m_rotationZ;

	float frame_time_;

	float m_leftSpeed, m_rightSpeed;
};

#endif