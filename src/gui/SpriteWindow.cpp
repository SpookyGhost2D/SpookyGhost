#include <ncine/imgui_internal.h>

#include "gui/SpriteWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "Sprite.h"
#include "Texture.h"

namespace {

// clang-format off
const char *anchorPointItems[] = { "Center", "Bottom Left", "Top Left", "Bottom Right", "Top Right" };
enum AnchorPointsEnum { CENTER, BOTTOM_LEFT, TOP_LEFT, BOTTOM_RIGHT, TOP_RIGHT };
const char *blendingPresets[] = { "Disabled", "Alpha", "Pre-multiplied Alpha", "Additive", "Multiply" };
// clang-format on

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

SpriteWindow::SpriteWindow(UserInterface &ui)
    : ui_(ui), spriteGraph_(4)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteWindow::create()
{
	ImGui::Begin(Labels::Sprite);

	if (ui_.selectedSpriteEntry_->isSprite())
	{
		Sprite &sprite = *ui_.selectedSpriteEntry_->toSprite();

		ImGui::InputText("Name", sprite.name.data(), Sprite::MaxNameLength,
		                 ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &sprite.name);

		int currentTextureCombo = theSpriteMgr->textureIndex(&sprite.texture());
		ui::comboString.clear();
		for (unsigned int i = 0; i < theSpriteMgr->textures().size(); i++)
		{
			const Texture &currentTex = *theSpriteMgr->textures()[i];
			ui::comboString.formatAppend("#%u: \"%s\" (%d x %d)", i, currentTex.name().data(), currentTex.width(), currentTex.height());
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		ImGui::Combo("Texture", &currentTextureCombo, ui::comboString.data());
		Texture *newTexture = theSpriteMgr->textures()[currentTextureCombo].get();
		if (&sprite.texture() != newTexture)
		{
			nc::Recti texRect = sprite.texRect();
			sprite.setTexture(newTexture);
			if (newTexture->width() > texRect.x && newTexture->height() > texRect.y)
			{
				if (newTexture->width() < texRect.x + texRect.w)
					texRect.w = newTexture->width() - texRect.x;
				if (newTexture->height() < texRect.y + texRect.h)
					texRect.h = newTexture->width() - texRect.y;
				sprite.setTexRect(texRect);
			}
		}

		// Create an array of sprites that can be a parent of the selected one
		static SpriteEntry *lastSelectedSpriteEntry = nullptr;
		if (lastSelectedSpriteEntry != ui_.selectedSpriteEntry_)
		{
			spriteGraph_.clear();
			spriteGraph_.pushBack(SpriteStruct(-1, nullptr));
			for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
				theSpriteMgr->sprites()[i]->visited = false;
			visitSprite(sprite);
			for (unsigned int i = 0; i < theSpriteMgr->sprites().size(); i++)
			{
				if (theSpriteMgr->sprites()[i]->visited == false)
					spriteGraph_.pushBack(SpriteStruct(i, theSpriteMgr->sprites()[i]));
			}
			lastSelectedSpriteEntry = ui_.selectedSpriteEntry_;
		}

		const ImVec2 colorButtonComboSize(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());
		Sprite *parentSprite = sprite.parent();

		if (ImGui::BeginCombo("Parent", "", ImGuiComboFlags_CustomPreview))
		{
			// Reversed order to match the rendering layer order
			for (int i = spriteGraph_.size() - 1; i >= 0; i--)
			{
				const int index = spriteGraph_[i].index;
				Sprite *currentSprite = spriteGraph_[i].sprite;

				if (index >= 0)
				{
					ImGui::PushID(currentSprite);
					const bool isSelected = (parentSprite == currentSprite);
					if (ImGui::Selectable("", isSelected))
						parentSprite = currentSprite;

					if (isSelected)
						ImGui::SetItemDefaultFocus();

					ImGui::SameLine();
					nc::Colorf entryColor = currentSprite->entryColor();
					if (currentSprite->parentGroup() != &theSpriteMgr->root())
						entryColor = currentSprite->parentGroup()->entryColor();
					const ImVec4 color(entryColor);
					ImGui::ColorButton(currentSprite->name.data(), color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
					ImGui::SameLine();
					ImGui::Text("#%d: \"%s\"", currentSprite->spriteId(), currentSprite->name.data());
					ImGui::PopID();
				}
				else
				{
					if (ImGui::Selectable("None", parentSprite == nullptr))
						parentSprite = nullptr;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginComboPreview())
		{
			if (parentSprite != nullptr)
			{
				nc::Colorf entryColor = parentSprite->entryColor();
				if (parentSprite->parentGroup() != &theSpriteMgr->root())
					entryColor = parentSprite->parentGroup()->entryColor();
				const ImVec4 entryVecColor(entryColor);
				ImGui::ColorButton(parentSprite->name.data(), entryVecColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, colorButtonComboSize);
				ImGui::SameLine();
				ImGui::Text("#%d: \"%s\"", parentSprite->spriteId(), parentSprite->name.data());
			}
			else
				ImGui::TextUnformatted("None");
			ImGui::EndComboPreview();
		}

		const Sprite *prevParent = sprite.parent();
		if (prevParent != parentSprite)
		{
			const nc::Vector2f absPosition = sprite.absPosition();
			sprite.setParent(parentSprite);
			sprite.setAbsPosition(absPosition);
		}

		ImGui::Separator();

		nc::Vector2f position(sprite.x, sprite.y);
		ImGui::SliderFloat2("Position", position.data(), 0.0f, static_cast<float>(theCanvas->texWidth()));
		sprite.x = roundf(position.x);
		sprite.y = roundf(position.y);
		ImGui::SliderFloat("Rotation", &sprite.rotation, 0.0f, 360.0f);
		ImGui::SliderFloat2("Scale", sprite.scaleFactor.data(), 0.0f, 8.0f);
		ImGui::SameLine();
		ui::auxString.format("%s##Scale", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
			sprite.scaleFactor.set(1.0f, 1.0f);

		const float halfBiggerDimension = sprite.width() > sprite.height() ? sprite.width() * 0.5f : sprite.height() * 0.5f;
		ImGui::SliderFloat2("Anchor Point", sprite.anchorPoint.data(), -halfBiggerDimension, halfBiggerDimension);
		static int currentAnchorSelection = 0;
		if (ImGui::Combo("Anchor Presets", &currentAnchorSelection, anchorPointItems, IM_COUNTOF(anchorPointItems)))
		{
			switch (currentAnchorSelection)
			{
				case AnchorPointsEnum::CENTER:
					sprite.anchorPoint.set(0.0f, 0.0f);
					break;
				case AnchorPointsEnum::BOTTOM_LEFT:
					sprite.anchorPoint.set(-sprite.width() * 0.5f, sprite.height() * 0.5f);
					break;
				case AnchorPointsEnum::TOP_LEFT:
					sprite.anchorPoint.set(-sprite.width() * 0.5f, -sprite.height() * 0.5f);
					break;
				case AnchorPointsEnum::BOTTOM_RIGHT:
					sprite.anchorPoint.set(sprite.width() * 0.5f, sprite.height() * 0.5f);
					break;
				case AnchorPointsEnum::TOP_RIGHT:
					sprite.anchorPoint.set(sprite.width() * 0.5f, -sprite.height() * 0.5f);
					break;
			}
		}
		sprite.anchorPoint.x = roundf(sprite.anchorPoint.x);
		sprite.anchorPoint.y = roundf(sprite.anchorPoint.y);

		if (sprite.parent() != nullptr)
		{
			ImGui::Text("Abs Position: %f, %f", sprite.absPosition().x, sprite.absPosition().y);
			ImGui::Text("Abs Rotation: %f", sprite.absRotation());
			ImGui::Text("Abs Scale: %f, %f", sprite.absScaleFactor().x, sprite.absScaleFactor().y);
		}

		ImGui::Separator();
		nc::Recti texRect = sprite.texRect();
		int minX = texRect.x;
		int maxX = minX + texRect.w;
		ImGui::DragIntRange2("Rect X", &minX, &maxX, 1.0f, 0, sprite.texture().width());

		int minY = texRect.y;
		int maxY = minY + texRect.h;
		ImGui::DragIntRange2("Rect Y", &minY, &maxY, 1.0f, 0, sprite.texture().height());

		texRect.x = minX;
		texRect.w = maxX - minX;
		texRect.y = minY;
		texRect.h = maxY - minY;
		if (texRect.x < 0)
			texRect.x = 0;
		if (texRect.w <= 0)
			texRect.w = 1;
		if (texRect.y < 0)
			texRect.y = 0;
		if (texRect.h <= 0)
			texRect.h = 1;

		ImGui::SameLine();
		ui::auxString.format("%s##Rect", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
			texRect = nc::Recti(0, 0, sprite.texture().width(), sprite.texture().height());

		const nc::Recti currentTexRect = sprite.texRect();
		if (texRect.x != currentTexRect.x || texRect.y != currentTexRect.y ||
		    texRect.w != currentTexRect.w || texRect.h != currentTexRect.h)
		{
			sprite.setTexRect(texRect);
		}

		bool isFlippedX = sprite.isFlippedX();
		ImGui::Checkbox("Flipped X", &isFlippedX);
		ImGui::SameLine();
		bool isFlippedY = sprite.isFlippedY();
		ImGui::Checkbox("Flipped Y", &isFlippedY);

		if (isFlippedX != sprite.isFlippedX())
			sprite.setFlippedX(isFlippedX);
		if (isFlippedY != sprite.isFlippedY())
			sprite.setFlippedY(isFlippedY);

		ImGui::Separator();
		int currentRgbBlendingPreset = static_cast<int>(sprite.rgbBlendingPreset());
		ImGui::Combo("RGB Blending", &currentRgbBlendingPreset, blendingPresets, IM_COUNTOF(blendingPresets));
		sprite.setRgbBlendingPreset(static_cast<Sprite::BlendingPreset>(currentRgbBlendingPreset));

		int currentAlphaBlendingPreset = static_cast<int>(sprite.alphaBlendingPreset());
		ImGui::Combo("Alpha Blending", &currentAlphaBlendingPreset, blendingPresets, IM_COUNTOF(blendingPresets));
		sprite.setAlphaBlendingPreset(static_cast<Sprite::BlendingPreset>(currentAlphaBlendingPreset));

		ImGui::ColorEdit4("Color", sprite.color.data(), ImGuiColorEditFlags_AlphaBar);
		ImGui::SameLine();
		ui::auxString.format("%s##Color", Labels::Reset);
		if (ImGui::Button(ui::auxString.data()))
			sprite.color = nc::Colorf::White;
	}

	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void SpriteWindow::visitSprite(Sprite &sprite)
{
	sprite.visited = true;
	for (unsigned int i = 0; i < sprite.children().size(); i++)
		visitSprite(*sprite.children()[i]);
}
