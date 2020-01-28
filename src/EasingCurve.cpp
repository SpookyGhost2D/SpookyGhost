#include <ncine/common_constants.h>
#include "EasingCurve.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

EasingCurve::EasingCurve(Type type, LoopMode loopMode)
    : type_(type), direction_(Direction::FORWARD), loopMode_(loopMode), forward_(true),
      time_(0.0f), start_(0.0f), end_(1.0f), scale_(1.0f), shift_(0.0f)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void EasingCurve::setTime(float time)
{
	time_ = time;

	if (time_ < start_)
		time_ = start_;
	else if (time_ > end_)
		time_ = end_;
}

void EasingCurve::reset()
{
	if (direction_ == Direction::FORWARD)
		time_ = start_;
	else if (direction_ == Direction::BACKWARD)
		time_ = end_;
}

float EasingCurve::value()
{
	switch (type_)
	{
		case Type::LINEAR:
			return time_ * scale_ + shift_;
		case Type::QUAD:
			return time_ * time_ * scale_ + shift_;
		case Type::CUBIC:
			return time_ * time_ * time_ * scale_ + shift_;
		case Type::QUART:
			return time_ * time_ * time_ * time_ * scale_ + shift_;
		case Type::QUINT:
			return time_ * time_ * time_ * time_ * time_ * scale_ + shift_;
		case Type::SINE:
			return sinf(time_ * ncine::fPi) * scale_ + shift_;
		case Type::EXPO:
			return (1.0f - powf(2, time_)) * scale_ + shift_;
		case Type::CIRC:
			return sqrtf(1.0f - time_ * time_) * scale_ + shift_;
	}

	return 1.0f;
}

void EasingCurve::next(float deltaTime)
{
	if ((forward_ && direction_ == Direction::FORWARD) ||
	    (!forward_ && direction_ == Direction::BACKWARD))
		time_ += deltaTime;
	else
		time_ -= deltaTime;

	if (time_ < start_)
	{
		if (direction_ == Direction::BACKWARD)
		{
			if (loopMode_ == LoopMode::DISABLED)
				time_ = start_;
			else if (loopMode_ == LoopMode::REWIND)
				time_ = end_ + time_;
			else if (loopMode_ == LoopMode::PING_PONG)
			{
				time_ = 2.0f * start_ - time_;
				forward_ = false;
			}
		}
		else
		{
			time_ = 2.0f * start_ - time_;
			forward_ = true;
		}
	}
	else if (time_ > end_)
	{
		if (direction_ == Direction::FORWARD)
		{
			if (loopMode_ == LoopMode::DISABLED)
				time_ = end_;
			else if (loopMode_ == LoopMode::REWIND)
				time_ = time_ - end_;
			else if (loopMode_ == LoopMode::PING_PONG)
			{
				time_ = 2.0f * end_ - time_;
				forward_ = false;
			}
		}
		else
		{
			time_ = 2.0f * end_ - time_;
			forward_ = true;
		}
	}
}
