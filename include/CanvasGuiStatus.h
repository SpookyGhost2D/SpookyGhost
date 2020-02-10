#ifndef CLASS_CANVASGUISTATUS
#define CLASS_CANVASGUISTATUS

#include <ncine/Vector2.h>

namespace nc = ncine;

/// The canvas gui status class
struct CanvasGuiStatus
{
	enum ZoomLevel
	{
		X1_8,
		X1_4,
		X1_2,
		X1,
		X2,
		X4,
		X8
	};

	enum ResizePreset
	{
		SIZE16,
		SIZE32,
		SIZE64,
		SIZE128,
		SIZE256,
		SIZE512,
		CUSTOM
	};

	const char *ResizeStrings[7] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "Custom" };

	ZoomLevel zoomLevel = ZoomLevel::X1;
	ResizePreset resizePreset = ResizePreset::SIZE256;
	nc::Vector2i customCanvasSize;

	static float zoomAmount(ZoomLevel zl);
	float zoomAmount() const;
	void setZoom(float zoomAmount);
	void increaseZoom();
	void decreaseZoom();

	void setResize(const nc::Vector2i &size);
	void setResize(int width, int height);

	nc::Vector2i resizeVector();
};

#endif
