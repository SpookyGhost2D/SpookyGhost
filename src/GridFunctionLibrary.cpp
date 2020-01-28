#include "GridFunctionLibrary.h"
#include "GridAnimation.h"
#include "Sprite.h"

nctl::Array<GridFunction> GridFunctionLibrary::gridFunctions_(4);

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

namespace {

void waveX(GridAnimation &gridAnimation)
{
	const float value = gridAnimation.curve().value();
	const nctl::Array<GridFunctionParameter> &parameters = gridAnimation.parameters();

	const float amplitude = parameters[0].value0;
	const float frequency = parameters[1].value0;
	const float py = parameters[2].value0;
	Sprite *sprite = gridAnimation.sprite();

	const int width = sprite->width();
	const int height = sprite->height();
	const int halfHeight = height / 2;
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		const float distPy = halfHeight + py - float(y);
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.x += (distPy / halfHeight) * amplitude * sinf(value * 2.0f * nc::fPi + (frequency * distPy / halfHeight));
		}
	}
}

void waveY(GridAnimation &gridAnimation)
{
	const float value = gridAnimation.curve().value();
	const nctl::Array<GridFunctionParameter> &parameters = gridAnimation.parameters();

	const float amplitude = parameters[0].value0;
	const float frequency = parameters[1].value0;
	const float px = parameters[2].value0;
	Sprite *sprite = gridAnimation.sprite();

	const int width = sprite->width();
	const int height = sprite->height();
	const int halfWidth = width / 2;
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int x = 0; x < width + 1; x++)
	{
		const float distPx = halfWidth + px - float(x);
		for (int y = 0; y < height + 1; y++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.y += (distPx / halfWidth) * amplitude * sinf(value * 2.0f * nc::fPi + (frequency * distPx / halfWidth));
		}
	}
}

void skewX(GridAnimation &gridAnimation)
{
	const float value = gridAnimation.curve().value();
	const nctl::Array<GridFunctionParameter> &parameters = gridAnimation.parameters();

	const float py = parameters[0].value0;
	Sprite *sprite = gridAnimation.sprite();

	const int width = sprite->width();
	const int height = sprite->height();
	const int halfHeight = height / 2;
	const float invWidth = 1.0f / float(width);
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		const float distPy = halfHeight + py - y;
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.x += -distPy * value * invWidth;
		}
	}
}

void skewY(GridAnimation &gridAnimation)
{
	const float value = gridAnimation.curve().value();
	const nctl::Array<GridFunctionParameter> &parameters = gridAnimation.parameters();

	const float px = parameters[0].value0;
	Sprite *sprite = gridAnimation.sprite();

	const int width = sprite->width();
	const int height = sprite->height();
	const int halfWidth = width / 2;
	const float invHeight = 1.0f / float(height);
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int x = 0; x < width + 1; x++)
	{
		const float distPx = halfWidth + px - x;
		for (int y = 0; y < height + 1; y++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.y += -distPx * value * invHeight;
		}
	}
}

void zoom(GridAnimation &gridAnimation)
{
	const float value = gridAnimation.curve().value();
	const nctl::Array<GridFunctionParameter> &parameters = gridAnimation.parameters();

	const float px = parameters[0].value0;
	const float py = parameters[1].value0;
	Sprite *sprite = gridAnimation.sprite();

	const int width = sprite->width();
	const int height = sprite->height();
	const int halfWidth = width / 2;
	const int halfHeight = height / 2;
	const float invWidth = 1.0f / float(width);
	const float invHeight = 1.0f / float(height);
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			const float distPx = halfWidth + px - x;
			const float distPy = halfHeight + py - y;
			v.x += -distPx * value * invWidth;
			v.y += -distPy * value * invHeight;
		}
	}
}

}

void GridFunctionLibrary::init()
{
	{
		GridFunction waveXFunction;
		waveXFunction.setName("Wave X");
		GridFunction::ParameterInfo &amplitude = waveXFunction.addParameter("Amplitude", GridFunction::ParameterType::FLOAT, 0.25f);
		amplitude.minValue.value0 = 0.0f;
		amplitude.maxValue.value0 = 1.0f;
		GridFunction::ParameterInfo &frequency = waveXFunction.addParameter("Frequency", GridFunction::ParameterType::FLOAT, 1.0f);
		frequency.minValue.value0 = 0.0f;
		frequency.maxValue.value0 = 8.0f;
		GridFunction::ParameterInfo &anchor = waveXFunction.addParameter("Anchor Y", GridFunction::ParameterType::FLOAT, 0.0f);
		anchor.minValue.value0 = -0.5f;
		anchor.maxValue.value0 = 0.5f;
		anchor.minMultiply = GridFunction::ValueMultiply::SPRITE_HEIGHT;
		anchor.maxMultiply = GridFunction::ValueMultiply::SPRITE_HEIGHT;
		anchor.anchorType = GridFunction::AnchorType::Y;
		waveXFunction.setCallback(waveX);
		gridFunctions_.pushBack(waveXFunction);
	}

	{
		GridFunction waveYFunction;
		waveYFunction.setName("Wave Y");
		GridFunction::ParameterInfo &amplitude = waveYFunction.addParameter("Amplitude", GridFunction::ParameterType::FLOAT, 0.25f);
		amplitude.minValue.value0 = 0.0f;
		amplitude.maxValue.value0 = 1.0f;
		GridFunction::ParameterInfo &frequency = waveYFunction.addParameter("Frequency", GridFunction::ParameterType::FLOAT, 1.0f);
		frequency.minValue.value0 = 0.0f;
		frequency.maxValue.value0 = 8.0f;
		GridFunction::ParameterInfo &anchor = waveYFunction.addParameter("Anchor X", GridFunction::ParameterType::FLOAT, 0.0f, 0.0f);
		anchor.minValue.value0 = -0.5f;
		anchor.maxValue.value0 = 0.5f;
		anchor.minMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchor.maxMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchor.anchorType = GridFunction::AnchorType::X;
		waveYFunction.setCallback(waveY);
		gridFunctions_.pushBack(waveYFunction);
	}

	{
		GridFunction skewXFunction;
		skewXFunction.setName("Skew X");
		GridFunction::ParameterInfo &anchor = skewXFunction.addParameter("Anchor Y", GridFunction::ParameterType::FLOAT, 0.0f);
		anchor.minValue.value0 = -0.5f;
		anchor.maxValue.value0 = 0.5f;
		anchor.minMultiply = GridFunction::ValueMultiply::SPRITE_HEIGHT;
		anchor.maxMultiply = GridFunction::ValueMultiply::SPRITE_HEIGHT;
		anchor.anchorType = GridFunction::AnchorType::Y;
		skewXFunction.setCallback(skewX);
		gridFunctions_.pushBack(skewXFunction);
	}

	{
		GridFunction skewYFunction;
		skewYFunction.setName("Skew Y");
		GridFunction::ParameterInfo &anchor = skewYFunction.addParameter("Anchor X", GridFunction::ParameterType::FLOAT, 0.0f, 0.0f);
		anchor.minValue.value0 = -0.5f;
		anchor.maxValue.value0 = 0.5f;
		anchor.minMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchor.maxMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchor.anchorType = GridFunction::AnchorType::X;
		skewYFunction.setCallback(skewY);
		gridFunctions_.pushBack(skewYFunction);
	}

	{
		GridFunction zoomFunction;
		zoomFunction.setName("Zoom");
		GridFunction::ParameterInfo &anchorx = zoomFunction.addParameter("Anchor X", GridFunction::ParameterType::FLOAT, 0.0f, 0.0f);
		anchorx.minValue.value0 = -0.5f;
		anchorx.maxValue.value0 = 0.5f;
		anchorx.minMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchorx.maxMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchorx.anchorType = GridFunction::AnchorType::X;
		GridFunction::ParameterInfo &anchory = zoomFunction.addParameter("Anchor Y", GridFunction::ParameterType::FLOAT, 0.0f, 0.0f);
		anchory.minValue.value0 = -0.5f;
		anchory.maxValue.value0 = 0.5f;
		anchory.minMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchory.maxMultiply = GridFunction::ValueMultiply::SPRITE_WIDTH;
		anchory.anchorType = GridFunction::AnchorType::Y;
		zoomFunction.setCallback(zoom);
		gridFunctions_.pushBack(zoomFunction);
	}
}
