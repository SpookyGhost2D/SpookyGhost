#ifndef CLASS_SERIALIZERCONTEXT
#define CLASS_SERIALIZERCONTEXT

#include <nctl/UniquePtr.h>
#include <nctl/HashMap.h>
#include <nctl/Array.h>

struct SerializerContext
{
	nctl::UniquePtr<nctl::HashMap<const Texture *, unsigned int>> textureHash;
	nctl::UniquePtr<nctl::HashMap<const Sprite *, unsigned int>> spriteHash;
	nctl::UniquePtr<nctl::HashMap<const IAnimation *, unsigned int>> animationHash;
};

struct DeserializerContext
{
	nctl::Array<nctl::UniquePtr<Texture>> *textures = nullptr;
	nctl::Array<nctl::UniquePtr<Sprite>> *sprites = nullptr;
	nctl::Array<nctl::UniquePtr<IAnimation>> *animations = nullptr;
	nctl::CStringHashMap<const GridFunction *> *functionHash = nullptr;
};

#endif
