#include <ncine/imgui.h>
#include "gui/gui_labels.h"
#include "gui/CanvasGuiSection.h"
#include "Canvas.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void CanvasGuiSection::create(Canvas &canvas)
{
	if (ImGui::CollapsingHeader(Labels::Canvas))
	{
		canvasZoomRadio_ = static_cast<int>(zoomLevel_);

		ImGui::PushID("Canvas");
		ImGui::Text("Zoom:");
		ImGui::SameLine();
		ImGui::RadioButton("1/8x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X1_8);
		ImGui::SameLine();
		ImGui::RadioButton("1/4x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X1_4);
		ImGui::SameLine();
		ImGui::RadioButton("1/2x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X1_2);
		ImGui::SameLine();
		ImGui::RadioButton("1x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X1);
		ImGui::SameLine();
		ImGui::RadioButton("2x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X2);
		ImGui::SameLine();
		ImGui::RadioButton("4x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X4);
		ImGui::SameLine();
		ImGui::RadioButton("8x", &canvasZoomRadio_, CanvasGuiSection::ZoomLevel::X8);
		ImGui::PopID();
		zoomLevel_ = static_cast<CanvasGuiSection::ZoomLevel>(canvasZoomRadio_);

		static int currentComboResize = 0;
		currentComboResize = static_cast<int>(resizePreset_);
		ImGui::Combo("Presets", &currentComboResize, ResizeStrings, IM_ARRAYSIZE(ResizeStrings));
		resizePreset_ = static_cast<CanvasGuiSection::ResizePreset>(currentComboResize);
		if (currentComboResize == CanvasGuiSection::ResizePreset::CUSTOM)
			ImGui::InputInt2("Custom Size", customCanvasSize_.data());
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

		ImGui::SameLine();
		if (ImGui::Button(Labels::Apply) &&
			(canvas.size().x != desiredCanvasSize.x || canvas.size().y != desiredCanvasSize.y))
		{
			canvas.resizeTexture(desiredCanvasSize);
		}

		ImGui::Text("Size: %d x %d", canvas.texWidth(), canvas.texHeight());
		ImGui::ColorEdit4("Background", canvas.backgroundColor.data(), ImGuiColorEditFlags_AlphaBar);
	}
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
	//return zoomAmount(zoomLevel);
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

void CanvasGuiSection::setZoom(float zoomAmount)
{
	if (zoomAmount <= 0.125f)
		zoomLevel_ = ZoomLevel::X1_8;
	else if (zoomAmount <= 0.25f)
		zoomLevel_ = ZoomLevel::X1_4;
	else if (zoomAmount <= 0.5f)
		zoomLevel_ = ZoomLevel::X1_2;
	else if (zoomAmount <= 1.0f)
		zoomLevel_ = ZoomLevel::X1;
	else if (zoomAmount <= 2.0f)
		zoomLevel_ = ZoomLevel::X2;
	else if (zoomAmount <= 4.0f)
		zoomLevel_ = ZoomLevel::X4;
	else
		zoomLevel_ = ZoomLevel::X8;
}

nc::Vector2i CanvasGuiSection::resizeVector()
{
	switch (resizePreset_)
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
