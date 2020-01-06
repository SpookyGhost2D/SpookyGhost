#include <ncine/imgui.h>
#include <ncine/imgui_internal.h>
#include <ncine/Application.h>

#include "UserInterface.h"
#include "Canvas.h"
#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "Sprite.h"
#include "Texture.h"
//#include "LuaSerializer.h"

namespace {

const char *easingCurveTypes[] = { "Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sine", "Exponential", "Circular" };
const char *easingCurveLoopModes[] = { "Disabled", "Rewind", "Ping Pong" };

const char *animationTypes[] = { "Parallel Group", "Sequential Group", "Property" };
enum AnimationTypesEnum { PARALLEL_GROUP, SEQUENTIAL_GROUP, PROPERTY };

const char *propertyTypes[] = { "None", "Position X", "Position Y", "Rotation" };
enum PropertyTypesEnum { NONE, POSITION_X, POSITION_Y, ROTATION };

const char *resizePresets[] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "custom" };
enum ResizePresetsEnum { SIZE16, SIZE32, SIZE64, SIZE128, SIZE256, SIZE512, CUSTOM };

int inputTextCallback(ImGuiInputTextCallbackData *data)
{
	nctl::String *string = reinterpret_cast<nctl::String *>(data->UserData);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		ASSERT(data->Buf == string->data());
		string->setLength(static_cast<unsigned int>(data->BufTextLen));
		data->Buf = string->data();
	}
	return 0;
}

const char *animStateToString(IAnimation::State state)
{
	switch (state)
	{
		case IAnimation::State::STOPPED: return "Stopped";
		case IAnimation::State::PAUSED: return "Paused";
		case IAnimation::State::PLAYING: return "Playing";
	}
	return "Unknown";
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface(Canvas &canvas, AnimationManager &animMgr, Sprite &sprite)
    : auxString_(MaxStringLength), filename_(MaxStringLength),
      canvas_(canvas), animMgr_(animMgr), sprite_(sprite)
{
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#ifdef __ANDROID__
	io.FontGlobalScale = 2.0f;
#endif
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::signalFrameSaved()
{
	ASSERT(shouldSaveAnim_ == true);

	saveAnimStatus_.filename.format("%s_%03d.png", filename_.data(), saveAnimStatus_.numSavedFrames);
	saveAnimStatus_.numSavedFrames++;
	if (saveAnimStatus_.numSavedFrames == saveAnimStatus_.lastFrame - saveAnimStatus_.firstFrame)
	{
		shouldSaveAnim_ = false;
		saveAnimStatus_.numSavedFrames = 0;
		pushStatusMessage("Animation saved");
	}
}

void UserInterface::pushStatusMessage(const char *message)
{
	statusMessage_ = message;
	lastStatus_ = nc::TimeStamp::now();
}

void UserInterface::createGui()
{
	createDockingSpace();

	ImGui::Begin("SpookyGhost");

	createCanvasGui();
	createSpriteGui();
	createAnimationsGui();
	createRenderGui();

	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImGuiCond_FirstUseEver);
	ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_HorizontalScrollbar);// | ImGuiWindowFlags_NoBackground);
	ImGui::Image(canvas_.imguiTexId(), ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::End();

	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();
	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createDockingSpace()
{
	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable == 0)
		return;

	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
	                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool open = true;
	ImGui::Begin("DockSpace", &open, windowFlags);

	ImGui::PopStyleVar(3);

	const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	//const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode |
	//                                          ImGuiDockNodeFlags_AutoHideTabBar;
	ImGuiID dockspaceId = ImGui::GetID("TheDockSpace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

	//createInitialDocking(dockspaceId);
	createMenuBar();

	ImGui::End();
}

void UserInterface::createInitialDocking(ImGuiID dockspaceId)
{
	//if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr)
	//	return;

	//ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None); //ImGuiDockNodeFlags_Dockspace
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	//ImGui::DockBuilderSetNodeSize(dockspaceId, ImVec2(1.0f, 1.0f));

	ImGuiID dockMainId = dockspaceId;
	ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.20f, nullptr, &dockMainId);
	ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.20f, nullptr, &dockMainId);

	ImGui::DockBuilderDockWindow("SpookyGhost", dockIdLeft);
	ImGui::DockBuilderDockWindow("Canvas", dockIdRight);
	ImGui::DockBuilderFinish(dockspaceId);
}

void UserInterface::createMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open")) { pushStatusMessage("Open and Save are not implemented yet"); }
			if (ImGui::MenuItem("Save")) { pushStatusMessage("Open and Save are not implemented yet"); }
			if (ImGui::MenuItem("Quit")) { nc::theApplication().quit(); }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void UserInterface::createCanvasGui()
{
	if (ImGui::CollapsingHeader("Canvas"))
	{
		ImGui::Text("Zoom:");
		ImGui::SameLine();
		if (ImGui::Button("1X"))
			canvasZoom_ = 1.0f;
		ImGui::SameLine();
		if (ImGui::Button("2X"))
			canvasZoom_ = 2.0f;
		ImGui::SameLine();
		if (ImGui::Button("4X"))
			canvasZoom_ = 4.0f;
		ImGui::SameLine();
		if (ImGui::Button("8X"))
			canvasZoom_ = 8.0f;

		nc::Vector2i desiredCanvasSize;
		static int currentComboResize = static_cast<int>(ResizePresetsEnum::SIZE256); // TOD: hard-coded initial state
		ImGui::Combo("Presets", &currentComboResize, resizePresets, IM_ARRAYSIZE(resizePresets));
		if (currentComboResize == ResizePresetsEnum::CUSTOM)
			ImGui::InputInt2("Custom Size", customCanvasSize_.data());
		else
			customCanvasSize_ = canvas_.size();
		switch (currentComboResize)
		{
			case ResizePresetsEnum::SIZE16:
				desiredCanvasSize.set(16, 16);
				break;
			case ResizePresetsEnum::SIZE32:
				desiredCanvasSize.set(32, 32);
				break;
			case ResizePresetsEnum::SIZE64:
				desiredCanvasSize.set(64, 64);
				break;
			case ResizePresetsEnum::SIZE128:
				desiredCanvasSize.set(128, 128);
				break;
			case ResizePresetsEnum::SIZE256:
				desiredCanvasSize.set(256, 256);
				break;
			case ResizePresetsEnum::SIZE512:
				desiredCanvasSize.set(512, 512);
				break;
			case ResizePresetsEnum::CUSTOM:
				desiredCanvasSize = customCanvasSize_;
				break;
		}

		if (desiredCanvasSize.x < 4)
			desiredCanvasSize.x = 4;
		else if (desiredCanvasSize.x > canvas_.maxTextureSize())
			desiredCanvasSize.x = canvas_.maxTextureSize();
		if (desiredCanvasSize.y < 4)
			desiredCanvasSize.y = 4;
		else if (desiredCanvasSize.y > canvas_.maxTextureSize())
			desiredCanvasSize.y = canvas_.maxTextureSize();

		ImGui::SameLine();
		if (ImGui::Button("Resize") &&
		    (canvas_.size().x != desiredCanvasSize.x || canvas_.size().y != desiredCanvasSize.y))
		{
			canvas_.resizeTexture(desiredCanvasSize);
		}

		ImGui::Text("Size: %d x %d", canvas_.texWidth(), canvas_.texHeight());
		ImGui::ColorEdit4("Background", canvas_.backgroundColor.data(), ImGuiColorEditFlags_AlphaBar);
	}
}

void UserInterface::createSpriteGui()
{
	// TODO: Load image
	// TODO: Set texrect
	// TODO: Show current info like position, rotation, frame, etc...

	if (ImGui::CollapsingHeader("Sprite"))
	{
		nc::Vector2f position(sprite_.x, sprite_.y);
		ImGui::SliderFloat2("Position", position.data(), 0.0f, canvas_.texWidth());
		sprite_.x = roundf(position.x);
		sprite_.y = roundf(position.y);

		ImGui::SliderFloat("Rotation", &sprite_.rotation, 0.0f, 360.0f);

		Texture &tex = sprite_.texture();
		const float texWidth = static_cast<float>(tex.width());
		const float texHeight = static_cast<float>(tex.height());
		ImVec2 size(texWidth * 4, texHeight * 4);
		ImVec2 uv0(0.0f, 0.0f);
		ImVec2 uv1(1.0, 1.0f);
		ImGui::Image(sprite_.imguiTexId(), size, uv0, uv1);

#if 0
		Texture &tex = sprite_.texture();
		const float texWidth = static_cast<float>(tex.width());
		const float texHeight = static_cast<float>(tex.height());
		ImVec2 size(texWidth, texHeight);
		ImVec2 uv0(0.0f, 0.0f);
		ImVec2 uv1(1.0, 1.0f);

		nc::Recti texRect = sprite_.texRect();
		size = ImVec2(texRect.w, texRect.h);
		uv0.x = texRect.x / texWidth;
		uv0.y = texRect.y / texHeight;
		uv1.x = (texRect.x + texRect.w) / texWidth;
		uv1.y = (texRect.y + texRect.h) / texHeight;

		bool isFlippedX = sprite_.isFlippedX();
		if (isFlippedX)
			nctl::swap(uv0.x, uv1.x);
		bool isFlippedY = sprite_.isFlippedY();
		if (isFlippedY)
			nctl::swap(uv0.y, uv1.y);


		ImGui::Image(sprite_.imguiTexId(), size, uv0, uv1);

		int minX = texRect.x;
		int maxX = minX + texRect.w;
		ImGui::DragIntRange2("Rect X", &minX, &maxX, 1.0f, 0, tex.width());
		int minY = texRect.y;
		int maxY = minY + texRect.h;
		ImGui::DragIntRange2("Rect Y", &minY, &maxY, 1.0f, 0, tex.height());
		texRect.x = minX;
		texRect.w = maxX - minX;
		texRect.y = minY;
		texRect.h = maxY - minY;
		ImGui::SameLine();

		if (ImGui::Button("Reset"))
			texRect = nc::Recti(0, 0, tex.width(), tex.height());

		if (s.texRect.x != spriteState_.texRect.x || s.texRect.y != spriteState_.texRect.y ||
		    s.texRect.w != spriteState_.texRect.w || s.texRect.h != spriteState_.texRect.h)
		{
			particleSystem->setTexRect(spriteState_.texRect);
			s.texRect = spriteState_.texRect;
		}

		ImGui::Checkbox("Flipped X", &isFlippedX);
		ImGui::SameLine();
		ImGui::Checkbox("Flipped Y", &isFlippedY);

		if (sprite_.isFlippedX() != isFlippedX)
			sprite_.setFlippedX(isFlippedX);
		if (sprite_.isFlippedY() != isFlippedY)
			sprite_.setFlippedY(isFlippedY);

		ImGui::SliderFloat2("Anchor Point", spriteState_.anchorPoint.data(), 0.0f, 1.0f);
		static int currentAnchorSelection = 0;
		if (ImGui::Combo("Anchor Presets", &currentAnchorSelection, anchorPointItems, IM_ARRAYSIZE(anchorPointItems)))
		{
			switch (currentAnchorSelection)
			{
				case 0:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorCenter;
					break;
				case 1:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorBottomLeft;
					break;
				case 2:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorTopLeft;
					break;
				case 3:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorBottomRight;
					break;
				case 4:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorTopRight;
					break;
			}
		}
		if (s.anchorPoint.x != spriteState_.anchorPoint.x ||
		    s.anchorPoint.y != spriteState_.anchorPoint.y)
		{
			particleSystem->setAnchorPoint(spriteState_.anchorPoint);
			s.anchorPoint = spriteState_.anchorPoint;
		}
#endif
	}
}

void UserInterface::createAnimationsGui()
{
	if (ImGui::CollapsingHeader("Animations"))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				case AnimationTypesEnum::PROPERTY:
					animMgr_.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			animMgr_.anims().clear();
		ImGui::Separator();

		for (unsigned int i = 0; i < animMgr_.anims().size(); i++)
			createRecursiveAnimationsGui(*animMgr_.anims()[i].get());
	}
}

void UserInterface::createRenderGui()
{
	if (ImGui::CollapsingHeader("Render"))
	{
		ImGui::InputText("Filename prefix", filename_.data(), MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &filename_);

		ImGui::InputFloat("FPS", &saveAnimStatus_.fps);
		ImGui::InputInt("First frame", &saveAnimStatus_.firstFrame);
		ImGui::InputInt("Last frame", &saveAnimStatus_.lastFrame);
		const float duration = saveAnimStatus_.numFrames() * saveAnimStatus_.inverseFps();
		ImGui::Text("Duration: %.2fs", duration);

		if (saveAnimStatus_.firstFrame < 0)
			saveAnimStatus_.firstFrame = 0;
		if (saveAnimStatus_.lastFrame < 0)
			saveAnimStatus_.lastFrame = 0;
		if (saveAnimStatus_.lastFrame <= saveAnimStatus_.firstFrame)
			saveAnimStatus_.lastFrame = saveAnimStatus_.firstFrame + 1;

		if (shouldSaveAnim_)
		{
			const unsigned int numSavedFrames = saveAnimStatus_.numSavedFrames;
			const float fraction = numSavedFrames / static_cast<float>(saveAnimStatus_.numFrames());
			auxString_.format("Frame: %d (%d/%d)", numSavedFrames, saveAnimStatus_.firstFrame + numSavedFrames, saveAnimStatus_.lastFrame);
			ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), auxString_.data());
		}
		else if (ImGui::Button("Save to PNG") && shouldSaveAnim_ == false)
		{
			if (filename_.isEmpty())
				pushStatusMessage("Set a filename prefix before saving an animation");
			else
			{
				saveAnimStatus_.filename.format("%s_%03d.png", filename_.data(), saveAnimStatus_.numSavedFrames);
				shouldSaveAnim_ = true;
			}
		}
	}
}

void UserInterface::createRecursiveAnimationsGui(IAnimation &anim)
{
	ImGui::PushID(&anim);
	switch (anim.type())
	{
		case IAnimation::Type::PROPERTY:
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
			createPropertyAnimationGui(propertyAnim);
			break;
		}
		case IAnimation::Type::PARALLEL_GROUP:
		{
			ParallelAnimationGroup &parallelAnim = static_cast<ParallelAnimationGroup &>(anim);
			createParallelAnimationGui(parallelAnim);
			break;
		}
		case IAnimation::Type::SEQUENTIAL_GROUP:
		{
			SequentialAnimationGroup &sequentialAnim = static_cast<SequentialAnimationGroup &>(anim);
			createSequentialAnimationGui(sequentialAnim);
			break;
		}
	}
	ImGui::PopID();
}

// TODO:: Unify with create sequential gui
void UserInterface::createParallelAnimationGui(ParallelAnimationGroup &animGroup)
{
	ASSERT(animGroup.type() == IAnimation::Type::PARALLEL_GROUP);

	if (ImGui::TreeNodeEx("Parallel Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::PROPERTY:
				{
					animGroup.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			animGroup.anims().clear();

		ImGui::Text("State: %s", animStateToString(animGroup.state()));
		if (ImGui::Button("Stop"))
			animGroup.stop();
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
			animGroup.pause();
		ImGui::SameLine();
		if (ImGui::Button("Play"))
			animGroup.play();
		ImGui::Separator();

		for (unsigned int i = 0; i < animGroup.anims().size(); i++)
			createRecursiveAnimationsGui(*animGroup.anims()[i].get());

		ImGui::TreePop();
	}
}

void UserInterface::createSequentialAnimationGui(SequentialAnimationGroup &animGroup)
{
	ASSERT(animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP);

	if (ImGui::TreeNodeEx("Sequential Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
				{

					animGroup.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::PROPERTY:
				{
					animGroup.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
			animGroup.anims().clear();

		ImGui::Text("State: %s", animStateToString(animGroup.state()));
		if (ImGui::Button("Stop"))
			animGroup.stop();
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
			animGroup.pause();
		ImGui::SameLine();
		if (ImGui::Button("Play"))
			animGroup.play();
		ImGui::Separator();

		for (unsigned int i = 0; i < animGroup.anims().size(); i++)
			createRecursiveAnimationsGui(*animGroup.anims()[i].get());

		ImGui::TreePop();
	}
}

void UserInterface::createPropertyAnimationGui(PropertyAnimation &anim)
{
	ASSERT(anim.type() == IAnimation::Type::PROPERTY);

	if (ImGui::TreeNodeEx("Property Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int currentComboProperty = -1;
		const nctl::String &propertyName = anim.propertyName();
		currentComboProperty = PropertyTypesEnum::NONE;
		if (anim.property() != nullptr)
		{
			for (unsigned int i = 0; i < IM_ARRAYSIZE(propertyTypes); i++)
			{
				if (propertyName == propertyTypes[i])
				{
					currentComboProperty = static_cast<PropertyTypesEnum>(i);
					break;
				}
			}
		}
		ImGui::Combo("Property", &currentComboProperty, propertyTypes, IM_ARRAYSIZE(propertyTypes));
		anim.setPropertyName(propertyTypes[currentComboProperty]);
		switch (currentComboProperty)
		{
			case PropertyTypesEnum::NONE:
				break;
			case PropertyTypesEnum::POSITION_X:
				anim.setProperty(&sprite_.x);
				break;
			case PropertyTypesEnum::POSITION_Y:
				anim.setProperty(&sprite_.y);
				break;
			case PropertyTypesEnum::ROTATION:
				anim.setProperty(&sprite_.rotation);
				break;
		}

		int currentComboCurveType = static_cast<int>(anim.curve().type());
		ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
		anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

		int currentComboLoopMode = static_cast<int>(anim.curve().loopMode());
		ImGui::Combo("Loop Mode", &currentComboLoopMode, easingCurveLoopModes, IM_ARRAYSIZE(easingCurveLoopModes));
		anim.curve().setLoopMode(static_cast<EasingCurve::LoopMode>(currentComboLoopMode));

		ImGui::SliderFloat("Shift", &anim.curve().shift(), -500.0f, 500.0f);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Shift"))
			anim.curve().shift() = 0.0f;
		ImGui::SliderFloat("Scale", &anim.curve().scale(), -500.0f, 500.0f);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Scale"))
			anim.curve().scale() = 1.0f;

		ImGui::Separator();
		float time = anim.curve().time();
		ImGui::SliderFloat("Time", &time, 0.0f, 1.0f);
		anim.curve().setTime(time);

		ImGui::Text("State: %s", animStateToString(anim.state()));
		if (ImGui::Button("Stop"))
			anim.stop();
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
			anim.pause();
		ImGui::SameLine();
		if (ImGui::Button("Play"))
			anim.play();

		ImGui::TreePop();
	}
}

