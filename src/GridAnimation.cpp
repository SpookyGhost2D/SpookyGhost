#include "GridAnimation.h"
#include "Sprite.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

GridAnimation::GridAnimation()
    : GridAnimation(EasingCurve::Type::LINEAR, EasingCurve::LoopMode::DISABLED)
{
}

GridAnimation::GridAnimation(EasingCurve::Type type, EasingCurve::LoopMode loopMode)
    : CurveAnimation(type, loopMode), type_(AnimationType::WOBBLE_X), sprite_(nullptr)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void GridAnimation::stop()
{
	CurveAnimation::stop();
	deform(curve_.value());
}

void GridAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			if (sprite_)
				deform(curve_.value());
			break;
		case State::PLAYING:
			const float value = curve_.next(speed_ * deltaTime);
			if (sprite_)
				deform(value);
			break;
	}

	CurveAnimation::update(deltaTime);
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

namespace {

void wobbleX(Sprite *sprite, float value)
{
	const int width = sprite->width();
	const int height = sprite->height();
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.x += 0.15f * sinf(value * (y / static_cast<float>(height + 1))) * (y / static_cast<float>(height + 1));
		}
	}
}

void wobbleY(Sprite *sprite, float value)
{
	const int width = sprite->width();
	const int height = sprite->height();
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Sprite::Vertex &v = interleavedVertices[index];
			v.y += 0.15f * sinf(value * (x / static_cast<float>(width + 1))) * (x / static_cast<float>(width + 1));
		}
	}
}

void zoom(Sprite *sprite, float value)
{
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
			const int distCenterX = halfWidth - x;
			const int distCenterY = halfHeight - y;
			v.x += -distCenterX * value * invWidth;
			v.y += -distCenterY * value * invHeight;
		}
	}
}

}

void GridAnimation::deform(float value)
{
	FATAL_ASSERT(sprite_ != nullptr);

	switch (type_)
	{
		case AnimationType::WOBBLE_X:
			wobbleX(sprite_, value);
			break;
		case AnimationType::WOBBLE_Y:
			wobbleY(sprite_, value);
			break;
		case AnimationType::ZOOM:
			zoom(sprite_, value);
			break;
	}
}
