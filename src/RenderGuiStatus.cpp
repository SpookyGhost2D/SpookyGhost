#include <ncine/imgui.h>
#include "RenderGuiStatus.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

float RenderGuiStatus::resizeAmount(ResizeLevel rl)
{
	switch (rl)
	{
		case ResizeLevel::X1_8:
			return 0.125f;
		case ResizeLevel::X1_4:
			return 0.25f;
		case ResizeLevel::X1_2:
			return 0.5f;
		case ResizeLevel::X1:
			return 1.0f;
		case ResizeLevel::X2:
			return 2.0f;
		case ResizeLevel::X4:
			return 4.0f;
		case ResizeLevel::X8:
			return 8.0f;
	}
	return 1.0f;
}

float RenderGuiStatus::resizeAmount() const
{
	return resizeAmount(resizeLevel);
}

void RenderGuiStatus::setResize(float resizeAmount)
{
	if (resizeAmount <= 0.125f)
		resizeLevel = ResizeLevel::X1_8;
	else if (resizeAmount <= 0.25f)
		resizeLevel = ResizeLevel::X1_4;
	else if (resizeAmount <= 0.5f)
		resizeLevel = ResizeLevel::X1_2;
	else if (resizeAmount <= 1.0f)
		resizeLevel = ResizeLevel::X1;
	else if (resizeAmount <= 2.0f)
		resizeLevel = ResizeLevel::X2;
	else if (resizeAmount <= 4.0f)
		resizeLevel = ResizeLevel::X4;
	else
		resizeLevel = ResizeLevel::X8;
}
