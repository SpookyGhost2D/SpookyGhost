#ifndef CLASS_CANVASGUISECTION
#define CLASS_CANVASGUISECTION

#include <ncine/Vector2.h>

namespace nc = ncine;

class Canvas;

/// The class handling the zoom controls of the canvas window
class CanvasGuiSection
{
  public:
	void create(Canvas &canvas);

	void setResize(const nc::Vector2i &size);
	void setResize(int width, int height);
	float zoomAmount() const;

	void resetZoom();
	void increaseZoom();
	void decreaseZoom();

	inline bool showBorders() const { return showBorders_; }

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
	int canvasZoomRadio_ = CanvasGuiSection::ZoomLevel::X1;
	bool showBorders_ = false;

	nc::Vector2i resizeVector(ResizePreset resizePreset);
	nc::Vector2i resizeVector();
};

#endif
