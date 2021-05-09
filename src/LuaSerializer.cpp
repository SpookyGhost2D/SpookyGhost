#include "LuaSerializer.h"
#include <ncine/LuaStateManager.h>
#include <ncine/LuaVector2Utils.h>
#include <ncine/LuaVector3Utils.h>
#include <ncine/LuaRectUtils.h>
#include <ncine/LuaColorUtils.h>
#include <ncine/IFile.h>

#include <ncine/Rect.h>
#include <ncine/Vector4.h>
#include <ncine/Color.h>
#include <ncine/Colorf.h>

#ifdef __EMSCRIPTEN__
	#include <ncine/EmscriptenLocalFile.h>
#endif

namespace Serializers {

void serialize(LuaSerializer &ls, const char *name, bool boolean)
{
	ls.buffer().formatAppend("%s = %s,\n", name, boolean ? "true" : "false");
}

void serialize(LuaSerializer &ls, const char *name, int number)
{
	ls.buffer().formatAppend("%s = %d,\n", name, number);
}

void serialize(LuaSerializer &ls, const char *name, unsigned int number)
{
	ls.buffer().formatAppend("%s = %u,\n", name, number);
}

void serialize(LuaSerializer &ls, const char *name, float number)
{
	ls.buffer().formatAppend("%s = %f,\n", name, number);
}

void serialize(LuaSerializer &ls, const char *name, const nctl::String &string)
{
	ls.buffer().formatAppend("%s = \"%s\",\n", name, string.data());
}

void serialize(LuaSerializer &ls, const char *name, const char *string)
{
	ls.buffer().formatAppend("%s = \"%s\",\n", name, string);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Recti &rect)
{
	ls.buffer().formatAppend("%s = {x = %d, y = %d, w = %d, h = %d},\n", name, rect.x, rect.y, rect.w, rect.h);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Rectf &rect)
{
	ls.buffer().formatAppend("%s = {x = %f, y = %f, w = %f, h = %f},\n", name, rect.x, rect.y, rect.w, rect.h);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Vector2i &vector)
{
	ls.buffer().formatAppend("%s = {x = %d, y = %d},\n", name, vector.x, vector.y);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Vector2f &vector)
{
	ls.buffer().formatAppend("%s = {x = %f, y = %f},\n", name, vector.x, vector.y);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Vector3i &vector)
{
	ls.buffer().formatAppend("%s = {x = %d, y = %d, z = %d},\n", name, vector.x, vector.y, vector.z);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Vector3f &vector)
{
	ls.buffer().formatAppend("%s = {x = %f, y = %f, z = %f},\n", name, vector.x, vector.y, vector.z);
}

void serialize(LuaSerializer &ls, const char *name, const nc::Color &color)
{
	ls.buffer().formatAppend("%s = {r = %d, g = %d, b = %d, a = %d},\n", name, color.r(), color.g(), color.b(), color.a());
}

void serialize(LuaSerializer &ls, const char *name, const nc::Colorf &color)
{
	ls.buffer().formatAppend("%s = {r = %f, g = %f, b = %f, a = %f},\n", name, color.r(), color.g(), color.b(), color.a());
}

void serializeGlobal(LuaSerializer &ls, const char *name, bool boolean)
{
	ls.buffer().formatAppend("%s = %s\n", name, boolean ? "true" : "false");
}

void serializeGlobal(LuaSerializer &ls, const char *name, int number)
{
	ls.buffer().formatAppend("%s = %d\n", name, number);
}

void serializeGlobal(LuaSerializer &ls, const char *name, unsigned int number)
{
	ls.buffer().formatAppend("%s = %u\n", name, number);
}

void serializeGlobal(LuaSerializer &ls, const char *name, float number)
{
	ls.buffer().formatAppend("%s = %f\n", name, number);
}

void serializeGlobal(LuaSerializer &ls, const char *name, const nctl::String &string)
{
	ls.buffer().formatAppend("%s = \"%s\"\n", name, string.data());
}

void serializeGlobal(LuaSerializer &ls, const char *name, const char *string)
{
	ls.buffer().formatAppend("%s = \"%s\"\n", name, string);
}

}

namespace Deserializers {

Array::Array(LuaSerializer &ls, const char *name)
    : ls_(ls), size_(0), index_(0)
{
	lua_State *L = ls_.luaState();
	nc::LuaUtils::retrieveFieldTable(L, -1, name);
	size_ = nc::LuaUtils::rawLen(L, -1);
	if (size_ > 0)
		nc::LuaUtils::rawGeti(L, -1, index_ + 1); // Lua arrays start from index 1
}

Array::~Array()
{
	lua_State *L = ls_.luaState();
	nc::LuaUtils::pop(L); // last element

	nc::LuaUtils::pop(L); // array table
}

bool Array::next()
{
	if (hasNext())
	{
		index_++;
		lua_State *L = ls_.luaState();
		nc::LuaUtils::pop(L); // previous element
		nc::LuaUtils::rawGeti(L, -1, index_ + 1); // Lua arrays start from index 1
		return true;
	}
	return false;
}

template <>
bool deserialize<bool>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveField<bool>(ls.luaState(), -1, name);
}

template <>
int deserialize<int>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveField<int>(ls.luaState(), -1, name);
}

template <>
unsigned int deserialize<unsigned int>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveField<unsigned int>(ls.luaState(), -1, name);
}

template <>
float deserialize<float>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveField<float>(ls.luaState(), -1, name);
}

template <>
const char *deserialize<const char *>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveField<const char *>(ls.luaState(), -1, name);
}

template <>
nc::Recti deserialize<nc::Recti>(LuaSerializer &ls, const char *name)
{
	return nc::LuaRectiUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Rectf deserialize<nc::Rectf>(LuaSerializer &ls, const char *name)
{
	return nc::LuaRectfUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Vector2i deserialize<nc::Vector2i>(LuaSerializer &ls, const char *name)
{
	return nc::LuaVector2iUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Vector2f deserialize<nc::Vector2f>(LuaSerializer &ls, const char *name)
{
	return nc::LuaVector2fUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Vector3i deserialize<nc::Vector3i>(LuaSerializer &ls, const char *name)
{
	return nc::LuaVector3iUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Vector3f deserialize<nc::Vector3f>(LuaSerializer &ls, const char *name)
{
	return nc::LuaVector3fUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
nc::Colorf deserialize<nc::Colorf>(LuaSerializer &ls, const char *name)
{
	return nc::LuaColorUtils::retrieveTableField(ls.luaState(), -1, name);
}

template <>
bool deserializeGlobal<bool>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveGlobal<bool>(ls.luaState(), name);
}

template <>
int deserializeGlobal<int>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveGlobal<int>(ls.luaState(), name);
}

template <>
unsigned int deserializeGlobal<unsigned int>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveGlobal<unsigned int>(ls.luaState(), name);
}

template <>
float deserializeGlobal<float>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveGlobal<float>(ls.luaState(), name);
}

template <>
const char *deserializeGlobal<const char *>(LuaSerializer &ls, const char *name)
{
	return nc::LuaUtils::retrieveGlobal<const char *>(ls.luaState(), name);
}

void deserialize(LuaSerializer &ls, const char *name, bool &boolean)
{
	boolean = nc::LuaUtils::retrieveField<bool>(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, int &number)
{
	number = nc::LuaUtils::retrieveField<int>(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, unsigned int &number)
{
	number = nc::LuaUtils::retrieveField<unsigned int>(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, float &number)
{
	number = nc::LuaUtils::retrieveField<float>(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nctl::String &string)
{
	string = nc::LuaUtils::retrieveField<const char *>(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, char *string)
{
	size_t length;
	nc::LuaUtils::retrieveField(ls.luaState(), -1, name, string, &length);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Recti &rect)
{
	rect = nc::LuaRectiUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Rectf &rect)
{
	rect = nc::LuaRectfUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Vector2i &vector)
{
	vector = nc::LuaVector2iUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Vector2f &vector)
{
	vector = nc::LuaVector2fUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Vector3i &vector)
{
	vector = nc::LuaVector3iUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Vector3f &vector)
{
	vector = nc::LuaVector3fUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserialize(LuaSerializer &ls, const char *name, nc::Colorf &color)
{
	color = nc::LuaColorUtils::retrieveTableField(ls.luaState(), -1, name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, bool &boolean)
{
	boolean = nc::LuaUtils::retrieveGlobal<bool>(ls.luaState(), name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, int &number)
{
	number = nc::LuaUtils::retrieveGlobal<int>(ls.luaState(), name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, unsigned int &number)
{
	number = nc::LuaUtils::retrieveGlobal<unsigned int>(ls.luaState(), name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, float &number)
{
	number = nc::LuaUtils::retrieveGlobal<float>(ls.luaState(), name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, nctl::String &string)
{
	string = nc::LuaUtils::retrieveGlobal<const char *>(ls.luaState(), name);
}

void deserializeGlobal(LuaSerializer &ls, const char *name, char *string)
{
	size_t length;
	nc::LuaUtils::retrieveGlobal(ls.luaState(), name, string, &length);
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

LuaSerializer::LuaSerializer(unsigned int bufferSize)
    : luaState_(nc::LuaStateManager::ApiType::NONE,
                nc::LuaStateManager::StatisticsTracking::DISABLED,
                nc::LuaStateManager::StandardLibraries::NOT_LOADED),
      bufferString_(bufferSize), context_(nullptr)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void LuaSerializer::reset()
{
	bufferString_.clear();
	indentAmount_ = 0;
}

bool LuaSerializer::load(const char *filename)
#ifdef __EMSCRIPTEN__
{
	return load(filename, nullptr);
}

bool LuaSerializer::load(const char *filename, const nc::EmscriptenLocalFile *localFile)
#endif
{
	luaState_.reopen();
#ifndef __EMSCRIPTEN__
	if (luaState_.runFromFile(filename) == false)
#else
	const bool loaded = (localFile != nullptr)
	                        ? luaState_.runFromMemory(localFile->filename(), localFile->data(), localFile->size())
	                        : luaState_.runFromFile(filename);

	if (loaded == false)
#endif
		return false;

	return true;
}

void LuaSerializer::save(const char *filename)
{
#ifdef __EMSCRIPTEN__
	// Don't save the configuration file locally
	if (strncmp(filename, "config.lua", 10) == 0)
	{
#endif

		nctl::UniquePtr<nc::IFile> fileHandle = nc::IFile::createFileHandle(filename);
		fileHandle->open(nc::IFile::OpenMode::WRITE | nc::IFile::OpenMode::BINARY);
		fileHandle->write(bufferString_.data(), bufferString_.length());
		fileHandle->close();

#ifdef __EMSCRIPTEN__
	}
	else
	{
		nc::EmscriptenLocalFile localFileSave;
		localFileSave.write(bufferString_.data(), bufferString_.length());
		localFileSave.save(filename);
	}
#endif
}

nctl::String &LuaSerializer::buffer()
{
	FATAL_ASSERT(indentAmount_ >= 0);
	for (int i = 0; i < indentAmount_; i++)
		bufferString_.append("\t");

	return bufferString_;
}

lua_State *LuaSerializer::luaState()
{
	return luaState_.state();
}
