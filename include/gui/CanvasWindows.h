#ifndef CLASS_TEXRECTWINDOW
#define CLASS_TEXRECTWINDOW

#include <ncine/Vector2.h>
#include <ncine/Colorf.h>

class UserInterface;
class Sprite;

namespace nc = ncine;

/// The canvas and tex rect windows class
class CanvasWindows
{
  public:
	explicit CanvasWindows(UserInterface &ui);

	inline bool isHoveringOnCanvasWindow() const { return hoveringOnCanvasWindow_; }
	inline bool isHoveringOnCanvas() const { return hoveringOnCanvas_; }

	void createCanvasWindow();
	void createTexRectWindow();

  private:
	enum class MouseStatus
	{
		IDLE,
		CLICKED,
		RELEASED,
		DRAGGING
	};

	class SpriteProperties
	{
	  public:
		void save(Sprite &sprite);
		void restore(Sprite &sprite);

	  private:
		bool saved_ = false;
		Sprite *parent_ = nullptr;
		nc::Vector2f position_ = nc::Vector2f(0.0f, 0.0f);
		float rotation_ = 0.0f;
		nc::Vector2f scaleFactor_ = nc::Vector2f(1.0f, 1.0f);
		nc::Vector2f anchorPoint_ = nc::Vector2f(1.0f, 1.0f);
		nc::Colorf color_ = nc::Colorf::White;
	};

	UserInterface &ui_;
	bool hoveringOnCanvasWindow_;
	bool hoveringOnCanvas_;
	SpriteProperties spriteProps_;

	void mouseWheelCanvasZoom();
};

#endif
