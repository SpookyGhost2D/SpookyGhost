#ifndef CLASS_EASINGCURVE
#define CLASS_EASINGCURVE

/// The easing curve class
class EasingCurve
{
  public:
	enum class Type
	{
		/// t*c0
		LINEAR,
		/// t*t*c0 + t*c1 + c2
		QUAD
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

	inline float coeff0() const { return c0_; }
	inline float coeff1() const { return c1_; }
	inline float coeff2() const { return c2_; }

	inline float &coeff0() { return c0_; }
	inline float &coeff1() { return c1_; }
	inline float &coeff2() { return c2_; }

	void setCoefficients(float c0);
	void setCoefficients(float c0, float c1);
	void setCoefficients(float c0, float c1, float c2);

	void reset();
	float value();
	float next(float deltaTime);

  private:
	Type type_;
	LoopMode loopMode_;
	bool forward_;

	float c0_;
	float c1_;
	float c2_;

	float time_;

	void initCoefficients();
};

#endif
