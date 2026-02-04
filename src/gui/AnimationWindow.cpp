#include <ncine/imgui_internal.h>
#include <ncine/FileSystem.h>

#include "gui/AnimationWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "GridAnimation.h"
#include "GridFunctionLibrary.h"
#include "Script.h"
#include "ScriptAnimation.h"
#include "ScriptManager.h"
#include "Sprite.h"

namespace {

// clang-format off
const char *easingCurveTypes[] = { "Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sine", "Exponential", "Circular" };
const char *loopDirections[] = { "Forward", "Backward" };
const char *loopModes[] = { "Disabled", "Rewind", "Ping Pong" };
// clang-format on

const int PlotArraySize = 512;
float plotArray[PlotArraySize];
int plotValueIndex = 0;

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

AnimationWindow::AnimationWindow(UserInterface &ui)
    : ui_(ui)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationWindow::create()
{
	ImGui::Begin(Labels::Animation);

	if (theAnimMgr->anims().isEmpty() == false && ui_.selectedAnimation_ != nullptr && ui_.selectedAnimation_ != &theAnimMgr->animGroup())
	{
		IAnimation &anim = *ui_.selectedAnimation_;
		if (anim.type() == IAnimation::Type::PROPERTY)
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(*ui_.selectedAnimation_);
			createPropertyAnimationGui(propertyAnim);
		}
		else if (anim.type() == IAnimation::Type::GRID)
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(*ui_.selectedAnimation_);
			createGridAnimationGui(gridAnim);
		}
		else if (anim.type() == IAnimation::Type::SCRIPT)
		{
			ScriptAnimation &scriptAnim = static_cast<ScriptAnimation &>(*ui_.selectedAnimation_);
			createScriptAnimationGui(scriptAnim);
		}
		else if (anim.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		{
			SequentialAnimationGroup &sequentialAnim = static_cast<SequentialAnimationGroup &>(*ui_.selectedAnimation_);

			ImGui::InputText("Name", sequentialAnim.name.data(), IAnimation::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &sequentialAnim.name);

			createDelayAnimationGui(sequentialAnim);
			createLoopAnimationGui(sequentialAnim.loop());
			createOverrideSpriteGui(sequentialAnim);
		}
		else if (anim.type() == IAnimation::Type::PARALLEL_GROUP)
		{
			ParallelAnimationGroup &parallelAnim = static_cast<ParallelAnimationGroup &>(*ui_.selectedAnimation_);

			ImGui::InputText("Name", parallelAnim.name.data(), IAnimation::MaxNameLength,
			                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &parallelAnim.name);

			createDelayAnimationGui(parallelAnim);
			createLoopAnimationGui(parallelAnim.loop());
			createOverrideSpriteGui(parallelAnim);
		}
	}

	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationWindow::createDelayAnimationGui(IAnimation &anim)
{
	float delay = anim.delay();
	ImGui::SliderFloat("Delay", &delay, 0.0f, 10.0f, "%.3fs");
	if (delay < 0.0f)
		delay = 0.0f;
	anim.setDelay(delay);
	ui::auxString.format("%.3fs / %.3fs", anim.currentDelay(), anim.delay());
	if (anim.delay() > 0)
		ImGui::ProgressBar(anim.currentDelay() / anim.delay(), ImVec2(0.0f, 0.0f), ui::auxString.data());
}

void AnimationWindow::createLoopAnimationGui(LoopComponent &loop)
{
	int currentLoopDirection = static_cast<int>(loop.direction());
	ImGui::Combo("Direction", &currentLoopDirection, loopDirections, IM_ARRAYSIZE(loopDirections));
	loop.setDirection(static_cast<Loop::Direction>(currentLoopDirection));

	int currentLoopMode = static_cast<int>(loop.mode());
	ImGui::Combo("Loop Mode", &currentLoopMode, loopModes, IM_ARRAYSIZE(loopModes));
	loop.setMode(static_cast<Loop::Mode>(currentLoopMode));

	if (loop.mode() != Loop::Mode::DISABLED)
	{
		float loopDelay = loop.delay();
		ImGui::SliderFloat("Loop Delay", &loopDelay, 0.0f, 10.0f, "%.3fs");
		if (loopDelay < 0.0f)
			loopDelay = 0.0f;
		loop.setDelay(loopDelay);
		ui::auxString.format("%.3fs / %.3fs", loop.currentDelay(), loopDelay);
		if (loopDelay > 0.0f)
			ImGui::ProgressBar(loop.currentDelay() / loopDelay, ImVec2(0.0f, 0.0f), ui::auxString.data());
	}
}

void AnimationWindow::createOverrideSpriteGui(AnimationGroup &animGroup)
{
	if (animGroup.anims().isEmpty() == false)
	{
		static Sprite *animSprite = nullptr;
		const ImVec2 colorButtonComboSize(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());

		bool comboReturnValue = false;
		if ((comboReturnValue = ImGui::BeginCombo("Sprite##Override", "", ImGuiComboFlags_CustomPreview)))
		{
			// Reversed order to match the rendering layer order
			for (int i = theSpriteMgr->sprites().size() - 1; i >= 0; i--)
			{
				Sprite *sprite = theSpriteMgr->sprites()[i];

				ImGui::PushID(sprite);
				const bool isSelected = (sprite == animSprite);
				if (ImGui::Selectable("", isSelected))
					animSprite = sprite;

				if (isSelected)
					ImGui::SetItemDefaultFocus();

				ImGui::SameLine();
				nc::Colorf entryColor = sprite->entryColor();
				if (sprite->parentGroup() != &theSpriteMgr->root())
					entryColor = sprite->parentGroup()->entryColor();
				const ImVec4 color(entryColor);
				ImGui::ColorButton(sprite->name.data(), color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
				ImGui::SameLine();
				ImGui::Text("#%d: \"%s\"", sprite->spriteId(), sprite->name.data());
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginComboPreview())
		{
			if (animSprite != nullptr)
			{
				nc::Colorf entryColor = animSprite->entryColor();
				if (animSprite->parentGroup() != &theSpriteMgr->root())
					entryColor = animSprite->parentGroup()->entryColor();
				const ImVec4 entryVecColor(entryColor);
				ImGui::ColorButton(animSprite->name.data(), entryVecColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
				ImGui::SameLine();
				ImGui::Text("#%d: \"%s\"", animSprite->spriteId(), animSprite->name.data());
			}
			else
				ImGui::TextUnformatted("None");
			ImGui::EndComboPreview();
		}

		ImGui::SameLine();
		if (ImGui::Button(Labels::Apply))
			theAnimMgr->overrideSprite(animGroup, animSprite);
	}
}

void AnimationWindow::createCurveAnimationGui(CurveAnimation &anim, const CurveAnimationGuiLimits &limits)
{
	createDelayAnimationGui(anim);

	int currentComboCurveType = static_cast<int>(anim.curve().type());
	ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
	anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

	createLoopAnimationGui(anim.curve().loop());

	ImGui::SliderFloat("Shift", &anim.curve().shift(), limits.minShift, limits.maxShift);
	ImGui::SameLine();
	ui::auxString.format("%s##Shift", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.curve().shift() = 0.0f;
	ImGui::SliderFloat("Scale", &anim.curve().scale(), limits.minScale, limits.maxScale);
	ImGui::SameLine();
	ui::auxString.format("%s##Scale", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.curve().scale() = 1.0f;

	ImGui::Separator();
	ImGui::SliderFloat("Speed", &anim.speed(), 0.0f, 5.0f);
	ImGui::SameLine();
	ui::auxString.format("%s##Speed", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		anim.speed() = 1.0f;

	ImGui::SliderFloat("Initial", &anim.curve().initialValue(), 0.0f, 1.0f);
	ImGui::SameLine();
	ImGui::Checkbox("##InitialEnabled", &anim.curve().hasInitialValue());

	ImGui::SliderFloat("Start", &anim.curve().start(), 0.0f, 1.0f);
	ImGui::SliderFloat("End", &anim.curve().end(), 0.0f, 1.0f);
	ImGui::SliderFloat("Time", &anim.curve().time(), anim.curve().start(), anim.curve().end());

	if (anim.curve().start() > anim.curve().end() ||
	    anim.curve().end() < anim.curve().start())
	{
		anim.curve().start() = anim.curve().end();
	}

	if (anim.curve().hasInitialValue() == false)
	{
		if (anim.curve().loop().direction() == Loop::Direction::FORWARD)
			anim.curve().setInitialValue(anim.curve().start());
		else
			anim.curve().setInitialValue(anim.curve().end());
	}
	else
	{
		if (anim.curve().initialValue() < anim.curve().start())
			anim.curve().initialValue() = anim.curve().start();
		else if (anim.curve().initialValue() > anim.curve().end())
			anim.curve().initialValue() = anim.curve().end();
	}

	if (anim.curve().time() < anim.curve().start())
		anim.curve().time() = anim.curve().start();
	else if (anim.curve().time() > anim.curve().end())
		anim.curve().time() = anim.curve().end();

	plotArray[plotValueIndex] = anim.curve().value();
	ui::auxString.format("%f", plotArray[plotValueIndex]);
	ImGui::PlotLines("Values", plotArray, PlotArraySize, 0, ui::auxString.data());
	ImGui::SameLine();
	ui::auxString.format("%s##Values", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
	{
		plotValueIndex = 0;
		for (unsigned int i = 0; i < PlotArraySize; i++)
			plotArray[i] = 0.0f;
	}

	if (anim.state() == IAnimation::State::PLAYING)
	{
		plotValueIndex++;
		if (plotValueIndex >= PlotArraySize)
			plotValueIndex = 0;
	}
}

bool createCustomSpritesCombo(SpriteAnimation &anim)
{
	Sprite *animSprite = anim.sprite();
	const ImVec2 colorButtonComboSize(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());
	Sprite *selectedSprite = animSprite;

	bool comboReturnValue = false;
	if ((comboReturnValue = ImGui::BeginCombo("Sprite", "", ImGuiComboFlags_CustomPreview)))
	{
		// Reversed order to match the rendering layer order
		for (int i = theSpriteMgr->sprites().size() - 1; i >= 0; i--)
		{
			Sprite *sprite = theSpriteMgr->sprites()[i];

			ImGui::PushID(sprite);
			const bool isSelected = (sprite == animSprite);
			if (ImGui::Selectable("", isSelected))
				selectedSprite = sprite;

			if (isSelected)
				ImGui::SetItemDefaultFocus();

			ImGui::SameLine();
			nc::Colorf entryColor = sprite->entryColor();
			if (sprite->parentGroup() != &theSpriteMgr->root())
				entryColor = sprite->parentGroup()->entryColor();
			const ImVec4 color(entryColor);
			ImGui::ColorButton(sprite->name.data(), color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
			ImGui::SameLine();
			ImGui::Text("#%d: \"%s\" (%d x %d)", sprite->spriteId(), sprite->name.data(), sprite->width(), sprite->height());
			ImGui::PopID();
		}

		if (ImGui::Selectable("None", animSprite == nullptr))
			selectedSprite = nullptr;

		ImGui::EndCombo();
	}

	if (ImGui::BeginComboPreview())
	{
		if (animSprite != nullptr)
		{
			nc::Colorf entryColor = animSprite->entryColor();
			if (animSprite->parentGroup() != &theSpriteMgr->root())
				entryColor = animSprite->parentGroup()->entryColor();
			const ImVec4 entryVecColor(entryColor);
			ImGui::ColorButton(animSprite->name.data(), entryVecColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
			ImGui::SameLine();
			ImGui::Text("#%d: \"%s\" (%d x %d)", animSprite->spriteId(), animSprite->name.data(), animSprite->width(), animSprite->height());
		}
		else
			ImGui::TextUnformatted("None");
		ImGui::EndComboPreview();
	}

	if (comboReturnValue)
		anim.setSprite(selectedSprite);

	return comboReturnValue;
}

void AnimationWindow::createPropertyAnimationGui(PropertyAnimation &anim)
{
	static CurveAnimationGuiLimits limits;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		const bool comboReturnValue = createCustomSpritesCombo(anim);
		if (comboReturnValue && anim.sprite() != nullptr)
			ui_.selectedSpriteEntry_ = anim.sprite();

		Sprite &sprite = *anim.sprite();

		static int currentComboProperty = -1;
		currentComboProperty = static_cast<Properties::Types>(anim.propertyType());

		bool setCurveShift = false;
		if (ImGui::Combo("Property", &currentComboProperty, Properties::Strings, IM_ARRAYSIZE(Properties::Strings)))
			setCurveShift = true;
		anim.setProperty(static_cast<Properties::Types>(currentComboProperty));
		switch (currentComboProperty)
		{
			case Properties::Types::NONE:
				break;
			case Properties::Types::POSITION_X:
				limits.minShift = -sprite.width() * 0.5f;
				limits.maxShift = theCanvas->texWidth() + sprite.width() * 0.5f;
				limits.minScale = -theCanvas->texWidth();
				limits.maxScale = theCanvas->texWidth();
				break;
			case Properties::Types::POSITION_Y:
				limits.minShift = -sprite.height() * 0.5f;
				limits.maxShift = theCanvas->texHeight() + sprite.height() * 0.5f;
				limits.minScale = -theCanvas->texHeight();
				limits.maxScale = theCanvas->texHeight();
				break;
			case Properties::Types::ROTATION:
				limits.minShift = 0.0f;
				limits.maxShift = 360.0f;
				limits.minScale = -360.0f;
				limits.maxScale = 360.0f;
				break;
			case Properties::Types::SCALE_X:
				limits.minShift = -8.0f;
				limits.maxShift = 8.0f;
				limits.minScale = -8.0f;
				limits.maxScale = 8.0f;
				break;
			case Properties::Types::SCALE_Y:
				limits.minShift = -8.0f;
				limits.maxShift = 8.0f;
				limits.minScale = -8.0f;
				limits.maxScale = 8.0f;
				break;
			case Properties::Types::ANCHOR_X:
				limits.minShift = -sprite.width() * 0.5f;
				limits.maxShift = sprite.width() * 0.5f;
				limits.minScale = -sprite.width();
				limits.maxScale = sprite.width();
				break;
			case Properties::Types::ANCHOR_Y:
				limits.minShift = -sprite.height() * 0.5f;
				limits.maxShift = sprite.height() * 0.5f;
				limits.minScale = -sprite.height();
				limits.maxScale = sprite.height();
				break;
			case Properties::Types::OPACITY:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_R:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_G:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
			case Properties::Types::COLOR_B:
				limits.minShift = 0.0f;
				limits.maxShift = 1.0f;
				limits.minScale = -1.0f;
				limits.maxScale = 1.0f;
				break;
		}
		if (setCurveShift && anim.property())
			anim.curve().setShift(*anim.property());

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	createCurveAnimationGui(anim, limits);
}

void AnimationWindow::createGridAnimationGui(GridAnimation &anim)
{
	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		const bool comboReturnValue = createCustomSpritesCombo(anim);
		if (comboReturnValue && anim.sprite() != nullptr)
			ui_.selectedSpriteEntry_ = anim.sprite();

		int currentComboFunction = 0;
		ui::comboString.clear();
		ui::comboString.formatAppend("None");
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < GridFunctionLibrary::gridFunctions().size(); i++)
		{
			const nctl::String &functionName = GridFunctionLibrary::gridFunctions()[i].name();
			ui::comboString.formatAppend("%s", functionName.data());
			ui::comboString.setLength(ui::comboString.length() + 1);

			if (anim.function() && functionName == anim.function()->name())
				currentComboFunction = i + 1;
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		ImGui::Combo("Function", &currentComboFunction, ui::comboString.data());
		const GridFunction *gridFunction = (currentComboFunction > 0) ? &GridFunctionLibrary::gridFunctions()[currentComboFunction - 1] : nullptr;
		if (anim.function() != gridFunction)
			anim.setFunction(gridFunction);

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);

		if (anim.function() != nullptr)
		{
			for (unsigned int i = 0; i < anim.function()->numParameters(); i++)
			{
				const GridFunction::ParameterInfo &paramInfo = anim.function()->parameterInfo(i);
				ui::auxString.format("%s##GridFunction%u", paramInfo.name.data(), i);
				float minValue = paramInfo.minValue.value0;
				float maxValue = paramInfo.maxValue.value0;

				if (anim.sprite())
				{
					if (paramInfo.minMultiply == GridFunction::ValueMultiply::SPRITE_WIDTH)
						minValue *= anim.sprite()->width();
					else if (paramInfo.minMultiply == GridFunction::ValueMultiply::SPRITE_HEIGHT)
						minValue *= anim.sprite()->height();

					if (paramInfo.maxMultiply == GridFunction::ValueMultiply::SPRITE_WIDTH)
						maxValue *= anim.sprite()->width();
					else if (paramInfo.maxMultiply == GridFunction::ValueMultiply::SPRITE_HEIGHT)
						maxValue *= anim.sprite()->height();
				}

				switch (paramInfo.type)
				{
					case GridFunction::ParameterType::FLOAT:
						ImGui::SliderFloat(ui::auxString.data(), &anim.parameters()[i].value0, minValue, maxValue);
						break;
					case GridFunction::ParameterType::VECTOR2F:
						ImGui::SliderFloat2(ui::auxString.data(), &anim.parameters()[i].value0, minValue, maxValue);
						break;
				}

				if (anim.sprite())
				{
					if (paramInfo.anchorType == GridFunction::AnchorType::X)
						anim.sprite()->gridAnchorPoint.x = anim.parameters()[i].value0;
					else if (paramInfo.anchorType == GridFunction::AnchorType::Y)
						anim.sprite()->gridAnchorPoint.y = anim.parameters()[i].value0;
					else if (paramInfo.anchorType == GridFunction::AnchorType::XY)
					{
						anim.sprite()->gridAnchorPoint.x = anim.parameters()[i].value0;
						anim.sprite()->gridAnchorPoint.y = anim.parameters()[i].value1;
					}
				}

				ImGui::SameLine();
				ui::auxString.format("%s##GridFunction%u", Labels::Reset, i);
				if (ImGui::Button(ui::auxString.data()))
				{
					anim.parameters()[i].value0 = paramInfo.initialValue.value0;
					anim.parameters()[i].value1 = paramInfo.initialValue.value1;
				}
			}
		}
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	createCurveAnimationGui(anim, limits);
}

void AnimationWindow::createScriptAnimationGui(ScriptAnimation &anim)
{
	CurveAnimationGuiLimits limits;
	limits.minScale = -10.0f;
	limits.maxScale = 10.0f;
	limits.minShift = -100.0f;
	limits.maxShift = 100.0f;

	ImGui::InputText("Name", anim.name.data(), IAnimation::MaxNameLength,
	                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &anim.name);
	ImGui::Separator();

	if (theSpriteMgr->sprites().isEmpty() == false)
	{
		const bool comboReturnValue = createCustomSpritesCombo(anim);
		if (comboReturnValue && anim.sprite() != nullptr)
			ui_.selectedSpriteEntry_ = anim.sprite();
	}
	else
		ImGui::TextDisabled("There are no sprites to animate");

	int scriptIndex = theScriptingMgr->scriptIndex(anim.script());
	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		// Assign to current script if none was assigned before
		if (scriptIndex < 0)
			scriptIndex = ui_.selectedScriptIndex_;

		ui::comboString.clear();
		for (unsigned int i = 0; i < theScriptingMgr->scripts().size(); i++)
		{
			Script &script = *theScriptingMgr->scripts()[i];
			ui::comboString.formatAppend("#%u: \"%s\" %s", i, nc::fs::baseName(script.name().data()).data(),
			                             script.canRun() ? Labels::CheckIcon : Labels::TimesIcon);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		// Also assign if no script was assigned before
		if (ImGui::Combo("Script", &scriptIndex, ui::comboString.data()) || anim.script() == nullptr)
		{
			anim.setScript(theScriptingMgr->scripts()[scriptIndex].get());
			ui_.selectedScriptIndex_ = scriptIndex;
		}

		ImGui::SameLine();
		bool isLocked = anim.isLocked();
		ImGui::Checkbox(Labels::Locked, &isLocked);
		anim.setLocked(isLocked);
	}
	else
		ImGui::TextDisabled("There are no scripts to use for animation");

	createCurveAnimationGui(anim, limits);
}
