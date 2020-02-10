#ifndef CLASS_RENDERGUISTATUS
#define CLASS_RENDERGUISTATUS

#include <nctl/String.h>

namespace nc = ncine;

struct SaveAnim
{
	static const unsigned int MaxStringLength = 256;
	inline float inverseFps() const { return 1.0f / static_cast<float>(fps); }

	nctl::String filename = nctl::String(MaxStringLength);
	unsigned int numSavedFrames = 0;
	int numFrames = 60;
	int fps = 60;
	float canvasResize = 1.0f;
	nc::Vector2i sheetDestPos;
};

/// The rener gui status class
struct RenderGuiStatus
{
	static const unsigned int MaxStringLength = 256;
	enum ResizeLevel
	{
		X1_8,
		X1_4,
		X1_2,
		X1,
		X2,
		X4,
		X8
	};

	ResizeLevel resizeLevel = ResizeLevel::X1;
	nctl::String filename = nctl::String(MaxStringLength);

	static float resizeAmount(ResizeLevel rl);
	float resizeAmount() const;
	void setResize(float resizeAmount);
};

#endif
