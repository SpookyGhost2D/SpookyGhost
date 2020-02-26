#ifndef CLASS_CANVASGUISECTION
#define CLASS_CANVASGUISECTION

#include <ncine/Vector2.h>

namespace nc = ncine;

class Canvas;

/// The canvas gui section class
class CanvasGuiSection
{
  public:
	void create(Canvas &canvas);

	void setResize(const nc::Vector2i &size);
	void setResize(int width, int height);
	float zoomAmount() const;

	void increaseZoom();
	void decreaseZoom();

  private:
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

	ZoomLevel zoomLevel_ = ZoomLevel::X1;
	ResizePreset resizePreset_ = ResizePreset::SIZE256;
	nc::Vector2i customCanvasSize_;
	const char *ResizeStrings[7] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "Custom" };
	int canvasZoomRadio_ = CanvasGuiSection::ZoomLevel::X1;

	void setZoom(float zoomAmount);

	nc::Vector2i resizeVector();
};

#endif
