#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_

class Input{
public:
	Input();
	
	Input(const Input& rhs) = delete;

	Input& operator=(const Input& rhs) = delete;
	
	~Input();
public:
	void Initialize();

	void KeyDown(unsigned int);
	
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);
private:
	bool keys_[256];
};

#endif