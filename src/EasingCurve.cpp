#include "EasingCurve.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

EasingCurve::EasingCurve(Type type, LoopMode loopMode)
    : type_(type), loopMode_(loopMode), forward_(true),
      c0_(0.0f), c1_(0.0f), c2_(0.0f), time_(0.0f)
{
	initCoefficients();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void EasingCurve::setTime(float time)
{
	time_ = time;

	if (time_ < 0.0f)
		time_ = 0.0f;
	else if (time_ > 1.0f)
		time_ = 1.0f;
}

void EasingCurve::setCoefficients(float c0)
{
	c0_ = c0;
}

void EasingCurve::setCoefficients(float c0, float c1)
{
	c0_ = c0;
	c1_ = c1;
}

void EasingCurve::setCoefficients(float c0, float c1, float c2)
{
	c0_ = c0;
	c1_ = c1;
	c2_ = c2;
}

void EasingCurve::reset()
{
	time_ = 0.0f;
}

float EasingCurve::value()
{
	switch (type_)
	{
		case Type::LINEAR:
			return time_ * c0_ + c1_;
		case Type::QUAD:
			return time_ * time_ * c0_ + time_ * c1_ + c2_;
	}
}

float EasingCurve::next(float deltaTime)
{
	if (forward_)
		time_ += deltaTime;
	else
		time_ -= deltaTime;

	if (time_ < 0.0f)
	{
		time_ = 0.0f;
		forward_ = true;
	}
	else if (time_ > 1.0f)
	{
		if (loopMode_ == LoopMode::DISABLED)
			time_ = 1.0f;
		else if (loopMode_ == LoopMode::REWIND)
			time_ = 0.0f;
		else if (loopMode_ == LoopMode::PING_PONG)
		{
			time_ = 1.0f;
			forward_ = false;
		}
	}

	return value();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void EasingCurve::initCoefficients()
{
	switch (type_)
	{
		case Type::LINEAR:
			c0_ = 1.0f;
			c1_ = 0.0f;
			break;
		case Type::QUAD:
			c0_ = 1.0f;
			c1_ = 0.0f;
			c2_ = 0.0f;
			break;
	}
}
