#include <ncine/imgui.h>
#include "gui/gui_labels.h"
#include "gui/gui_common.h"
#include "gui/CanvasGuiSection.h"
#include "Canvas.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void CanvasGuiSection::create(Canvas &canvas)
{
	ui::auxString.format("Zoom: %.2f", zoomAmount());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	if (ImGui::Button(Labels::PlusIcon))
		increaseZoom();
	ImGui::SameLine();
	if (ImGui::Button(Labels::MinusIcon))
		decreaseZoom();
	ImGui::PopStyleVar();
	ImGui::SameLine();
	if (ImGui::Button(ui::auxString.data()))
		resetZoom();

	ImGui::SameLine();
	ImGui::ColorEdit4("Background", canvas.backgroundColor.data(), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
	ImGui::SameLine();

	currentComboResize_ = static_cast<int>(resizePreset_);
	ui::comboString.clear();
	for (unsigned int i = 0; i < IM_ARRAYSIZE(ResizeStrings); i++)
	{
		ui::comboString.formatAppend("%s", ResizeStrings[i]);
		if (ResizePreset(i) != ResizePreset::CUSTOM &&
		    canvas.size().x == resizeVector(ResizePreset(i)).x &&
		    canvas.size().y == resizeVector(ResizePreset(i)).y)
		{
			ui::comboString.formatAppend(" %s", Labels::SelectedIcon);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
	}
	ui::comboString.setLength(ui::comboString.length() + 1);
	// Append a second '\0' to signal the end of the combo item list
	ui::comboString[ui::comboString.length() - 1] = '\0';

	ui::auxString.clear();
	if (currentComboResize_ != static_cast<int>(ResizePreset::CUSTOM))
		ui::auxString.append("Resize");
	ui::auxString.append("##ResizeCombo");

	ImGui::PushItemWidth(100.0f);
	ImGui::Combo(ui::auxString.data(), &currentComboResize_, ui::comboString.data());
	ImGui::PopItemWidth();
	resizePreset_ = static_cast<CanvasGuiSection::ResizePreset>(currentComboResize_);
	if (currentComboResize_ == CanvasGuiSection::ResizePreset::CUSTOM)
	{
		ImGui::SameLine();
		ImGui::PushItemWidth(80.0f);
		ImGui::InputInt2("Resize", customCanvasSize_.data(), ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();
	}
	else
		customCanvasSize_ = canvas.size();

	nc::Vector2i desiredCanvasSize = resizeVector();

	if (desiredCanvasSize.x < 4)
		desiredCanvasSize.x = 4;
	else if (desiredCanvasSize.x > canvas.maxTextureSize())
		desiredCanvasSize.x = canvas.maxTextureSize();
	if (desiredCanvasSize.y < 4)
		desiredCanvasSize.y = 4;
	else if (desiredCanvasSize.y > canvas.maxTextureSize())
		desiredCanvasSize.y = canvas.maxTextureSize();

	if (canvas.size().x != desiredCanvasSize.x || canvas.size().y != desiredCanvasSize.y)
		canvas.resizeTexture(desiredCanvasSize);

	ImGui::SameLine();
	ImGui::Checkbox("Borders", &showBorders_);

	ImGui::Separator();
}

void CanvasGuiSection::setResize(const nc::Vector2i &size)
{
	setResize(size.x, size.y);
}

void CanvasGuiSection::setResize(int width, int height)
{
	customCanvasSize_.set(width, height);
	if (width == 16 && height == 16)
		resizePreset_ = ResizePreset::SIZE16;
	else if (width == 32 && height == 32)
		resizePreset_ = ResizePreset::SIZE32;
	else if (width == 64 && height == 64)
		resizePreset_ = ResizePreset::SIZE64;
	else if (width == 128 && height == 128)
		resizePreset_ = ResizePreset::SIZE128;
	else if (width == 256 && height == 256)
		resizePreset_ = ResizePreset::SIZE256;
	else if (width == 512 && height == 512)
		resizePreset_ = ResizePreset::SIZE512;
	else
		resizePreset_ = ResizePreset::CUSTOM;
}

float CanvasGuiSection::zoomAmount() const
{
	switch (zoomLevel_)
	{
		case ZoomLevel::X1_8:
			return 0.125f;
		case ZoomLevel::X1_4:
			return 0.25f;
		case ZoomLevel::X1_2:
			return 0.5f;
		case ZoomLevel::X1:
			return 1.0f;
		case ZoomLevel::X2:
			return 2.0f;
		case ZoomLevel::X4:
			return 4.0f;
		case ZoomLevel::X8:
			return 8.0f;
	}
	return 1.0f;
}

void CanvasGuiSection::resetZoom()
{
	zoomLevel_ = ZoomLevel::X1;
}

void CanvasGuiSection::increaseZoom()
{
	const unsigned int zoomIndex = static_cast<unsigned int>(zoomLevel_);
	if (zoomIndex < ZoomLevel::X8)
		zoomLevel_ = static_cast<ZoomLevel>(zoomIndex + 1);
}

void CanvasGuiSection::decreaseZoom()
{
	const unsigned int zoomIndex = static_cast<unsigned int>(zoomLevel_);
	if (zoomIndex > 0)
		zoomLevel_ = static_cast<ZoomLevel>(zoomIndex - 1);
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

nc::Vector2i CanvasGuiSection::resizeVector(ResizePreset resizePreset)
{
	switch (resizePreset)
	{
		case ResizePreset::SIZE16:
			return nc::Vector2i(16, 16);
		case ResizePreset::SIZE32:
			return nc::Vector2i(32, 32);
		case ResizePreset::SIZE64:
			return nc::Vector2i(64, 64);
		case ResizePreset::SIZE128:
			return nc::Vector2i(128, 128);
		case ResizePreset::SIZE256:
			return nc::Vector2i(256, 256);
		case ResizePreset::SIZE512:
			return nc::Vector2i(512, 512);
		case ResizePreset::CUSTOM:
			return customCanvasSize_;
	}

	return customCanvasSize_;
}

nc::Vector2i CanvasGuiSection::resizeVector()
{
	return resizeVector(resizePreset_);
}
