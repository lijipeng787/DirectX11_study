#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

class Input {

public:
	Input() {}

	explicit Input(const Input& copy) {}

	~Input() {}

public:
	void KeyDown(unsigned int input) { m_keys[input] = true; }

	void KeyUp(unsigned int input) { m_keys[input] = false; }

	bool IsKeyDown(unsigned int key) { return m_keys[key]; }

private:
	bool m_keys[256] = { false };
};

#endif 