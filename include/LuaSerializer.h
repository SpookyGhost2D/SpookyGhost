#ifndef CLASS_LUASERIALIZER
#define CLASS_LUASERIALIZER

#include <nctl/String.h>
#include <nctl/Array.h>
#include <nctl/HashMap.h>
#include <ncine/LuaUtils.h>

struct lua_State;

namespace ncine {

class LuaStateManager;

template <class T> class Rect;
using Recti = Rect<int>;
using Rectf = Rect<float>;

template <class T> class Vector2;
using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;

template <class T> class Vector3;
using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;

class Color;
class Colorf;

}

namespace nc = ncine;

/// The class that helps with Lua serialization
class LuaSerializer
{
  public:
	LuaSerializer(unsigned int bufferSize);

	void reset();

	bool load(const char *filename);
#ifdef __EMSCRIPTEN__
	bool ((const char *filename, const nc::EmscriptenLocalFile *localFile);
#endif
	void save(const char *filename);

	inline void setContext(void *context) { context_ = context; }
	inline void *context() const { return context_; }

	inline void indent() { indentAmount_++; }
	inline void unindent() { indentAmount_--; }

	nctl::String &buffer();

	lua_State *luaState();

  private:
	nctl::String bufferString_;
	int indentAmount_;
	nctl::UniquePtr<nc::LuaStateManager> luaState_;
	void *context_;

	void createNewState();
};

namespace Serializers {

template <class T>
void serialize(LuaSerializer &ls, const char *name, const nctl::Array<const T *> &array)
{
	ASSERT(name);
	ls.buffer().formatAppend("%s =\n", name);
	ls.buffer().append("{\n");
	ls.indent();
	for (unsigned int i = 0; i < array.size(); i++)
		serialize(ls, array[i]);
	ls.unindent();
	ls.buffer().append("}\n");
}

template <class T>
void serialize(LuaSerializer &ls, const char *name, const nctl::Array<nctl::UniquePtr<T>> &array)
{
	ASSERT(name);
	ls.buffer().formatAppend("%s =\n", name);
	ls.buffer().append("{\n");
	ls.indent();
	for (unsigned int i = 0; i < array.size(); i++)
		serialize(ls, *array[i]);
	ls.unindent();
	ls.buffer().append("}\n");
}

void serialize(LuaSerializer &ls, const char *name, bool boolean);
void serialize(LuaSerializer &ls, const char *name, int number);
void serialize(LuaSerializer &ls, const char *name, unsigned int number);
void serialize(LuaSerializer &ls, const char *name, float number);
void serialize(LuaSerializer &ls, const char *name, const nctl::String &string);
void serialize(LuaSerializer &ls, const char *name, const char *string);

void serialize(LuaSerializer &ls, const char *name, const nc::Recti &rect);
void serialize(LuaSerializer &ls, const char *name, const nc::Rectf &rect);

void serialize(LuaSerializer &ls, const char *name, const nc::Vector2i &vector);
void serialize(LuaSerializer &ls, const char *name, const nc::Vector2f &vector);
void serialize(LuaSerializer &ls, const char *name, const nc::Vector3i &vector);
void serialize(LuaSerializer &ls, const char *name, const nc::Vector3f &vector);

void serialize(LuaSerializer &ls, const char *name, const nc::Color &color);
void serialize(LuaSerializer &ls, const char *name, const nc::Colorf &color);

template <class T> void serializePtr(LuaSerializer &ls, const char *name, const T *ptr, const nctl::HashMap<const T *, unsigned int> &hash)
{
	const unsigned int *indexFind = hash.find(ptr);
	const int index = indexFind ? static_cast<int>(*indexFind) : -1;
	serialize(ls, name, index);
}

void serializeGlobal(LuaSerializer &ls, const char *name, bool boolean);
void serializeGlobal(LuaSerializer &ls, const char *name, int number);
void serializeGlobal(LuaSerializer &ls, const char *name, unsigned int number);
void serializeGlobal(LuaSerializer &ls, const char *name, float number);
void serializeGlobal(LuaSerializer &ls, const char *name, const nctl::String &string);
void serializeGlobal(LuaSerializer &ls, const char *name, const char *string);

void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Recti &rect);
void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Rectf &rect);

void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Vector2i &vector);
void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Vector2f &vector);
void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Vector3i &vector);
void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Vector3f &vector);

void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Color &color);
void serializeGlobal(LuaSerializer &ls, const char *name, const nc::Colorf &color);

}

namespace Deserializers {

class Array
{
  public:
	Array(LuaSerializer &ls, const char *name);
	~Array();
	inline unsigned int size() const { return size_; }
	inline unsigned int index() const { return index_; }
	inline bool hasNext() const { return index_ < size_; }
	bool next();

  private:
	LuaSerializer &ls_;
	unsigned int size_;
	unsigned int index_;
};

template <class T>
bool deserialize(LuaSerializer &ls, const char *name, nctl::Array<T> &array)
{
	ASSERT(name);
	lua_State *L = ls.luaState();

	if (nc::LuaUtils::tryRetrieveGlobalTable(L, name) == false)
		return false;
	const unsigned int numElements = nc::LuaUtils::rawLen(L, -1);
	for (unsigned int i = 0; i < numElements; i++)
	{
		nc::LuaUtils::rawGeti(L, -1, i + 1); // Lua arrays start from index 1
		deserialize(ls, array[i]);
		nc::LuaUtils::pop(L);
	}

	return true;
}

template <class T> T deserialize(LuaSerializer &ls, const char *name) {}
template <> bool deserialize<bool>(LuaSerializer &ls, const char *name);
template <> int deserialize<int>(LuaSerializer &ls, const char *name);
template <> unsigned int deserialize<unsigned int>(LuaSerializer &ls, const char *name);
template <> float deserialize<float>(LuaSerializer &ls, const char *name);
template <> const char *deserialize<const char *>(LuaSerializer &ls, const char *name);

template <> nc::Recti deserialize<nc::Recti>(LuaSerializer &ls, const char *name);
template <> nc::Rectf deserialize<nc::Rectf>(LuaSerializer &ls, const char *name);

template <> nc::Vector2i deserialize<nc::Vector2i>(LuaSerializer &ls, const char *name);
template <> nc::Vector2f deserialize<nc::Vector2f>(LuaSerializer &ls, const char *name);
template <> nc::Vector3i deserialize<nc::Vector3i>(LuaSerializer &ls, const char *name);
template <> nc::Vector3f deserialize<nc::Vector3f>(LuaSerializer &ls, const char *name);

template <> nc::Colorf deserialize<nc::Colorf>(LuaSerializer &ls, const char *name);

template <class T> T deserializeGlobal(LuaSerializer &ls, const char *name) {}
template <> bool deserializeGlobal<bool>(LuaSerializer &ls, const char *name);
template <> int deserializeGlobal<int>(LuaSerializer &ls, const char *name);
template <> unsigned int deserializeGlobal<unsigned int>(LuaSerializer &ls, const char *name);
template <> float deserializeGlobal<float>(LuaSerializer &ls, const char *name);
template <> const char *deserializeGlobal<const char *>(LuaSerializer &ls, const char *name);

void deserialize(LuaSerializer &ls, const char *name, bool &boolean);
void deserialize(LuaSerializer &ls, const char *name, int &number);
void deserialize(LuaSerializer &ls, const char *name, unsigned int &number);
void deserialize(LuaSerializer &ls, const char *name, float &number);
void deserialize(LuaSerializer &ls, const char *name, nctl::String &string);
void deserialize(LuaSerializer &ls, const char *name, char *string);

void deserialize(LuaSerializer &ls, const char *name, nc::Recti &rect);
void deserialize(LuaSerializer &ls, const char *name, nc::Rectf &rect);

void deserialize(LuaSerializer &ls, const char *name, nc::Vector2i &vector);
void deserialize(LuaSerializer &ls, const char *name, nc::Vector2f &vector);
void deserialize(LuaSerializer &ls, const char *name, nc::Vector3i &vector);
void deserialize(LuaSerializer &ls, const char *name, nc::Vector3f &vector);

void deserialize(LuaSerializer &ls, const char *name, nc::Colorf &color);

template <class T> T *deserializePtr(LuaSerializer &ls, const char *name, nctl::Array<T *> &array)
{
	const int index = deserialize<int>(ls, name);
	T *ptr = (index >= 0 && index < array.size()) ? array[index] : nullptr;
	return ptr;
}

template <class T> T *deserializePtr(LuaSerializer &ls, const char *name, nctl::Array<nctl::UniquePtr<T>> &array)
{
	const int index = deserialize<int>(ls, name);
	T *ptr = (index >= 0 && index < array.size()) ? array[index].get() : nullptr;
	return ptr;
}

void deserializeGlobal(LuaSerializer &ls, const char *name, bool &boolean);
void deserializeGlobal(LuaSerializer &ls, const char *name, int &number);
void deserializeGlobal(LuaSerializer &ls, const char *name, unsigned int &number);
void deserializeGlobal(LuaSerializer &ls, const char *name, float &number);
void deserializeGlobal(LuaSerializer &ls, const char *name, nctl::String &string);
void deserializeGlobal(LuaSerializer &ls, const char *name, char *string);

}

#endif
