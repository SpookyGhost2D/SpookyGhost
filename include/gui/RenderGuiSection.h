#ifndef CLASS_RENDERGUISECTION
#define CLASS_RENDERGUISECTION

#include <ncine/Vector2.h>
#include "gui/gui_common.h"

namespace nc = ncine;

class UserInterface;

struct SaveAnim
{
	inline float inverseFps() const { return 1.0f / static_cast<float>(fps); }

	nctl::String filename = nctl::String(ui::MaxStringLength);
	unsigned int numSavedFrames = 0;
	int numFrames = 60;
	int fps = 60;
	float canvasResize = 1.0f;
	nc::Vector2i sheetDestPos;
};

/// The render gui section class
class RenderGuiSection
{
public:
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
	nctl::String filename = nctl::String(ui::MaxStringLength);

	RenderGuiSection(UserInterface &ui)
	    : ui_(ui) {}

	inline const SaveAnim &saveAnimStatus() const { return saveAnimStatus_; }
	inline SaveAnim &saveAnimStatus() { return saveAnimStatus_; }
	inline bool shouldSaveFrames() const { return shouldSaveFrames_; }
	inline bool shouldSaveSpritesheet() const { return shouldSaveSpritesheet_; }

	static float resizeAmount(ResizeLevel rl);
	float resizeAmount() const;
	void setResize(float resizeAmount);

	void create();
	void signalFrameSaved();
	void cancelRender();

  private:
	UserInterface &ui_;
	SaveAnim saveAnimStatus_;
	bool shouldSaveFrames_ = false;
	bool shouldSaveSpritesheet_ = false;
};

#endif
