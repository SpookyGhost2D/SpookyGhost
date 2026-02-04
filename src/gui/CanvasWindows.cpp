#include <ncine/imgui_internal.h>

#include "gui/CanvasWindows.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "Sprite.h"
#include "Texture.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

CanvasWindows::CanvasWindows(UserInterface &ui)
    : ui_(ui), hoveringOnCanvasWindow_(false), hoveringOnCanvas_(false)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void CanvasWindows::createCanvasWindow()
{
	const float canvasZoom = ui_.canvasGuiSection_.zoomAmount();

	hoveringOnCanvasWindow_ = false;
	ImGui::SetNextWindowSize(ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImGuiCond_Once);
	ImGui::Begin(Labels::Canvas, nullptr, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::IsWindowHovered())
		hoveringOnCanvasWindow_ = true;
	ui_.canvasGuiSection_.create(*theCanvas);

	const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(theCanvas->imguiTexId())),
	             ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	ImDrawList *drawList = ImGui::GetWindowDrawList();
	const float lineThickness = (canvasZoom < 1.0f) ? 1.0f : canvasZoom;
	if (ui_.canvasGuiSection_.showBorders())
	{
		const ImRect borderRect(cursorScreenPos.x, cursorScreenPos.y, cursorScreenPos.x + theCanvas->texWidth() * canvasZoom, cursorScreenPos.y + theCanvas->texHeight() * canvasZoom);
		drawList->AddRect(borderRect.Min, borderRect.Max, ImColor(1.0f, 0.0f, 1.0f, 1.0f), 0.0f, ImDrawFlags_RoundCornersAll, lineThickness);
	}

	hoveringOnCanvas_ = false;
	if (ImGui::IsItemHovered() && theSpriteMgr->sprites().isEmpty() == false && ui_.selectedSpriteEntry_->isSprite())
	{
		// Disable keyboard navigation for an easier sprite move with arrow keys
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);
		ui_.enableKeyboardNav_ = false;

		hoveringOnCanvas_ = true;
		const ImVec2 mousePos = ImGui::GetMousePos();
		const ImVec2 relativePos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);
		Sprite &sprite = *ui_.selectedSpriteEntry_->toSprite();
		const nc::Vector2f newSpriteAbsPos(roundf(relativePos.x / canvasZoom), roundf(relativePos.y / canvasZoom));
		const nc::Vector2f spriteRelativePos(newSpriteAbsPos - sprite.absPosition());

		static bool shiftAndClick = false;
		static bool ctrlAndClick = false;
		if (ImGui::GetIO().KeyShift || ImGui::GetIO().KeyCtrl)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				shiftAndClick = ImGui::GetIO().KeyShift;
				ctrlAndClick = ImGui::GetIO().KeyCtrl;

				// One frame lasting message
				ui_.statusMessage_.format("Coordinates: %d, %d", static_cast<int>(spriteRelativePos.x), static_cast<int>(spriteRelativePos.y));

				ImU32 color = IM_COL32(0, 0, 0, 255); // opaque black
				if (ImGui::GetIO().KeyShift)
					color |= 0x000000FF; // red
				if (ImGui::GetIO().KeyCtrl)
					color |= 0x00FF0000; // blue

				const ImRect spriteRect(cursorScreenPos.x + (sprite.absPosition().x - sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y - sprite.absHeight() / 2) * canvasZoom,
				                        cursorScreenPos.x + (sprite.absPosition().x + sprite.absWidth() / 2) * canvasZoom,
				                        cursorScreenPos.y + (sprite.absPosition().y + sprite.absHeight() / 2) * canvasZoom);
				drawList->AddRect(spriteRect.Min, spriteRect.Max, color, 0.0f, ImDrawFlags_RoundCornersAll, lineThickness);
				if (spriteRect.Contains(mousePos))
				{
					drawList->AddLine(ImVec2(spriteRect.Min.x, mousePos.y), ImVec2(spriteRect.Max.x, mousePos.y), color, lineThickness);
					drawList->AddLine(ImVec2(mousePos.x, spriteRect.Min.y), ImVec2(mousePos.x, spriteRect.Max.y), color, lineThickness);
				}

				spriteProps_.save(sprite);

				const float rectHalfSize = 2.0f * canvasZoom;
				drawList->AddRectFilled(ImVec2(mousePos.x - rectHalfSize, mousePos.y - rectHalfSize), ImVec2(mousePos.x + rectHalfSize, mousePos.y + rectHalfSize), color);
				if (ImGui::GetIO().KeyCtrl &&
				    (spriteRelativePos.x != sprite.gridAnchorPoint.x || spriteRelativePos.y != sprite.gridAnchorPoint.y))
				{
					// Update grid anchor point while pressing Ctrl, clicking and moving the mouse
					sprite.gridAnchorPoint = spriteRelativePos;
					theAnimMgr->assignGridAnchorToParameters(&sprite);
				}
			}
			if (ImGui::GetIO().KeyShift && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				spriteProps_.restore(sprite);

				// Update sprite anchor point while pressing Shift and releasing the mouse button
				sprite.anchorPoint = spriteRelativePos;
				sprite.setAbsPosition(newSpriteAbsPos);
			}
		}
		else
		{
			if (shiftAndClick || ctrlAndClick)
			{
				spriteProps_.restore(sprite);

				if (shiftAndClick)
				{
					// Update sprite anchor point while clicking the mouse button and releasing the Shift key
					sprite.anchorPoint = spriteRelativePos;
					sprite.setAbsPosition(newSpriteAbsPos);
				}
			}
			else if (ImGui::GetIO().KeyAlt && (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f) || ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
			{
				// Update sprite position while pressing Alt, clicking and moving the mouse (with pixel snapping)
				if (sprite.absPosition().x != newSpriteAbsPos.x || sprite.absPosition().y != newSpriteAbsPos.y)
					sprite.setAbsPosition(newSpriteAbsPos);
			}

			shiftAndClick = false;
			ctrlAndClick = false;
		}
	}

	mouseWheelCanvasZoom();

	ImGui::End();
}

void CanvasWindows::createTexRectWindow()
{
	const float canvasZoom = ui_.canvasGuiSection_.zoomAmount();
	static MouseStatus mouseStatus_ = MouseStatus::IDLE;
	static ImVec2 startPos(0.0f, 0.0f);
	static ImVec2 endPos(0.0f, 0.0f);
	static bool moveRect = false;
	static bool resizeRect = false;
	static ImVec2 resizeDir(0.0f, 0.0f);

	Sprite &sprite = *ui_.selectedSpriteEntry_->toSprite();
	const ImVec2 size = ui_.selectedSpriteEntry_->isSprite()
	                        ? ImVec2(sprite.texture().width() * canvasZoom, sprite.texture().height() * canvasZoom)
	                        : ImVec2(theCanvas->texWidth() * canvasZoom, theCanvas->texHeight() * canvasZoom);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);

	ImGui::Begin(Labels::TexRect, nullptr, ImGuiWindowFlags_HorizontalScrollbar);

	ui::auxString.format("Zoom: %.2f", ui_.canvasGuiSection_.zoomAmount());
	const ImVec2 closeItemSpacing(ImGui::GetStyle().ItemSpacing.x * 0.5f, ImGui::GetStyle().ItemSpacing.y * 0.5f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, closeItemSpacing);
	if (ImGui::Button(Labels::PlusIcon))
		ui_.canvasGuiSection_.increaseZoom();
	ImGui::SameLine();
	if (ImGui::Button(Labels::MinusIcon))
		ui_.canvasGuiSection_.decreaseZoom();
	ImGui::PopStyleVar();

	ImGui::SameLine();
	if (ImGui::Button(ui::auxString.data()))
		ui_.canvasGuiSection_.resetZoom();
	ImGui::Separator();

	if (ui_.selectedSpriteEntry_->isSprite())
	{
		const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
		nc::Recti texRect = sprite.texRect();
		ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(sprite.imguiTexId())), size);

		mouseWheelCanvasZoom();

		if (ImGui::IsItemHovered())
		{
			// Disable keyboard navigation
			ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NavEnableKeyboard);
			ui_.enableKeyboardNav_ = false;

			const ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 relPos(mousePos.x - cursorScreenPos.x, mousePos.y - cursorScreenPos.y);

			relPos.x = roundf(relPos.x / canvasZoom);
			relPos.y = roundf(relPos.y / canvasZoom);

			// One frame lasting message
			ui_.statusMessage_.format("Coordinates: %d, %d", static_cast<int>(relPos.x), static_cast<int>(relPos.y));

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				mouseStatus_ = MouseStatus::CLICKED;
				startPos = ImGui::GetMousePos();

				// Only moving the rectangle if the user starts dragging from inside it
				const bool insideTexRect = (relPos.x >= texRect.x) && (relPos.x <= (texRect.x + texRect.w)) &&
				                           (relPos.y >= texRect.y) && (relPos.y <= (texRect.y + texRect.h));

				moveRect = (resizeRect == false && ImGui::GetIO().KeyAlt && insideTexRect);
				// Pressing Alt and dragging from outside the rectangle will do nothing
				if (moveRect == false && ImGui::GetIO().KeyAlt)
					mouseStatus_ = MouseStatus::IDLE;

				resizeRect = (moveRect == false && ImGui::GetIO().KeyShift && insideTexRect);

				// Determining if the user has clicked a corner or a side
				const float threshold = 0.2f;
				resizeDir.x = 0;
				if ((relPos.x - texRect.x) <= texRect.w * threshold)
					resizeDir.x = -1;
				else if ((relPos.x - texRect.x) >= texRect.w * (1.0f - threshold))
					resizeDir.x = 1;

				resizeDir.y = 0;
				if ((relPos.y - texRect.y) <= texRect.h * threshold)
					resizeDir.y = -1;
				else if ((relPos.y - texRect.y) >= texRect.h * (1.0f - threshold))
					resizeDir.y = 1;

				if (resizeDir.x == 0 && resizeDir.y == 0)
					resizeRect = false;

				// Pressing Shift and dragging from outside the rectangle will do nothing
				if (resizeRect == false && ImGui::GetIO().KeyShift)
					mouseStatus_ = MouseStatus::IDLE;
			}
		}

		if (ImGui::IsWindowHovered() && mouseStatus_ != MouseStatus::IDLE && mouseStatus_ != MouseStatus::RELEASED)
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				mouseStatus_ = MouseStatus::DRAGGING;
				endPos = ImGui::GetMousePos();

				// If the user releases the Alt key while dragging
				if (moveRect && ImGui::GetIO().KeyAlt == false)
					mouseStatus_ = MouseStatus::RELEASED;

				// If the user releases the Shift key while dragging
				if (resizeRect && ImGui::GetIO().KeyShift == false)
					mouseStatus_ = MouseStatus::RELEASED;
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				mouseStatus_ = MouseStatus::RELEASED;
				endPos = ImGui::GetMousePos();
			}
		}
		else if (mouseStatus_ == MouseStatus::CLICKED || mouseStatus_ == MouseStatus::DRAGGING)
			mouseStatus_ = MouseStatus::RELEASED;

		if (moveRect == false && resizeRect == false)
		{
			if (mouseStatus_ == MouseStatus::CLICKED)
			{
				// Clipping to image
				if (startPos.x > cursorScreenPos.x + size.x)
					startPos.x = cursorScreenPos.x + size.x;
				if (startPos.y > cursorScreenPos.y + size.y)
					startPos.y = cursorScreenPos.y + size.y;

				// Zoomed pixel snapping
				if (canvasZoom > 1.0f)
				{
					startPos.x = roundf((startPos.x - cursorScreenPos.x) / canvasZoom) * canvasZoom + cursorScreenPos.x;
					startPos.y = roundf((startPos.y - cursorScreenPos.y) / canvasZoom) * canvasZoom + cursorScreenPos.y;
				}
			}
			else if (mouseStatus_ == MouseStatus::DRAGGING || mouseStatus_ == MouseStatus::RELEASED)
			{
				// Clipping to image
				if (endPos.x < cursorScreenPos.x)
					endPos.x = cursorScreenPos.x;
				if (endPos.y < cursorScreenPos.y)
					endPos.y = cursorScreenPos.y;

				if (endPos.x > cursorScreenPos.x + size.x)
					endPos.x = cursorScreenPos.x + size.x;
				if (endPos.y > cursorScreenPos.y + size.y)
					endPos.y = cursorScreenPos.y + size.y;

				// Zoomed pixel snapping
				if (canvasZoom > 1.0f)
				{
					endPos.x = roundf((endPos.x - cursorScreenPos.x) / canvasZoom) * canvasZoom + cursorScreenPos.x;
					endPos.y = roundf((endPos.y - cursorScreenPos.y) / canvasZoom) * canvasZoom + cursorScreenPos.y;
				}
			}
		}
		else
		{
			if (mouseStatus_ == MouseStatus::DRAGGING || mouseStatus_ == MouseStatus::RELEASED)
			{
				const ImVec2 diff(roundf((endPos.x - startPos.x) / canvasZoom), roundf((endPos.y - startPos.y) / canvasZoom));

				if (moveRect)
				{
					texRect.x += roundf((endPos.x - startPos.x) / canvasZoom); // rewrite
					texRect.y += roundf((endPos.y - startPos.y) / canvasZoom);
				}
				else if (resizeRect)
				{
					if (resizeDir.x > 0.0f)
						texRect.w += diff.x;
					else if (resizeDir.x < 0.0f)
					{
						texRect.x += diff.x;
						texRect.w -= diff.x;
					}

					if (resizeDir.y > 0.0f)
						texRect.h += diff.y;
					else if (resizeDir.y < 0.0f)
					{
						texRect.y += diff.y;
						texRect.h -= diff.y;
					}
				}

				if (texRect.w <= 0)
					texRect.w = 1;
				if (texRect.h <= 0)
					texRect.h = 1;

				// Clipping to image
				if (texRect.x < 0)
					texRect.x = 0;
				if (texRect.y < 0)
					texRect.y = 0;

				if (texRect.x > size.x - texRect.w)
					texRect.x = size.x - texRect.w;
				if (texRect.y > size.y - texRect.h)
					texRect.y = size.y - texRect.h;
			}
		}

		ImVec2 minRect(startPos);
		ImVec2 maxRect(endPos);
		if (mouseStatus_ == MouseStatus::IDLE || mouseStatus_ == MouseStatus::CLICKED || moveRect || resizeRect ||
		    (!(maxRect.x - minRect.x != 0.0f && maxRect.y - minRect.y != 0.0f) && mouseStatus_ != MouseStatus::DRAGGING))
		{
			// Setting the non covered rect from the sprite texrect
			minRect.x = cursorScreenPos.x + (texRect.x * canvasZoom);
			minRect.y = cursorScreenPos.y + (texRect.y * canvasZoom);
			maxRect.x = minRect.x + (texRect.w * canvasZoom);
			maxRect.y = minRect.y + (texRect.h * canvasZoom);
		}
		else
		{
			// Setting the non covered rect from the mouse dragged area
			if (minRect.x > maxRect.x)
				nctl::swap(minRect.x, maxRect.x);
			if (minRect.y > maxRect.y)
				nctl::swap(minRect.y, maxRect.y);
		}

		const ImU32 darkGray = IM_COL32(0, 0, 0, 85);
		ImRect top(ImVec2(cursorScreenPos.x, cursorScreenPos.y), ImVec2(cursorScreenPos.x + size.x, minRect.y));
		ImRect left(ImVec2(cursorScreenPos.x, minRect.y), ImVec2(minRect.x, cursorScreenPos.y + size.y));
		ImRect right(ImVec2(maxRect.x, minRect.y), ImVec2(cursorScreenPos.x + size.x, cursorScreenPos.y + size.y));
		ImRect bottom(ImVec2(minRect.x, maxRect.y), ImVec2(maxRect.x, cursorScreenPos.y + size.y));
		ImDrawList *drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(top.Min, top.Max, darkGray);
		drawList->AddRectFilled(left.Min, left.Max, darkGray);
		drawList->AddRectFilled(right.Min, right.Max, darkGray);
		drawList->AddRectFilled(bottom.Min, bottom.Max, darkGray);

		if (mouseStatus_ == MouseStatus::RELEASED)
		{
			texRect.x = (minRect.x - cursorScreenPos.x) / canvasZoom;
			texRect.y = (minRect.y - cursorScreenPos.y) / canvasZoom;
			texRect.w = (maxRect.x - minRect.x) / canvasZoom;
			texRect.h = (maxRect.y - minRect.y) / canvasZoom;
			ASSERT(texRect.x >= 0);
			ASSERT(texRect.y >= 0);

			if (texRect.w > 0 && texRect.h > 0)
				sprite.setTexRect(texRect);

			moveRect = false;
			resizeRect = false;
			// Back to idle mouse status after assigning the new texrect
			mouseStatus_ = MouseStatus::IDLE;
		}
	}

	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void CanvasWindows::mouseWheelCanvasZoom()
{
	if (ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f)
	{
		const float wheel = ImGui::GetIO().MouseWheel;

		if (wheel > 0.0f)
			ui_.canvasGuiSection_.increaseZoom();
		else if (wheel < 0.0f)
			ui_.canvasGuiSection_.decreaseZoom();
	}
}

void CanvasWindows::SpriteProperties::save(Sprite &sprite)
{
	if (saved_)
		return;
	ASSERT(saved_ == false);

	parent_ = sprite.parent();
	position_.set(sprite.x, sprite.y);
	rotation_ = sprite.rotation;
	scaleFactor_ = sprite.scaleFactor;
	anchorPoint_ = sprite.anchorPoint;
	color_ = sprite.color;

	const nc::Vector2f absPosition = sprite.absPosition();
	sprite.setParent(nullptr);
	sprite.setAbsPosition(absPosition);
	sprite.rotation = 0.0f;
	sprite.scaleFactor.set(1.0f, 1.0f);
	sprite.anchorPoint.set(0.0f, 0.0f);
	sprite.color = nc::Colorf::White;

	saved_ = true;
}

void CanvasWindows::SpriteProperties::restore(Sprite &sprite)
{
	if (saved_ == false)
		return;
	ASSERT(saved_ == true);

	sprite.setParent(parent_);
	sprite.x = position_.x;
	sprite.y = position_.y;
	sprite.rotation = rotation_;
	sprite.scaleFactor = scaleFactor_;
	sprite.anchorPoint = anchorPoint_;
	sprite.color = color_;

	saved_ = false;
}
