#ifndef CLASS_GRIDFUNCTION
#define CLASS_GRIDFUNCTION

#include <nctl/Array.h>
#include <nctl/String.h>
#include "GridFunctionParameter.h"

class GridAnimation;

namespace nc = ncine;

/// The class that applies function to the grid
class GridFunction
{
  public:
	static const unsigned int MaxNameLength = 64;
	using CallbackType = void (*)(GridAnimation &gridAnimation);

	enum class ParameterType
	{
		FLOAT,
		VECTOR2F
	};

	enum class ValueMultiply
	{
		IDENTITY,
		SPRITE_WIDTH,
		SPRITE_HEIGHT
	};

	enum class AnchorType
	{
		NONE,
		X,
		Y,
		XY
	};

	struct ParameterInfo
	{
		ParameterInfo()
		    : name(MaxNameLength), type(ParameterType::FLOAT) {}
		ParameterInfo(const char *paramName, ParameterType paramType)
		    : name(paramName), type(paramType), initialValue(0.0f, 0.0f) {}
		ParameterInfo(const char *paramName, ParameterType paramType, float initialValue0)
		    : name(paramName), type(paramType), initialValue(initialValue0, initialValue0) {}
		ParameterInfo(const char *paramName, ParameterType paramType, float initialValue0, float initialValue1)
		    : name(paramName), type(paramType), initialValue(initialValue0, initialValue1) {}
		ParameterInfo(const ParameterInfo &) = default;
		ParameterInfo &operator=(const ParameterInfo &) = default;

		nctl::String name;
		ParameterType type;
		GridFunctionParameter initialValue;
		GridFunctionParameter minValue;
		GridFunctionParameter maxValue;
		ValueMultiply initialMultiply = ValueMultiply::IDENTITY;
		ValueMultiply minMultiply = ValueMultiply::IDENTITY;
		ValueMultiply maxMultiply = ValueMultiply::IDENTITY;
		AnchorType anchorType = AnchorType::NONE;
	};

	GridFunction();

	inline const nctl::String &name() const { return name_; };
	inline void setName(const char *name) { name_ = name; }

	inline unsigned int numParameters() const { return parametersInfo_.size(); };

	const ParameterInfo &parameterInfo(unsigned int index) const;
	const char *parameterName(unsigned int index) const;
	ParameterType parameterType(unsigned int index) const;
	GridFunctionParameter &parameterInitialValue(unsigned int index);

	ParameterInfo &addParameter(const char *name, ParameterType type, float initialValue0);
	ParameterInfo &addParameter(const char *name, ParameterType type, float initialValue0, float initialvalue1);

	inline void setCallback(CallbackType callback) { callback_ = callback; }

	void execute(GridAnimation &animation) const;

  private:
	nctl::String name_;
	nctl::Array<ParameterInfo> parametersInfo_;
	CallbackType callback_;
};

#endif
