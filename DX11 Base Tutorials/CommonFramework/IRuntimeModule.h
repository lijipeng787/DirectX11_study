#pragma once

#include "Interface.h"

Interface IRuntimeModule{
public:
	virtual ~IRuntimeModule() {};
public:
	virtual bool PreInitialize() = 0;

	virtual bool Initialize() = 0;

	virtual bool PostInitialize() = 0;

	virtual void Finalize() = 0;

	virtual void Tick() = 0;
};