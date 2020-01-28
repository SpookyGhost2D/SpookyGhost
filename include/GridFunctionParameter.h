#ifndef CLASS_GRIDFUNCTIONPARAMETER
#define CLASS_GRIDFUNCTIONPARAMETER

/// The actual parameter value for a grid function
struct GridFunctionParameter
{
	GridFunctionParameter()
	    : value0(0.0f), value1(0.0) {}
	GridFunctionParameter(float v0, float v1)
	    : value0(v0), value1(v1) {}

	inline void set(float v0, float v1)
	{
		value0 = v0;
		value1 = v1;
	}

	float value0;
	float value1;
};

#endif
