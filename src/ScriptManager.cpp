#include <ncine/LuaUtils.h>
#include <ncine/LuaVector2Utils.h>
#include <ncine/LuaColorUtils.h>
#include <ncine/LuaRectUtils.h>
#include "singletons.h"
#include "ScriptManager.h"
#include "Script.h"
#include "SpriteManager.h"
#include "Sprite.h"
#include "Texture.h"
#include "Canvas.h"

namespace {
const char *spriteKey = "k";

static const char *vertexX = "x";
static const char *vertexY = "y";
static const char *vertexU = "u";
static const char *vertexV = "v";

enum class Components
{
	XYUV,
	XY,
	UV,
	X,
	Y,
	U,
	V
};

void verticesHelper(lua_State *L, Sprite *sprite, Components components)
{
	if (sprite)
	{
		const nctl::Array<Sprite::Vertex> &vertices = sprite->interleavedVertices();
		nc::LuaUtils::createTable(L, vertices.size(), 0);
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			const Sprite::Vertex &vertex = vertices[i];
			switch (components)
			{
				case Components::XYUV:
				{
					nc::LuaUtils::createTable(L, 0, 4);
					nc::LuaUtils::pushField(L, vertexX, vertex.x);
					nc::LuaUtils::pushField(L, vertexY, vertex.y);
					nc::LuaUtils::pushField(L, vertexU, vertex.u);
					nc::LuaUtils::pushField(L, vertexV, vertex.v);
				}
					break;
				case Components::XY:
				{
					nc::LuaUtils::createTable(L, 0, 2);
					nc::LuaUtils::pushField(L, vertexX, vertex.x);
					nc::LuaUtils::pushField(L, vertexY, vertex.y);
				}
					break;
				case Components::UV:
				{
					nc::LuaUtils::createTable(L, 0, 2);
					nc::LuaUtils::pushField(L, vertexU, vertex.u);
					nc::LuaUtils::pushField(L, vertexV, vertex.v);
				}
					break;
				case Components::X:
					nc::LuaUtils::push(L, vertex.x);
					break;
				case Components::Y:
					nc::LuaUtils::push(L, vertex.y);
					break;
				case Components::U:
					nc::LuaUtils::push(L, vertex.u);
					break;
				case Components::V:
					nc::LuaUtils::push(L, vertex.v);
					break;
			}

			nc::LuaUtils::rawSeti(L, -2, i + 1); // Lua arrays start from index 1
		}
	}
}

void setVerticesHelper(lua_State *L, Sprite *sprite, Components components)
{
	if (sprite)
	{
		nctl::Array<Sprite::Vertex> &vertices = sprite->interleavedVertices();
		if (nc::LuaUtils::isTable(L, -1) && nc::LuaUtils::rawLen(L, -1) == vertices.size())
		{
			for (unsigned int i = 0; i < vertices.size(); i++)
			{
				nc::LuaUtils::rawGeti(L, -1, i + 1); // Lua arrays start from index 1
				switch (components)
				{
					case Components::XYUV:
					{
						const float x = nc::LuaUtils::retrieveField<float>(L, -1, vertexX);
						const float y = nc::LuaUtils::retrieveField<float>(L, -1, vertexY);
						const float u = nc::LuaUtils::retrieveField<float>(L, -1, vertexU);
						const float v = nc::LuaUtils::retrieveField<float>(L, -1, vertexV);
						vertices[i].x = x;
						vertices[i].y = y;
						vertices[i].u = u;
						vertices[i].v = v;
					}
						break;
					case Components::XY:
					{
						const float x = nc::LuaUtils::retrieveField<float>(L, -1, vertexX);
						const float y = nc::LuaUtils::retrieveField<float>(L, -1, vertexY);
						vertices[i].x = x;
						vertices[i].y = y;
					}
						break;
					case Components::UV:
					{
						const float u = nc::LuaUtils::retrieveField<float>(L, -1, vertexU);
						const float v = nc::LuaUtils::retrieveField<float>(L, -1, vertexV);
						vertices[i].u = u;
						vertices[i].v = v;
					}
						break;
					case Components::X:
						vertices[i].x = nc::LuaUtils::retrieve<float>(L, -1);
						break;
					case Components::Y:
						vertices[i].y = nc::LuaUtils::retrieve<float>(L, -1);
						break;
					case Components::U:
						vertices[i].u = nc::LuaUtils::retrieve<float>(L, -1);
						break;
					case Components::V:
						vertices[i].v = nc::LuaUtils::retrieve<float>(L, -1);
						break;
				}
				nc::LuaUtils::pop(L);
			}
		}
	}
}

}

namespace LuaNames {

static const char *DISABLED = "DISABLED";
static const char *ALPHA = "ALPHA";
static const char *PREMULTIPLIED_ALPHA = "PREMULTIPLIED_ALPHA";
static const char *ADDITIVE = "ADDITIVE";
static const char *MULTIPLY = "MULTIPLY";
static const char *BlendingPreset = "blending_preset";

static const char *canvasWidth = "get_canvas_width";
static const char *canvasHeight = "get_canvas_height";
static const char *textureWidth = "get_texture_width";
static const char *textureHeight = "get_texture_height";
static const char *width = "get_width";
static const char *height = "get_height";

static const char *position = "get_position";
static const char *positionX = "get_x";
static const char *positionY = "get_y";
static const char *rotation = "get_rotation";
static const char *scale = "get_scale";
static const char *scaleX = "get_scale_x";
static const char *scaleY = "get_scale_y";
static const char *anchorPoint = "get_anchor";
static const char *anchorPointX = "get_anchor_x";
static const char *anchorPointY = "get_anchor_y";
static const char *color = "get_color";
static const char *texRect = "get_texrect";
static const char *isFlippedX = "get_flipped_x";
static const char *isFlippedY = "get_flipped_y";
static const char *blendingPreset = "get_blending";
static const char *numVertices = "get_num_vertices";
static const char *vertices = "get_vertices";
static const char *verticesXY = "get_vertices_xy";
static const char *verticesUV = "get_vertices_uv";
static const char *verticesX = "get_vertices_x";
static const char *verticesY = "get_vertices_y";
static const char *verticesU = "get_vertices_u";
static const char *verticesV = "get_vertices_v";

static const char *setPosition = "set_position";
static const char *setPositionX = "set_x";
static const char *setPositionY = "set_y";
static const char *setRotation = "set_rotation";
static const char *setScale = "set_scale";
static const char *setScaleX = "set_scale_x";
static const char *setScaleY = "set_scale_y";
static const char *setAnchorPoint = "set_anchor";
static const char *setAnchorPointX = "set_anchor_x";
static const char *setAnchorPointY = "set_anchor_y";
static const char *setColor = "set_color";
static const char *setTexRect = "set_texrect";
static const char *setFlippedX = "set_flipped_x";
static const char *setFlippedY = "set_flipped_y";
static const char *setBlendingPreset = "set_blending";
static const char *setVertices = "set_vertices";
static const char *setVerticesXY = "set_vertices_xy";
static const char *setVerticesUV = "set_vertices_uv";
static const char *setVerticesX = "set_vertices_x";
static const char *setVerticesY = "set_vertices_y";
static const char *setVerticesU = "set_vertices_u";
static const char *setVerticesV = "set_vertices_v";

}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

int ScriptManager::scriptIndex(const Script *script) const
{
	if (script == nullptr)
		return -1;

	int index = -1;
	for (unsigned int i = 0; i < scripts_.size(); i++)
	{
		if (scripts_[i].get() == script)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	return index;
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

Sprite *ScriptManager::retrieveSprite(lua_State *L)
{
	Sprite *sprite = nullptr;

	nc::LuaUtils::push(L, reinterpret_cast<void *>(&spriteKey));
	const int type = nc::LuaUtils::getTable(L, nc::LuaUtils::registryIndex());
	if (nc::LuaUtils::isLightUserData(type))
		sprite = reinterpret_cast<Sprite *>(nc::LuaUtils::retrieveUserData(L, -1));
	nc::LuaUtils::pop(L, 1);

	return sprite;
}

void ScriptManager::pushSprite(lua_State *L, Sprite *sprite)
{
	nc::LuaUtils::push(L, reinterpret_cast<void *>(&spriteKey));
	nc::LuaUtils::push(L, sprite);
	nc::LuaUtils::setTable(L, nc::LuaUtils::registryIndex());
}

void ScriptManager::exposeConstants(lua_State *L)
{
	nc::LuaUtils::createTable(L, 0, 5);

	nc::LuaUtils::pushField(L, LuaNames::DISABLED, static_cast<int64_t>(Sprite::BlendingPreset::DISABLED));
	nc::LuaUtils::pushField(L, LuaNames::ALPHA, static_cast<int64_t>(Sprite::BlendingPreset::ALPHA));
	nc::LuaUtils::pushField(L, LuaNames::PREMULTIPLIED_ALPHA, static_cast<int64_t>(Sprite::BlendingPreset::PREMULTIPLIED_ALPHA));
	nc::LuaUtils::pushField(L, LuaNames::ADDITIVE, static_cast<int64_t>(Sprite::BlendingPreset::ADDITIVE));
	nc::LuaUtils::pushField(L, LuaNames::MULTIPLY, static_cast<int64_t>(Sprite::BlendingPreset::MULTIPLY));

	nc::LuaUtils::setGlobal(L, LuaNames::BlendingPreset);
}

void ScriptManager::exposeFunctions(lua_State *L)
{
	nc::LuaUtils::addGlobalFunction(L, LuaNames::canvasWidth, canvasWidth);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::canvasHeight, canvasHeight);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::textureWidth, textureWidth);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::textureHeight, textureHeight);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::width, width);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::height, height);

	nc::LuaUtils::addGlobalFunction(L, LuaNames::position, position);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::positionX, positionX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::positionY, positionY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::rotation, rotation);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::scale, scale);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::scaleX, scaleX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::scaleY, scaleY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::anchorPoint, anchorPoint);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::anchorPointX, anchorPointX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::anchorPointY, anchorPointY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::color, color);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::texRect, texRect);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::isFlippedX, isFlippedX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::isFlippedY, isFlippedY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::blendingPreset, blendingPreset);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::numVertices, numVertices);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::vertices, vertices);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesXY, verticesXY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesUV, verticesUV);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesX, verticesX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesY, verticesY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesU, verticesU);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::verticesV, verticesV);

	nc::LuaUtils::addGlobalFunction(L, LuaNames::setPosition, setPosition);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setPositionX, setPositionX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setPositionY, setPositionY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setRotation, setRotation);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setScale, setScale);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setScaleX, setScaleX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setScaleY, setScaleY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setAnchorPoint, setAnchorPoint);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setAnchorPointX, setAnchorPointX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setAnchorPointY, setAnchorPointY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setColor, setColor);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setTexRect, setTexRect);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setFlippedX, setFlippedX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setFlippedY, setFlippedY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setBlendingPreset, setBlendingPreset);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVertices, setVertices);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesXY, setVerticesXY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesUV, setVerticesUV);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesX, setVerticesX);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesY, setVerticesY);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesU, setVerticesU);
	nc::LuaUtils::addGlobalFunction(L, LuaNames::setVerticesV, setVerticesV);
}

int ScriptManager::canvasWidth(lua_State *L)
{
	nc::LuaUtils::push(L, theCanvas->texWidth());
	return 1;
}

int ScriptManager::canvasHeight(lua_State *L)
{
	nc::LuaUtils::push(L, theCanvas->texHeight());
	return 1;
}

int ScriptManager::textureWidth(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const int width = sprite ? sprite->texture().width() : 0;
	nc::LuaUtils::push(L, width);

	return 1;
}

int ScriptManager::textureHeight(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const int height = sprite ? sprite->texture().height() : 0;
	nc::LuaUtils::push(L, height);

	return 1;
}

int ScriptManager::width(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const int width = sprite ? sprite->width() : 0;
	nc::LuaUtils::push(L, width);

	return 1;
}

int ScriptManager::height(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const int height = sprite ? sprite->height() : 0;
	nc::LuaUtils::push(L, height);

	return 1;
}

int ScriptManager::position(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const nc::Vector2f pos = sprite ? nc::Vector2f(sprite->x, sprite->y) : nc::Vector2f(0.0f, 0.0f);
	nc::LuaVector2fUtils::push(L, pos);

	return 1;
}

int ScriptManager::positionX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float x = sprite ? sprite->x : 0.0f;
	nc::LuaUtils::push(L, x);

	return 1;
}

int ScriptManager::positionY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float y = sprite ? sprite->y : 0.0f;
	nc::LuaUtils::push(L, y);

	return 1;
}

int ScriptManager::rotation(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float rot = sprite ? sprite->rotation : 0.0f;
	nc::LuaUtils::push(L, rot);

	return 1;
}

int ScriptManager::scale(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const nc::Vector2f scale = sprite ? sprite->scaleFactor : nc::Vector2f(1.0f, 1.0f);
	nc::LuaVector2fUtils::push(L, scale);

	return 1;
}

int ScriptManager::scaleX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float scaleX = sprite ? sprite->scaleFactor.x : 1.0f;
	nc::LuaUtils::push(L, scaleX);

	return 1;
}

int ScriptManager::scaleY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float scaleY = sprite ? sprite->scaleFactor.y : 1.0f;
	nc::LuaUtils::push(L, scaleY);

	return 1;
}

int ScriptManager::anchorPoint(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const nc::Vector2f anchorPoint = sprite ? sprite->anchorPoint : nc::Vector2f(0.0f, 0.0f);
	nc::LuaVector2fUtils::push(L, anchorPoint);

	return 1;
}

int ScriptManager::anchorPointX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float anchorPointX = sprite ? sprite->anchorPoint.x : 0.0f;
	nc::LuaUtils::push(L, anchorPointX);

	return 1;
}

int ScriptManager::anchorPointY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const float anchorPointY = sprite ? sprite->anchorPoint.y : 0.0f;
	nc::LuaUtils::push(L, anchorPointY);

	return 1;
}

int ScriptManager::color(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const nc::Colorf color = sprite ? sprite->color : nc::Colorf::White;
	nc::LuaColorUtils::push(L, color);

	return 1;
}

int ScriptManager::texRect(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const nc::Recti texRect = sprite ? sprite->texRect() : nc::Recti();
	nc::LuaRectiUtils::push(L, texRect);

	return 1;
}

int ScriptManager::isFlippedX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const bool isFlippedX = sprite ? sprite->isFlippedX() : false;
	nc::LuaUtils::push(L, isFlippedX);

	return 1;
}

int ScriptManager::isFlippedY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const bool isFlippedY = sprite ? sprite->isFlippedY() : false;
	nc::LuaUtils::push(L, isFlippedY);

	return 1;
}

int ScriptManager::blendingPreset(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const Sprite::BlendingPreset blendingPreset = sprite ? sprite->blendingPreset() : Sprite::BlendingPreset::DISABLED;
	nc::LuaUtils::push(L, static_cast<int64_t>(blendingPreset));

	return 1;
}

int ScriptManager::numVertices(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	const unsigned int numVertices = sprite ? sprite->interleavedVertices().size() : 0;
	nc::LuaUtils::push(L, numVertices);

	return 1;
}

int ScriptManager::vertices(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::XYUV);

	return 1;
}

int ScriptManager::verticesXY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::XY);

	return 1;
}

int ScriptManager::verticesUV(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::UV);

	return 1;
}

int ScriptManager::verticesX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::X);

	return 1;
}

int ScriptManager::verticesY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::Y);

	return 1;
}

int ScriptManager::verticesU(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::U);
	return 1;
}

int ScriptManager::verticesV(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	verticesHelper(L, sprite, Components::V);

	return 1;
}

int ScriptManager::setPosition(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const nc::Vector2f pos = nc::LuaVector2fUtils::retrieveTable(L, -1);
		sprite->x = pos.x;
		sprite->y = pos.y;
	}

	return 0;
}

int ScriptManager::setPositionX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float x = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->x = x;
	}

	return 0;
}

int ScriptManager::setPositionY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float y = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->y = y;
	}

	return 0;
}

int ScriptManager::setRotation(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float rot = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->rotation = rot;
	}

	return 0;
}

int ScriptManager::setScale(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const nc::Vector2f scale = nc::LuaVector2fUtils::retrieveTable(L, -1);
		sprite->scaleFactor = scale;
	}

	return 0;
}

int ScriptManager::setScaleX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float scaleX = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->scaleFactor.x = scaleX;
	}

	return 0;
}

int ScriptManager::setScaleY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float scaleY = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->scaleFactor.y = scaleY;
	}

	return 0;
}

int ScriptManager::setAnchorPoint(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const nc::Vector2f anchorPoint = nc::LuaVector2fUtils::retrieveTable(L, -1);
		sprite->anchorPoint = anchorPoint;
	}

	return 0;
}

int ScriptManager::setAnchorPointX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float anchorPointX = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->anchorPoint.x = anchorPointX;
	}

	return 0;
}

int ScriptManager::setAnchorPointY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const float anchorPointY = nc::LuaUtils::retrieve<float>(L, -1);
		sprite->anchorPoint.y = anchorPointY;
	}

	return 0;
}

int ScriptManager::setColor(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		int colorIndex = 0;
		const nc::Colorf color = nc::LuaColorUtils::retrieve(L, -1, colorIndex);
		sprite->color = color;
	}

	return 0;
}

int ScriptManager::setTexRect(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		int rectIndex = 0;
		const nc::Recti texRect = nc::LuaRectiUtils::retrieve(L, -1, rectIndex);
		sprite->setTexRect(texRect);
	}

	return 0;
}

int ScriptManager::setFlippedX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const bool flippedX = nc::LuaUtils::retrieve<bool>(L, -1);
		sprite->setFlippedX(flippedX);
	}

	return 0;
}

int ScriptManager::setFlippedY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const bool flippedY = nc::LuaUtils::retrieve<bool>(L, -1);
		sprite->setFlippedY(flippedY);
	}

	return 0;
}

int ScriptManager::setBlendingPreset(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	if (sprite)
	{
		const Sprite::BlendingPreset blendingPreset = static_cast<Sprite::BlendingPreset>(nc::LuaUtils::retrieve<int64_t>(L, -1));
		sprite->setBlendingPreset(blendingPreset);
	}

	return 0;
}

int ScriptManager::setVertices(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::XYUV);

	return 0;
}

int ScriptManager::setVerticesXY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::XY);

	return 0;
}

int ScriptManager::setVerticesUV(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::UV);

	return 0;
}

int ScriptManager::setVerticesX(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::X);

	return 0;
}

int ScriptManager::setVerticesY(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::Y);

	return 0;
}

int ScriptManager::setVerticesU(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::U);

	return 0;
}

int ScriptManager::setVerticesV(lua_State *L)
{
	Sprite *sprite = retrieveSprite(L);
	setVerticesHelper(L, sprite, Components::V);

	return 0;
}
