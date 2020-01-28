#ifndef CLASS_EASINGCURVE
#define CLASS_EASINGCURVE

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

	enum class Direction
	{
		FORWARD,
		BACKWARD
	};

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	EasingCurve(Type type, LoopMode loopMode);

	inline Type type() const { return type_; }
	inline void setType(Type type) { type_ = type; }

	inline Direction direction() const { return direction_; }
	inline void setDirection(Direction direction) { direction_ = direction; }

	inline LoopMode loopMode() const { return loopMode_; }
	inline void setLoopMode(LoopMode loopMode) { loopMode_ = loopMode; }

	inline bool isGoingForward() const { return forward_; }
	inline void goForward(bool forward) { forward_ = forward; }

	inline float time() const { return time_; }
	inline float &time() { return time_; }
	void setTime(float time);

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
	Direction direction_;
	LoopMode loopMode_;
	bool forward_;

	float time_;
	float start_;
	float end_;

	float scale_;
	float shift_;
};

#endif
