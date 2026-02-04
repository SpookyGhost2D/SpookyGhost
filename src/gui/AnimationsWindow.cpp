#include <ncine/imgui_internal.h>

#include "gui/AnimationsWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "GridAnimation.h"
#include "GridFunction.h"
#include "Script.h"
#include "ScriptAnimation.h"
#include "Sprite.h"
#include "ScriptManager.h"
#include "SpriteManager.h"
#include "SpriteEntry.h"

namespace {

// clang-format off
const char *animationTypes[] = { "Parallel Group", "Sequential Group", "Property", "Grid", "Script" };
enum AnimationTypesEnum { PARALLEL_GROUP, SEQUENTIAL_GROUP, PROPERTY, GRID, SCRIPT };
// clang-format on

IAnimation *removeAnimWithContextMenu = nullptr;
bool editAnimName = false;

unsigned int nextAnimNameId()
{
	static unsigned int animNameId = 0;
	return animNameId++;
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

AnimationsWindow::AnimationsWindow(UserInterface &ui)
    : ui_(ui)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationsWindow::removeAnimation()
{
	IAnimation *newSelection = nullptr;
	if (ui_.selectedAnimation_->parent())
	{
		nctl::Array<nctl::UniquePtr<IAnimation>> &anims = ui_.selectedAnimation_->parent()->anims();
		const int index = ui_.selectedAnimation_->indexInParent();
		if (index > 0)
			newSelection = anims[index - 1].get();
		else
		{
			if (anims.size() > 1)
				newSelection = anims[index + 1].get();
			else
				newSelection = ui_.selectedAnimation_->parent();
		}
	}

	theAnimMgr->removeAnimation(ui_.selectedAnimation_);
	ui_.selectedAnimation_ = newSelection;
}

void AnimationsWindow::create()
{
	ImGui::Begin(Labels::Animations);

	static int currentComboAnimType = 0;
	ImGui::PushItemWidth(ImGui::GetFontSize() * 12.0f);
	ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button(Labels::Add))
	{
		nctl::Array<nctl::UniquePtr<IAnimation>> *anims = &theAnimMgr->anims();
		AnimationGroup *parent = &theAnimMgr->animGroup();
		if (ui_.selectedAnimation_)
		{
			if (ui_.selectedAnimation_->isGroup())
			{
				AnimationGroup *animGroup = static_cast<AnimationGroup *>(ui_.selectedAnimation_);
				anims = &animGroup->anims();
				parent = animGroup;
			}
			else if (ui_.selectedAnimation_->parent() != nullptr)
			{
				anims = &ui_.selectedAnimation_->parent()->anims();
				parent = ui_.selectedAnimation_->parent();
			}
		}
		Sprite *selectedSprite = ui_.selectedSpriteEntry_->toSprite();

		// Search the index of the selected animation in its parent
		unsigned int selectedIndex = anims->size() - 1;
		for (unsigned int i = 0; i < anims->size(); i++)
		{
			if ((*anims)[i].get() == ui_.selectedAnimation_)
			{
				selectedIndex = i;
				break;
			}
		}

		switch (currentComboAnimType)
		{
			case AnimationTypesEnum::PARALLEL_GROUP:
				anims->insertAt(++selectedIndex, nctl::makeUnique<ParallelAnimationGroup>());
				break;
			case AnimationTypesEnum::SEQUENTIAL_GROUP:
				anims->insertAt(++selectedIndex, nctl::makeUnique<SequentialAnimationGroup>());
				break;
			case AnimationTypesEnum::PROPERTY:
				anims->insertAt(++selectedIndex, nctl::makeUnique<PropertyAnimation>(selectedSprite));
				static_cast<PropertyAnimation &>(*(*anims)[selectedIndex]).setProperty(Properties::Types::NONE);
				break;
			case AnimationTypesEnum::GRID:
				anims->insertAt(++selectedIndex, nctl::makeUnique<GridAnimation>(selectedSprite));
				break;
			case AnimationTypesEnum::SCRIPT:
				Script *selectedScript = nullptr;
				if (theScriptingMgr->scripts().isEmpty() == false &&
				    ui_.selectedScriptIndex_ >= 0 && ui_.selectedScriptIndex_ <= theScriptingMgr->scripts().size() - 1)
				{
					selectedScript = theScriptingMgr->scripts()[ui_.selectedScriptIndex_].get();
				}
				anims->insertAt(++selectedIndex, nctl::makeUnique<ScriptAnimation>(selectedSprite, selectedScript));
				break;
		}
		(*anims)[selectedIndex]->setParent(parent);
		ui_.selectedAnimation_ = (*anims)[selectedIndex].get();
		if (ui_.selectedAnimation_->isGroup())
			ui::auxString.format("AnimGroup%u", nextAnimNameId());
		else
			ui::auxString.format("Anim%u", nextAnimNameId());
		ui_.selectedAnimation_->name = ui::auxString;
	}

	// Remove an animation as instructed by the context menu
	if (removeAnimWithContextMenu)
	{
		ui_.selectedAnimation_ = removeAnimWithContextMenu;
		removeAnimation();
		removeAnimWithContextMenu = nullptr;
	}

	const bool enableRemoveButton = ui_.selectedAnimation_ && ui_.selectedAnimation_ != &theAnimMgr->animGroup();
	ImGui::BeginDisabled(enableRemoveButton == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Remove) || (ui_.deleteKeyPressed_ && ImGui::IsWindowHovered() && editAnimName == false))
		removeAnimation();
	ImGui::EndDisabled();

	// Search the index of the selected animation in its parent
	int selectedIndex = ui_.selectedAnimation_ ? ui_.selectedAnimation_->indexInParent() : -1;

	// Repeat the check after the remove button
	const bool enableCloneButton = ui_.selectedAnimation_ && ui_.selectedAnimation_ != &theAnimMgr->animGroup();
	ImGui::BeginDisabled(enableCloneButton == false);
	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Clone))
		ui_.selectedAnimation_->parent()->anims().insertAt(++selectedIndex, nctl::move(ui_.selectedAnimation_->clone()));
	ImGui::EndDisabled();

	const bool enableControlButtons = ui_.selectedAnimation_ != nullptr && theAnimMgr->anims().isEmpty() == false;
	const bool enableStopButton = enableControlButtons && ui_.selectedAnimation_->isStopped() == false;
	const bool enablePauseButton = enableControlButtons && ui_.selectedAnimation_->isPlaying() == true;
	const bool enablePlayButton = enableControlButtons && ui_.selectedAnimation_->isPlaying() == false;

	ImGui::BeginDisabled(enableControlButtons == false);
	ImGui::BeginDisabled(enableStopButton == false);
	if (ImGui::Button(Labels::Stop))
		ui_.selectedAnimation_->stop();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(enablePauseButton == false);
	if (ImGui::Button(Labels::Pause))
		ui_.selectedAnimation_->pause();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(enablePlayButton == false);
	if (ImGui::Button(Labels::Play))
		ui_.selectedAnimation_->play();
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

	const bool enableMoveUpButton = enableCloneButton && selectedIndex > 0;
	ImGui::BeginDisabled(enableMoveUpButton == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::MoveUp))
		nctl::swap(ui_.selectedAnimation_->parent()->anims()[selectedIndex], ui_.selectedAnimation_->parent()->anims()[selectedIndex - 1]);
	ImGui::EndDisabled();

	const bool enableMoveDownButton = enableCloneButton && selectedIndex < ui_.selectedAnimation_->parent()->anims().size() - 1;
	ImGui::BeginDisabled(enableMoveDownButton == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::MoveDown))
		nctl::swap(ui_.selectedAnimation_->parent()->anims()[selectedIndex], ui_.selectedAnimation_->parent()->anims()[selectedIndex + 1]);
	ImGui::EndDisabled();

	if (ImGui::IsWindowHovered())
	{
		// Disable keyboard navigation
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);
		ui_.enableKeyboardNav_ = false;

		if (enableMoveUpButton && ImGui::IsKeyReleased(ImGuiKey_UpArrow))
			nctl::swap(ui_.selectedAnimation_->parent()->anims()[selectedIndex], ui_.selectedAnimation_->parent()->anims()[selectedIndex - 1]);
		if (enableMoveDownButton && ImGui::IsKeyReleased(ImGuiKey_DownArrow))
			nctl::swap(ui_.selectedAnimation_->parent()->anims()[selectedIndex], ui_.selectedAnimation_->parent()->anims()[selectedIndex + 1]);
	}

	ImGui::PushItemWidth(ImGui::GetFontSize() * 16.0f);
	ImGui::SliderFloat("Speed Multiplier", &theAnimMgr->speedMultiplier(), 0.0f, 5.0f);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ui::auxString.format("%s##Speed Multiplier", Labels::Reset);
	if (ImGui::Button(ui::auxString.data()))
		theAnimMgr->speedMultiplier() = 1.0f;

	ImGui::Separator();

	if (theAnimMgr->anims().isEmpty() == false)
	{
		// Special animation list entry for the root animation group in the animation manager
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
		if (ui_.selectedAnimation_ == &theAnimMgr->animGroup())
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		ui::auxString.format("%s Root (%u children)", Labels::GroupIcon, theAnimMgr->anims().size());
		if (theAnimMgr->animGroup().isStopped())
			ui::auxString.formatAppend(" %s", Labels::StopIcon);
		else if (theAnimMgr->animGroup().isPaused())
			ui::auxString.formatAppend(" %s", Labels::PauseIcon);
		else if (theAnimMgr->animGroup().isPlaying())
			ui::auxString.formatAppend(" %s", Labels::PlayIcon);

		// Force tree expansion to see the selected animation
		if (ui_.selectedAnimation_ && ui_.selectedAnimation_ != &theAnimMgr->animGroup())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		const bool treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&theAnimMgr->animGroup()), nodeFlags, "%s", ui::auxString.data());
		if (ImGui::IsItemClicked())
			ui_.selectedAnimation_ = &theAnimMgr->animGroup();

		if (ImGui::BeginPopupContextItem())
		{
			ui_.selectedAnimation_ = &theAnimMgr->animGroup();

			const bool enableStopButton = ui_.selectedAnimation_->isStopped() == false;
			const bool enablePauseButton = ui_.selectedAnimation_->isPlaying() == true;
			const bool enablePlayButton = ui_.selectedAnimation_->isPlaying() == false;

			ImGui::BeginDisabled(enableStopButton == false);
			if (ImGui::MenuItem(Labels::Stop))
				ui_.selectedAnimation_->stop();
			ImGui::EndDisabled();
			ImGui::BeginDisabled(enablePauseButton == false);
			if (ImGui::MenuItem(Labels::Pause))
				ui_.selectedAnimation_->pause();
			ImGui::EndDisabled();
			ImGui::BeginDisabled(enablePlayButton == false);
			if (ImGui::MenuItem(Labels::Play))
				ui_.selectedAnimation_->play();
			ImGui::EndDisabled();

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ANIMATION_TREENODE"))
			{
				IM_ASSERT(payload->DataSize == sizeof(DragAnimationPayload));
				const DragAnimationPayload &dragPayload = *reinterpret_cast<const DragAnimationPayload *>(payload->Data);

				nctl::UniquePtr<IAnimation> dragAnimation(nctl::move(dragPayload.parent.anims()[dragPayload.index]));
				ui_.selectedAnimation_ = dragAnimation.get(); // set before moving the unique pointer
				dragPayload.parent.anims().removeAt(dragPayload.index);

				dragAnimation->setParent(&theAnimMgr->animGroup());
				theAnimMgr->anims().pushBack(nctl::move(dragAnimation));
			}

			ImGui::EndDragDropTarget();
		}

		if (treeIsOpen)
		{
			unsigned int animId = 0;
			for (unsigned int i = 0; i < theAnimMgr->anims().size(); i++)
				createAnimationListEntry(*theAnimMgr->anims()[i], i, animId);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void AnimationsWindow::createAnimationListEntry(IAnimation &anim, unsigned int index, unsigned int &animId)
{
	static bool setFocus = false;

	ImGuiTreeNodeFlags nodeFlags = anim.isGroup() ? (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen)
	                                              : (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	if (&anim == ui_.selectedAnimation_)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	AnimationGroup *animGroup = nullptr;
	if (anim.isGroup())
		animGroup = static_cast<AnimationGroup *>(&anim);

	// To preserve indentation no group is created
	if (anim.isSprite())
	{
		SpriteAnimation &spriteAnim = static_cast<SpriteAnimation &>(anim);
		SpriteEntry *spriteEntry = spriteAnim.sprite();

		if (spriteEntry != nullptr)
		{
			ui::auxString.format("###AnimColor%lu", reinterpret_cast<uintptr_t>(spriteEntry));
			nc::Colorf entryColor = spriteEntry->entryColor();
			if (spriteEntry->parentGroup() != &theSpriteMgr->root())
				entryColor = spriteEntry->parentGroup()->entryColor();
			const ImVec4 entryVecColor(entryColor);
			ImGui::ColorButton(ui::auxString.data(), entryVecColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop);
			ImGui::SameLine();
		}
	}

	ui::auxString.format("%s###Anim%lu", anim.enabled ? Labels::EnabledAnimIcon : Labels::DisabledAnimIcon, reinterpret_cast<uintptr_t>(&anim));
	ImGui::Checkbox(ui::auxString.data(), &anim.enabled);
	ImGui::SameLine();

	ui::auxString.clear();
	if (anim.isGroup())
		ui::auxString.format("%s ", Labels::GroupIcon);
	ui::auxString.formatAppend("#%u: ", animId);
	if (anim.name.isEmpty() == false)
		ui::auxString.formatAppend("\"%s\" (", anim.name.data());
	if (anim.type() == IAnimation::Type::PARALLEL_GROUP)
		ui::auxString.formatAppend("Parallel Group (%u children)", animGroup->anims().size());
	else if (anim.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		ui::auxString.formatAppend("Sequential Group (%u children)", animGroup->anims().size());

	if (anim.isSprite())
	{
		SpriteAnimation &spriteAnim = static_cast<SpriteAnimation &>(anim);

		if (anim.type() == IAnimation::Type::PROPERTY)
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
			ui::auxString.formatAppend("%s property", propertyAnim.propertyName());
		}
		else if (anim.type() == IAnimation::Type::GRID)
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
			ui::auxString.formatAppend("%s grid", (gridAnim.function() != nullptr) ? gridAnim.function()->name().data() : "None");
		}
		else if (anim.type() == IAnimation::Type::SCRIPT)
		{
			ui::auxString.append("Script");
		}

		if (spriteAnim.sprite() != nullptr)
		{
			if (spriteAnim.sprite()->name.isEmpty() == false)
				ui::auxString.formatAppend(" for sprite \"%s\"", spriteAnim.sprite()->name.data());
			if (spriteAnim.sprite() == ui_.selectedSpriteEntry_)
				ui::auxString.formatAppend(" %s", Labels::SelectedSpriteIcon);
			if (spriteAnim.isLocked())
				ui::auxString.formatAppend(" %s", Labels::LockedAnimIcon);
		}
	}
	if (anim.name.isEmpty() == false)
		ui::auxString.append(")");

	if (anim.state() == IAnimation::State::STOPPED)
		ui::auxString.formatAppend(" %s", Labels::StopIcon);
	else if (anim.state() == IAnimation::State::PAUSED)
		ui::auxString.formatAppend(" %s", Labels::PauseIcon);
	else if (anim.state() == IAnimation::State::PLAYING)
		ui::auxString.formatAppend(" %s", Labels::PlayIcon);

	// Force tree expansion to see the selected animation
	if (ui_.selectedAnimation_ && animGroup && (ui_.selectedAnimation_ != animGroup))
	{
		if (animGroup == ui_.selectedAnimation_->parent())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	}

	bool treeIsOpen = false;
	if (editAnimName && ui_.selectedAnimation_ == &anim)
	{
		ui::auxString.format("###AnimationName%lu", reinterpret_cast<uintptr_t>(&anim));
		if (setFocus)
		{
			ImGui::SetKeyboardFocusHere();
			setFocus = false;
		}

		if ((nodeFlags & ImGuiTreeNodeFlags_Leaf) == 0)
		{
			nodeFlags |= ImGuiTreeNodeFlags_AllowOverlap;
			treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&anim), nodeFlags, "");
			ImGui::SameLine();
		}
		ImGui::InputText(ui::auxString.data(), anim.name.data(), anim.name.capacity(),
		                 ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue, ui::inputTextCallback, &anim.name);
		if (ImGui::IsItemDeactivated())
			editAnimName = false;
	}
	else
		treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&anim), nodeFlags, "%s", ui::auxString.data());

	if (ImGui::IsItemClicked())
	{
		if (ui_.selectedAnimation_ != &anim)
			editAnimName = false;
		ui_.selectedAnimation_ = &anim;
		if (ImGui::GetIO().KeyCtrl)
		{
			editAnimName = true;
			setFocus = true;
		}

		if (anim.isSprite())
		{
			SpriteAnimation &spriteAnim = static_cast<SpriteAnimation &>(anim);
			if (spriteAnim.sprite())
				ui_.selectedSpriteEntry_ = spriteAnim.sprite();
		}
	}

	if (ImGui::BeginPopupContextItem())
	{
		ui_.selectedAnimation_ = &anim;

		const bool enableStopButton = ui_.selectedAnimation_->isStopped() == false;
		const bool enablePauseButton = ui_.selectedAnimation_->isPlaying() == true;
		const bool enablePlayButton = ui_.selectedAnimation_->isPlaying() == false;

		ImGui::BeginDisabled(enableStopButton == false);
		if (ImGui::MenuItem(Labels::Stop))
			ui_.selectedAnimation_->stop();
		ImGui::EndDisabled();
		ImGui::BeginDisabled(enablePauseButton == false);
		if (ImGui::MenuItem(Labels::Pause))
			ui_.selectedAnimation_->pause();
		ImGui::EndDisabled();
		ImGui::BeginDisabled(enablePlayButton == false);
		if (ImGui::MenuItem(Labels::Play))
			ui_.selectedAnimation_->play();
		ImGui::EndDisabled();

		ImGui::Separator();

		const bool showLocked = anim.isGroup() == false;
		if (showLocked)
		{
			CurveAnimation &curveAnim = static_cast<CurveAnimation &>(anim);
			bool locked = curveAnim.isLocked();
			ui::auxString.format("Locked %s", Labels::LockedAnimIcon);
			ImGui::Checkbox(ui::auxString.data(), &locked);
			curveAnim.setLocked(locked);
		}
		if (ImGui::MenuItem(Labels::SelectParent))
			ui_.selectedAnimation_ = anim.parent();

		ImGui::Separator();

		if (ImGui::MenuItem(Labels::Clone))
			ui_.selectedAnimation_->parent()->anims().insertAt(++index, nctl::move(ui_.selectedAnimation_->clone()));
		if (ImGui::MenuItem(Labels::Remove))
			removeAnimWithContextMenu = &anim;

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		DragAnimationPayload dragPayload = { *anim.parent(), index };
		ImGui::SetDragDropPayload("ANIMATION_TREENODE", &dragPayload, sizeof(DragAnimationPayload));
		ImGui::Text("%s", ui::auxString.data());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ANIMATION_TREENODE"))
		{
			IM_ASSERT(payload->DataSize == sizeof(DragAnimationPayload));
			const DragAnimationPayload &dragPayload = *reinterpret_cast<const DragAnimationPayload *>(payload->Data);

			bool dragIsPossible = true;
			if (dragPayload.parent.anims()[dragPayload.index]->isGroup())
			{
				AnimationGroup *dragAnimGroup = static_cast<AnimationGroup *>(dragPayload.parent.anims()[dragPayload.index].get());
				AnimationGroup *destParent = anim.parent();
				while (destParent != nullptr)
				{
					if (destParent == dragAnimGroup)
					{
						// Trying to drag a parent group inside a children
						dragIsPossible = false;
						break;
					}
					destParent = destParent->parent();
				}
			}

			if (dragIsPossible)
			{
				nctl::UniquePtr<IAnimation> dragAnimation(nctl::move(dragPayload.parent.anims()[dragPayload.index]));
				ui_.selectedAnimation_ = dragAnimation.get(); // set before moving the unique pointer
				dragPayload.parent.anims().removeAt(dragPayload.index);
				if (anim.isGroup())
				{
					dragAnimation->setParent(animGroup);
					animGroup->anims().pushBack(nctl::move(dragAnimation));
				}
				else
				{
					dragAnimation->setParent(anim.parent());
					anim.parent()->anims().insertAt(index, nctl::move(dragAnimation));
				}
			}
		}

		ImGui::EndDragDropTarget();
	}

	animId++;
	if (anim.isGroup() && treeIsOpen)
	{
		for (unsigned int i = 0; i < animGroup->anims().size(); i++)
			createAnimationListEntry(*animGroup->anims()[i], i, animId);

		ImGui::TreePop();
	}
}
