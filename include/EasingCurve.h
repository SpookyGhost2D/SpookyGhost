#ifndef CLASS_EASINGCURVE
#define CLASS_EASINGCURVE

#include "LoopComponent.h"

/// The easing curve class
class EasingCurve
{
  public:
	enum class Type
	{
		LINEAR,
		QUAD,
		CUBIC,
		QUART,
		QUINT,
		SINE,
		EXPO,
		CIRC,
	};

	EasingCurve(Type type, Loop::Mode loopMode);

	inline Type type() const { return type_; }
	inline void setType(Type type) { type_ = type; }

	inline const LoopComponent &loop() const { return loop_; }
	inline LoopComponent &loop() { return loop_; }

	inline bool hasInitialValue() const { return withInitialValue_; }
	inline bool &hasInitialValue() { return withInitialValue_; }
	inline void enableInitialValue(bool withInitialValue) { withInitialValue_ = withInitialValue; }

	inline float time() const { return time_; }
	inline float &time() { return time_; }
	void setTime(float time);

	inline float initialValue() const { return initialValue_; }
	inline float &initialValue() { return initialValue_; }
	void setInitialValue(float initialValue);

	inline float start() const { return start_; }
	inline float &start() { return start_; }
	inline void setStart(float start) { start_ = start; }

	inline float end() const { return end_; }
	inline float &end() { return end_; }
	inline void setEnd(float end) { end_ = end; }

	inline float scale() const { return scale_; }
	inline float &scale() { return scale_; }
	void setScale(float scale) { scale_ = scale; }

	inline float shift() const { return shift_; }
	inline float &shift() { return shift_; }
	void setShift(float shift) { shift_ = shift; }

	void reset();
	float value();
	void next(float deltaTime);

  private:
	Type type_;
	LoopComponent loop_;
	bool withInitialValue_;

	float time_;
	float initialValue_;
	float start_;
	float end_;

	float scale_;
	float shift_;
};

#endif
