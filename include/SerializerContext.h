#ifndef CLASS_SERIALIZERCONTEXT
#define CLASS_SERIALIZERCONTEXT

#include <nctl/UniquePtr.h>
#include <nctl/HashMap.h>
#include <nctl/Array.h>

struct SerializerContext
{
	nctl::UniquePtr<nctl::HashMap<const Texture *, unsigned int>> textureHash;
	nctl::UniquePtr<nctl::HashMap<const SpriteEntry *, unsigned int>> spriteEntryHash;
	nctl::UniquePtr<nctl::HashMap<const Script *, unsigned int>> scriptHash;
	nctl::UniquePtr<nctl::HashMap<const IAnimation *, unsigned int>> animationHash;
};

struct DeserializerContext
{
	int version = 0;
	nctl::Array<nctl::UniquePtr<Texture>> *textures = nullptr;
	nctl::Array<nctl::UniquePtr<SpriteEntry>> *spriteEntries = nullptr;
	nctl::Array<nctl::UniquePtr<Script>> *scripts = nullptr;
	nctl::Array<nctl::UniquePtr<IAnimation>> *animations = nullptr;
	nctl::HashMap<const char *, const GridFunction *> *functionHash = nullptr;
};

#endif
