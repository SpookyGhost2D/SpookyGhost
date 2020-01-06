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

	enum class LoopMode
	{
		DISABLED,
		REWIND,
		PING_PONG
	};

	EasingCurve(Type type, LoopMode loopMode);

	inline Type type() const { return type_; }
	inline void setType(Type type) { type_ = type; }

	inline LoopMode loopMode() const { return loopMode_; }
	inline void setLoopMode(LoopMode loopMode) { loopMode_ = loopMode; }

	inline bool isGoingForward() const { return forward_; }
	inline void goForward(bool forward) { forward_ = forward; }

	inline float time() const { return time_; }
	void setTime(float time);

	inline float scale() const { return scale_; }
	inline float &scale() { return scale_; }
	void setScale(float scale) { scale_ = scale; }

	inline float shift() const { return shift_; }
	inline float &shift() { return shift_; }
	void setShift(float shift) { shift_ = shift; }

	void reset();
	float value();
	float next(float deltaTime);

  private:
	Type type_;
	LoopMode loopMode_;
	bool forward_;

	float time_;
	float scale_;
	float shift_;
};

#endif
