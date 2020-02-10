#include "GridFunction.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

GridFunction::GridFunction()
    : name_(MaxNameLength), parametersInfo_(4), callback_(nullptr)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

const GridFunction::ParameterInfo &GridFunction::parameterInfo(unsigned int index) const
{
	FATAL_ASSERT(index < parametersInfo_.size());
	return parametersInfo_[index];
}

const char *GridFunction::parameterName(unsigned int index) const
{
	FATAL_ASSERT(index < parametersInfo_.size());
	return parametersInfo_[index].name.data();
}

GridFunction::ParameterType GridFunction::parameterType(unsigned int index) const
{
	FATAL_ASSERT(index < parametersInfo_.size());
	return parametersInfo_[index].type;
}

GridFunctionParameter &GridFunction::parameterInitialValue(unsigned int index)
{
	FATAL_ASSERT(index < parametersInfo_.size());
	return parametersInfo_[index].initialValue;
}

GridFunction::ParameterInfo &GridFunction::addParameter(const char *name, ParameterType type, float initialValue0)
{
	parametersInfo_.pushBack(ParameterInfo(name, type, initialValue0));
	return parametersInfo_.back();
}

GridFunction::ParameterInfo &GridFunction::addParameter(const char *name, ParameterType type, float initialValue0, float initialValue1)
{
	parametersInfo_.pushBack(ParameterInfo(name, type, initialValue0, initialValue1));
	return parametersInfo_.back();
}

void GridFunction::execute(GridAnimation &animation) const
{
	if (callback_ != nullptr)
		callback_(animation);
}
