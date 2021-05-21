#ifndef CLASS_SCRIPTMANAGER
#define CLASS_SCRIPTMANAGER

#include <nctl/Array.h>

struct lua_State;

class Sprite;
class Script;

namespace nc = ncine;

/// The class that handles Lua scripts
class ScriptManager
{
  public:
	ScriptManager() {}

	inline nctl::Array<nctl::UniquePtr<Script>> &scripts() { return scripts_; }
	inline const nctl::Array<nctl::UniquePtr<Script>> &scripts() const { return scripts_; }

	int scriptIndex(const Script *script) const;

	static void pushSprite(lua_State *L, Sprite *sprite);

  private:
	nctl::Array<nctl::UniquePtr<Script>> scripts_;

	static Sprite *retrieveSprite(lua_State *L);

	static void exposeConstants(lua_State *L);
	static void exposeFunctions(lua_State *L);

	static int canvasWidth(lua_State *L);
	static int canvasHeight(lua_State *L);
	static int textureWidth(lua_State *L);
	static int textureHeight(lua_State *L);
	static int width(lua_State *L);
	static int height(lua_State *L);

	static int position(lua_State *L);
	static int positionX(lua_State *L);
	static int positionY(lua_State *L);
	static int rotation(lua_State *L);
	static int scale(lua_State *L);
	static int scaleX(lua_State *L);
	static int scaleY(lua_State *L);
	static int anchorPoint(lua_State *L);
	static int anchorPointX(lua_State *L);
	static int anchorPointY(lua_State *L);
	static int color(lua_State *L);
	static int texRect(lua_State *L);
	static int isFlippedX(lua_State *L);
	static int isFlippedY(lua_State *L);
	static int rgbBlendingPreset(lua_State *L);
	static int alphaBlendingPreset(lua_State *L);
	static int numVertices(lua_State *L);

	static int vertices(lua_State *L);
	static int verticesXY(lua_State *L);
	static int verticesUV(lua_State *L);
	static int verticesX(lua_State *L);
	static int verticesY(lua_State *L);
	static int verticesU(lua_State *L);
	static int verticesV(lua_State *L);

	static int setPosition(lua_State *L);
	static int setPositionX(lua_State *L);
	static int setPositionY(lua_State *L);
	static int setRotation(lua_State *L);
	static int setScale(lua_State *L);
	static int setScaleX(lua_State *L);
	static int setScaleY(lua_State *L);
	static int setAnchorPoint(lua_State *L);
	static int setAnchorPointX(lua_State *L);
	static int setAnchorPointY(lua_State *L);
	static int setColor(lua_State *L);
	static int setTexRect(lua_State *L);
	static int setFlippedX(lua_State *L);
	static int setFlippedY(lua_State *L);
	static int setRgbBlendingPreset(lua_State *L);
	static int setAlphaBlendingPreset(lua_State *L);

	static int setVertices(lua_State *L);
	static int setVerticesXY(lua_State *L);
	static int setVerticesUV(lua_State *L);
	static int setVerticesX(lua_State *L);
	static int setVerticesY(lua_State *L);
	static int setVerticesU(lua_State *L);
	static int setVerticesV(lua_State *L);

	friend class Script;
};

#endif
