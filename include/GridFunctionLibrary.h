#ifndef CLASS_GRIDFUNCTIONLIBRARY
#define CLASS_GRIDFUNCTIONLIBRARY

#include "GridFunction.h"

namespace nc = ncine;

class GridFunctionLibrary
{
  public:
	static void init();
	static const nctl::Array<GridFunction> &gridFunctions() { return gridFunctions_; }

  private:
	static nctl::Array<GridFunction> gridFunctions_;
};

#endif
