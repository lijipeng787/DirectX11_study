#pragma once

#include <d3d11.h>

class OutputMergeStateManager {
public:
	OutputMergeStateManager();

	OutputMergeStateManager(const OutputMergeStateManager& rhs) = delete;;

	OutputMergeStateManager& operator= (const OutputMergeStateManager& rhs) = delete;

	~OutputMergeStateManager();
public:
private:
}