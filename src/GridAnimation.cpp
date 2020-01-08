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
    : curve_(type, loopMode), speed_(1.0f), type_(AnimationType::WOBBLE), sprite_(nullptr)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void GridAnimation::stop()
{
	curve_.reset();
	state_ = State::STOPPED;
}

void GridAnimation::play()
{
	if (state_ == State::STOPPED)
		curve_.reset();
	state_ = State::PLAYING;
}

void GridAnimation::update(float deltaTime)
{
	switch (state_)
	{
		case State::STOPPED:
		case State::PAUSED:
			deform(curve_.value());
			break;
		case State::PLAYING:
			const float value = curve_.next(speed_ * deltaTime);
			deform(value);
			break;
	}

	if (curve_.time() >= 1.0f && state_ == State::PLAYING &&
	    curve_.loopMode() == EasingCurve::LoopMode::DISABLED)
		state_ = State::STOPPED;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void GridAnimation::deform(float value)
{
	if (sprite_ == nullptr)
		return;

	const int width = sprite_->width();
	const int height = sprite_->height();

	const nctl::Array<Sprite::VertexPosition> &restVertices = sprite_->vertexRestPositions();
	nctl::Array<Sprite::Vertex> &interleavedVertices = sprite_->interleavedVertices();

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			const Sprite::VertexPosition &r = restVertices[index];
			Sprite::Vertex &v = interleavedVertices[index];
			v.y = r.y + 0.15f * sinf(value * (x / static_cast<float>(width + 1))) * (x / static_cast<float>(width + 1));
		}
	}
}
