#include <ncine/imgui_internal.h>
#include <ncine/InputEvents.h>

#include "gui/SpritesWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "AnimationManager.h"
#include "SpriteManager.h"
#include "SpriteEntry.h"
#include "Sprite.h"
#include "Texture.h"

namespace {

bool editSpriteName = false;

unsigned int nextSpriteNameId()
{
	static unsigned int spriteNameId = 0;
	return spriteNameId++;
}

void recursiveCloneSprite(const Sprite *parentSprite, Sprite *clonedParentSprite)
{
	// Reverse for loop to add cloned child sprites after the original ones
	for (int i = theSpriteMgr->sprites().size() - 1; i >= 0; i--)
	{
		Sprite *childSprite = theSpriteMgr->sprites()[i];
		if (childSprite->parent() == parentSprite)
		{
			const int index = childSprite->indexInParent();
			childSprite->parentGroup()->children().insertAt(index + 1, nctl::move(childSprite->clone()));

			Sprite *clonedChildSprite = childSprite->parentGroup()->children()[index + 1]->toSprite();
			clonedChildSprite->setParentGroup(childSprite->parentGroup());
			clonedChildSprite->setParent(clonedParentSprite);
			recursiveCloneSprite(childSprite, clonedChildSprite);
		}
	}
}

void moveSpriteEntry(SpriteEntry &spriteEntry, unsigned int indexInParent, bool upDirection)
{
	SpriteGroup *parentGroup = spriteEntry.parentGroup();
	const unsigned int spriteId = spriteEntry.isSprite() ? spriteEntry.spriteId() : 0;
	const int swapIndexDiff = upDirection ? 1 : -1;

	nctl::swap(parentGroup->children()[indexInParent], parentGroup->children()[indexInParent + swapIndexDiff]);

	if (spriteEntry.isSprite())
	{
		// Performing an explicit swap to avoid a call to `updateLinearArray()`
		theSpriteMgr->sprites()[spriteId]->setSpriteId(spriteId + swapIndexDiff);
		theSpriteMgr->sprites()[spriteId + swapIndexDiff]->setSpriteId(spriteId);
		nctl::swap(theSpriteMgr->sprites()[spriteId], theSpriteMgr->sprites()[spriteId + swapIndexDiff]);
	}
	else
		theSpriteMgr->updateSpritesArray();
}

void recursiveUpdateParentOnSpriteRemoval(SpriteGroup &group, Sprite *sprite)
{
	for (unsigned int i = 0; i < group.children().size(); i++)
	{
		Sprite *childSprite = group.children()[i]->toSprite();
		SpriteGroup *childGroup = group.children()[i]->toGroup();
		if (childSprite && childSprite->parent() == sprite)
			childSprite->setParent(nullptr);
		else if (childGroup)
			recursiveUpdateParentOnSpriteRemoval(*childGroup, sprite);
	}
}

void updateParentOnSpriteRemoval(Sprite *sprite)
{
	recursiveUpdateParentOnSpriteRemoval(theSpriteMgr->root(), sprite);
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpritesWindow::SpritesWindow(UserInterface &ui)
    : ui_(ui)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpritesWindow::create()
{
	ImGui::Begin(Labels::Sprites);

	if (theSpriteMgr->textures().isEmpty() == false)
	{
		if (ImGui::Button(Labels::Add))
		{
			if (ui_.selectedTextureIndex_ >= 0 && ui_.selectedTextureIndex_ < theSpriteMgr->textures().size())
			{
				if (theSpriteMgr->sprites().isEmpty())
					ui_.numFrames_ = 0; // force focus on the canvas

				Texture &tex = *theSpriteMgr->textures()[ui_.selectedTextureIndex_];
				Sprite *addedSprite = theSpriteMgr->addSprite(ui_.selectedSpriteEntry_, &tex);
				ui::auxString.format("Sprite%u", nextSpriteNameId());
				addedSprite->name = ui::auxString;
				ui_.selectedSpriteEntry_ = addedSprite;
				theSpriteMgr->updateSpritesArray();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(Labels::AddGroup))
		{
			SpriteGroup *addedGroup = theSpriteMgr->addGroup(ui_.selectedSpriteEntry_);
			ui::auxString.format("Group%u", nextSpriteNameId());
			addedGroup->name() = ui::auxString;
			ui_.selectedSpriteEntry_ = addedGroup;
			theSpriteMgr->updateSpritesArray();
		}

		const bool enableRemoveButton = theSpriteMgr->children().isEmpty() == false && ui_.selectedSpriteEntry_ != &theSpriteMgr->root();
		ImGui::BeginDisabled(enableRemoveButton == false);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Remove) || (enableRemoveButton && ui_.deleteKeyPressed_ && ImGui::IsWindowHovered() && editSpriteName == false))
		{
			if (ui_.selectedSpriteEntry_->isSprite())
				removeSprite();
			else if (ui_.selectedSpriteEntry_->isGroup())
				removeSpriteGroup();
		}
		ImGui::EndDisabled();

		// Repeat the check after the remove button
		const bool enableCloneButton = theSpriteMgr->children().isEmpty() == false && ui_.selectedSpriteEntry_ != &theSpriteMgr->root();
		ImGui::BeginDisabled(enableCloneButton == false);
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Clone))
		{
			if (ui_.selectedSpriteEntry_->isSprite())
				cloneSprite();
			else if (ui_.selectedSpriteEntry_->isGroup())
				cloneSpriteGroup();
		}
		ImGui::EndDisabled();

		const unsigned int indexInParent = ui_.selectedSpriteEntry_->indexInParent();
		SpriteGroup *parentGroup = ui_.selectedSpriteEntry_->parentGroup();

		const bool enableMoveUpButton = enableCloneButton && indexInParent < parentGroup->children().size() - 1;
		ImGui::BeginDisabled(enableMoveUpButton == false);
		if (ImGui::Button(Labels::MoveUp))
			moveSpriteEntry(*ui_.selectedSpriteEntry_, indexInParent, true);
		ImGui::EndDisabled();

		ImGui::SameLine();

		const bool enableMoveDownButton = enableCloneButton && indexInParent > 0;
		ImGui::BeginDisabled(enableMoveDownButton == false);
		if (ImGui::Button(Labels::MoveDown))
			moveSpriteEntry(*ui_.selectedSpriteEntry_, indexInParent, false);
		ImGui::EndDisabled();

		if (ImGui::IsWindowHovered())
		{
			// Disable keyboard navigation
			ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);
			ui_.enableKeyboardNav_ = false;

			if (enableMoveUpButton && ImGui::IsKeyReleased(ImGuiKey_UpArrow))
				moveSpriteEntry(*ui_.selectedSpriteEntry_, indexInParent, true);
			if (enableMoveDownButton && ImGui::IsKeyReleased(ImGuiKey_DownArrow))
				moveSpriteEntry(*ui_.selectedSpriteEntry_, indexInParent, false);
		}

		ImGui::Separator();
	}
	else
		ImGui::Text("Load at least one texture in order to add sprites");

	if (theSpriteMgr->children().isEmpty() == false)
	{
		// Special group list entry for the root group in the sprite manager
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
		if (ui_.selectedSpriteEntry_ == &theSpriteMgr->root())
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		ui::auxString.format("%s Root (%u children)", Labels::GroupIcon, theSpriteMgr->children().size());
		// Force tree expansion to see the selected animation
		if (ui_.selectedSpriteEntry_ && ui_.selectedSpriteEntry_ != &theSpriteMgr->root())
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		const bool treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(&theSpriteMgr->root()), nodeFlags, "%s", ui::auxString.data());
		if (ImGui::IsItemClicked())
			ui_.selectedSpriteEntry_ = &theSpriteMgr->root();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SPRITEENTRY_TREENODE"))
			{
				IM_ASSERT(payload->DataSize == sizeof(DragSpriteEntryPayload));
				const DragSpriteEntryPayload &dragPayload = *reinterpret_cast<const DragSpriteEntryPayload *>(payload->Data);

				nctl::UniquePtr<SpriteEntry> dragEntry(nctl::move(dragPayload.parent.children()[dragPayload.index]));
				ui_.selectedSpriteEntry_ = dragEntry.get(); // set before moving the unique pointer
				dragPayload.parent.children().removeAt(dragPayload.index);

				dragEntry->setParentGroup(&theSpriteMgr->root());
				theSpriteMgr->children().pushBack(nctl::move(dragEntry));
				theSpriteMgr->updateSpritesArray();
			}

			ImGui::EndDragDropTarget();
		}

		if (treeIsOpen)
		{
			// Reversed order to match the rendering layer order
			for (int i = theSpriteMgr->children().size() - 1; i >= 0; i--)
				createSpriteListEntry(*theSpriteMgr->children()[i], i);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void SpritesWindow::createSpriteListEntry(SpriteEntry &entry, unsigned int index)
{
	static bool setFocus = false;

	Sprite *sprite = entry.toSprite();
	SpriteGroup *groupEntry = entry.toGroup();

	bool treeIsOpen = false;
	ui::auxString.clear();
	if (sprite)
	{
		if (sprite->parentGroup() == &theSpriteMgr->root())
		{
			ui::auxString.format("###SpriteColor%lu", reinterpret_cast<uintptr_t>(sprite));
			ImGui::ColorEdit3(ui::auxString.data(), entry.entryColor().data(), ImGuiColorEditFlags_NoInputs);
			ImGui::SameLine();
		}

		// To preserve indentation no group is created
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (ui_.selectedSpriteEntry_ == &entry)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		ui::auxString.format("%s###Sprite%lu", sprite->visible ? Labels::VisibleIcon : Labels::InvisibleIcon, reinterpret_cast<uintptr_t>(sprite));
		ImGui::Checkbox(ui::auxString.data(), &sprite->visible);
		ImGui::SameLine();

		ui::auxString.format("#%u: \"%s\" (%d x %d) %s", sprite->spriteId(), sprite->name.data(), sprite->width(), sprite->height(),
		                     &sprite->texture() == theSpriteMgr->textures()[ui_.selectedTextureIndex_].get() ? Labels::SelectedTextureIcon : "");
		if (editSpriteName && &entry == ui_.selectedSpriteEntry_)
		{
			ui::auxString.format("###SpriteName%lu", reinterpret_cast<uintptr_t>(sprite));
			if (setFocus)
			{
				ImGui::SetKeyboardFocusHere();
				setFocus = false;
			}
			ImGui::InputText(ui::auxString.data(), sprite->name.data(), sprite->name.capacity(),
			                 ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue, ui::inputTextCallback, &sprite->name);
			if (ImGui::IsItemDeactivated())
				editSpriteName = false;
		}
		else
			ImGui::TreeNodeEx(static_cast<void *>(sprite), nodeFlags, "%s", ui::auxString.data());

		if (ImGui::BeginPopupContextItem())
		{
			ui_.selectedSpriteEntry_ = sprite;

			const bool enableSelectParent = sprite->parent() != nullptr;
			ImGui::BeginDisabled(enableSelectParent == false);
			if (ImGui::MenuItem(Labels::SelectParent))
				ui_.selectedSpriteEntry_ = sprite->parent();
			ImGui::EndDisabled();
			if (ImGui::MenuItem(Labels::SelectParentGroup))
				ui_.selectedSpriteEntry_ = sprite->parentGroup();
			ImGui::Separator();
			if (ImGui::MenuItem(Labels::Clone))
				cloneSprite();
			if (ImGui::MenuItem(Labels::Remove))
				removeSprite();
			ImGui::EndPopup();
		}

		if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > ui::ImageTooltipDelay && editSpriteName == false)
		{
			if (sprite->texture().width() > 0 && sprite->texture().height() > 0 &&
			    sprite->texRect().w > 0 && sprite->texRect().h > 0)
			{
				const float invTextureWidth = 1.0f / sprite->texture().width();
				const float invTextureHeight = 1.0f / sprite->texture().height();
				const float aspectRatio = sprite->texRect().w / static_cast<float>(sprite->texRect().h);
				const nc::Recti texRect = sprite->flippingTexRect();
				const ImVec2 uv0(texRect.x * invTextureWidth, texRect.y * invTextureHeight);
				const ImVec2 uv1(uv0.x + texRect.w * invTextureWidth, uv0.y + texRect.h * invTextureHeight);

				float width = ui::ImageTooltipSize;
				float height = ui::ImageTooltipSize;
				if (aspectRatio > 1.0f)
					height = width / aspectRatio;
				else
					width *= aspectRatio;

				ImGui::BeginTooltip();
				ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(sprite->texture().imguiTexId())), ImVec2(width, height), uv0, uv1);
				ImGui::EndTooltip();
			}
		}
	}
	else if (groupEntry)
	{
		ui::auxString.format("###GroupColor%lu", reinterpret_cast<uintptr_t>(groupEntry));
		ImGui::ColorEdit3(ui::auxString.data(), groupEntry->entryColor().data(), ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();

		// To preserve indentation no group is created
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
		if (ui_.selectedSpriteEntry_ == &entry)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		ui::auxString.format("%s \"%s\"", Labels::GroupIcon, groupEntry->name().data());

		if (editSpriteName && ui_.selectedSpriteEntry_ == &entry)
		{
			ui::auxString.format("###GroupName%lu", reinterpret_cast<uintptr_t>(groupEntry));
			if (setFocus)
			{
				ImGui::SetKeyboardFocusHere();
				setFocus = false;
			}
			ImGui::InputText(ui::auxString.data(), groupEntry->name().data(), groupEntry->name().capacity(),
			                 ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue, ui::inputTextCallback, &groupEntry->name());
			if (ImGui::IsItemDeactivated())
				editSpriteName = false;
		}
		else
			treeIsOpen = ImGui::TreeNodeEx(static_cast<void *>(groupEntry), nodeFlags, "%s", ui::auxString.data());

		if (ImGui::BeginPopupContextItem())
		{
			ui_.selectedSpriteEntry_ = groupEntry;

			if (ImGui::MenuItem(Labels::SelectParentGroup))
				ui_.selectedSpriteEntry_ = groupEntry->parentGroup();
			ImGui::Separator();
			if (ImGui::MenuItem(Labels::Clone))
				cloneSpriteGroup();
			if (ImGui::MenuItem(Labels::Remove))
				removeSpriteGroup();
			ImGui::EndPopup();
		}
	}

	if (ImGui::IsItemClicked())
	{
		ui_.selectedSpriteEntry_ = &entry;
		editSpriteName = false;
		if (ImGui::GetIO().KeyCtrl)
		{
			editSpriteName = true;
			setFocus = true;
		}
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		ASSERT(entry.toGroup() != &theSpriteMgr->root());
		DragSpriteEntryPayload dragPayload = { *entry.parentGroup(), index };
		ImGui::SetDragDropPayload("SPRITEENTRY_TREENODE", &dragPayload, sizeof(DragSpriteEntryPayload));
		ImGui::Text("%s", ui::auxString.data());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SPRITEENTRY_TREENODE"))
		{
			IM_ASSERT(payload->DataSize == sizeof(DragSpriteEntryPayload));
			const DragSpriteEntryPayload &dragPayload = *reinterpret_cast<const DragSpriteEntryPayload *>(payload->Data);

			bool dragIsPossible = true;
			if (dragPayload.parent.children()[dragPayload.index]->isGroup())
			{
				SpriteGroup *dragGrop = dragPayload.parent.children()[dragPayload.index]->toGroup();
				SpriteGroup *destParent = entry.parentGroup();
				while (destParent != nullptr)
				{
					if (destParent == dragGrop)
					{
						// Trying to drag a group inside one of its children
						dragIsPossible = false;
						break;
					}
					destParent = destParent->parentGroup();
				}
			}

			if (dragIsPossible)
			{
				nctl::UniquePtr<SpriteEntry> dragEntry(nctl::move(dragPayload.parent.children()[dragPayload.index]));
				ui_.selectedSpriteEntry_ = dragEntry.get(); // set before moving the unique pointer
				dragPayload.parent.children().removeAt(dragPayload.index);

				if (entry.isGroup())
				{
					dragEntry->setParentGroup(entry.toGroup());
					entry.toGroup()->children().pushBack(nctl::move(dragEntry));
				}
				else
				{
					dragEntry->setParentGroup(entry.parentGroup());
					entry.parentGroup()->children().insertAt(index, nctl::move(dragEntry));
				}
				theSpriteMgr->updateSpritesArray();
			}
		}

		ImGui::EndDragDropTarget();
	}

	if (entry.isGroup() && treeIsOpen)
	{
		// Reversed order to match the rendering layer order
		for (int i = groupEntry->children().size() - 1; i >= 0; i--)
			createSpriteListEntry(*groupEntry->children()[i], i);

		ImGui::TreePop();
	}
}

void SpritesWindow::cloneSprite()
{
	// TODO: Should the name of a cloned anim, animGroup, sprite and spriteGroup have a "_%u" id suffix?
	ASSERT(ui_.selectedSpriteEntry_->isSprite());
	const Sprite *selectedSprite = ui_.selectedSpriteEntry_->toSprite();

	const int index = ui_.selectedSpriteEntry_->indexInParent();
	ui_.selectedSpriteEntry_->parentGroup()->children().insertAt(index + 1, nctl::move(selectedSprite->clone()));

	Sprite *clonedSprite = ui_.selectedSpriteEntry_->parentGroup()->children()[index + 1]->toSprite();
	clonedSprite->setParentGroup(ui_.selectedSpriteEntry_->parentGroup());
	recursiveCloneSprite(selectedSprite, clonedSprite);

	theSpriteMgr->updateSpritesArray();
	ui_.selectedSpriteEntry_ = clonedSprite;
}

void SpritesWindow::cloneSpriteGroup()
{
	ASSERT(ui_.selectedSpriteEntry_->isGroup());
	const SpriteGroup *selectedGroup = ui_.selectedSpriteEntry_->toGroup();

	const int index = ui_.selectedSpriteEntry_->indexInParent();
	ui_.selectedSpriteEntry_->parentGroup()->children().insertAt(index + 1, nctl::move(selectedGroup->clone()));
	SpriteGroup *clonedGroup = ui_.selectedSpriteEntry_->parentGroup()->children()[index + 1]->toGroup();
	clonedGroup->setParentGroup(ui_.selectedSpriteEntry_->parentGroup());

	theSpriteMgr->updateSpritesArray();
	ui_.selectedSpriteEntry_ = clonedGroup;
}

void SpritesWindow::removeSprite()
{
	ASSERT(ui_.selectedSpriteEntry_->isSprite());
	Sprite *selectedSprite = ui_.selectedSpriteEntry_->toSprite();

	SpriteEntry *newSelection = nullptr;
	// Update sprite entry selection
	nctl::Array<nctl::UniquePtr<SpriteEntry>> &children = selectedSprite->parentGroup()->children();
	const int index = selectedSprite->indexInParent();

	if (index > 0)
		newSelection = children[index - 1].get();
	else
	{
		if (children.size() > 1)
			newSelection = children[index + 1].get();
		else
			newSelection = selectedSprite->parentGroup();
	}
	children.removeAt(index);

	ui_.updateSelectedAnimOnSpriteRemoval(selectedSprite); // always before removing the animation
	theAnimMgr->removeSprite(selectedSprite);
	updateParentOnSpriteRemoval(selectedSprite);
	theSpriteMgr->updateSpritesArray();
	ui_.selectedSpriteEntry_ = newSelection;
}

void SpritesWindow::recursiveRemoveSpriteGroup(SpriteGroup &group)
{
	for (unsigned int i = 0; i < group.children().size(); i++)
	{
		Sprite *childSprite = group.children()[i]->toSprite();
		SpriteGroup *childGroup = group.children()[i]->toGroup();
		if (childSprite)
		{
			ui_.updateSelectedAnimOnSpriteRemoval(childSprite); // always before removing the animation
			theAnimMgr->removeSprite(childSprite);
			updateParentOnSpriteRemoval(childSprite);
		}
		else if (childGroup)
			recursiveRemoveSpriteGroup(*childGroup);
	}
}

void SpritesWindow::removeSpriteGroup()
{
	ASSERT(ui_.selectedSpriteEntry_->isGroup());
	SpriteGroup *selectedGroup = ui_.selectedSpriteEntry_->toGroup();
	recursiveRemoveSpriteGroup(*selectedGroup);

	SpriteEntry *newSelection = nullptr;
	// Update sprite entry selection
	nctl::Array<nctl::UniquePtr<SpriteEntry>> &children = selectedGroup->parentGroup()->children();
	const int index = selectedGroup->indexInParent();

	if (index > 0)
		newSelection = children[index - 1].get();
	else
	{
		if (children.size() > 1)
			newSelection = children[index + 1].get();
		else
			newSelection = selectedGroup->parentGroup();
	}
	children.removeAt(index);

	theSpriteMgr->updateSpritesArray();
	ui_.selectedSpriteEntry_ = newSelection;
}
