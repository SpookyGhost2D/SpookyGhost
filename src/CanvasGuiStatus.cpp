#include <ncine/imgui.h>
#include "CanvasGuiStatus.h"

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

float CanvasGuiStatus::zoomAmount(ZoomLevel zl)
{
	switch (zl)
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

float CanvasGuiStatus::zoomAmount() const
{
	return zoomAmount(zoomLevel);
}

void CanvasGuiStatus::setZoom(float zoomAmount)
{
	if (zoomAmount <= 0.125f)
		zoomLevel = ZoomLevel::X1_8;
	else if (zoomAmount <= 0.25f)
		zoomLevel = ZoomLevel::X1_4;
	else if (zoomAmount <= 0.5f)
		zoomLevel = ZoomLevel::X1_2;
	else if (zoomAmount <= 1.0f)
		zoomLevel = ZoomLevel::X1;
	else if (zoomAmount <= 2.0f)
		zoomLevel = ZoomLevel::X2;
	else if (zoomAmount <= 4.0f)
		zoomLevel = ZoomLevel::X4;
	else
		zoomLevel = ZoomLevel::X8;
}

void CanvasGuiStatus::increaseZoom()
{
	const unsigned int zoomIndex = static_cast<unsigned int>(zoomLevel);
	if (zoomIndex < ZoomLevel::X8)
		zoomLevel = static_cast<ZoomLevel>(zoomIndex + 1);
}

void CanvasGuiStatus::decreaseZoom()
{
	const unsigned int zoomIndex = static_cast<unsigned int>(zoomLevel);
	if (zoomIndex > 0)
		zoomLevel = static_cast<ZoomLevel>(zoomIndex - 1);
}

void CanvasGuiStatus::setResize(const nc::Vector2i &size)
{
	setResize(size.x, size.y);
}

void CanvasGuiStatus::setResize(int width, int height)
{
	customCanvasSize.set(width, height);
	if (width == 16 && height == 16)
		resizePreset = ResizePreset::SIZE16;
	else if (width == 32 && height == 32)
		resizePreset = ResizePreset::SIZE32;
	else if (width == 64 && height == 64)
		resizePreset = ResizePreset::SIZE64;
	else if (width == 128 && height == 128)
		resizePreset = ResizePreset::SIZE128;
	else if (width == 256 && height == 256)
		resizePreset = ResizePreset::SIZE256;
	else if (width == 512 && height == 512)
		resizePreset = ResizePreset::SIZE512;
	else
		resizePreset = ResizePreset::CUSTOM;
}

nc::Vector2i CanvasGuiStatus::resizeVector()
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
			return customCanvasSize;
	}

	return customCanvasSize;
}
