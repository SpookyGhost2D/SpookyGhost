#include "LuaSaver.h"
#include "SpriteManager.h"
#include "Texture.h"
#include "AnimationManager.h"
#include "GridFunctionLibrary.h"

#include "Serializers.h"
#include "LuaSerializer.h"
#include "SerializerContext.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

LuaSaver::LuaSaver()
{
	serializer_ = nctl::makeUnique<LuaSerializer>();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void LuaSaver::load(const char *filename, Data &data)
{
	DeserializerContext context;
	serializer_->setContext(&context);
	context.textures = &data.spriteMgr.textures();
	context.sprites = &data.spriteMgr.sprites();
	context.animations = &data.animMgr.anims();

	const unsigned int numFunctions = GridFunctionLibrary::gridFunctions().size();
	nctl::CStringHashMap<const GridFunction *> functionHash(numFunctions * 2);
	context.functionHash = &functionHash;
	for (unsigned int i = 0; i < numFunctions; i++)
	{
		const GridFunction &function = GridFunctionLibrary::gridFunctions()[i];
		functionHash.insert(function.name().data(), &function);
	}

	serializer_->load(filename);

	data.spriteMgr.textures().clear();
	data.spriteMgr.sprites().clear();
	data.animMgr.anims().clear();

	Deserializers::deserialize(*serializer_, "canvas", data.canvas);
	Deserializers::deserialize(*serializer_, "textures", data.spriteMgr.textures());
	Deserializers::deserialize(*serializer_, "sprites", data.spriteMgr.sprites());
	Deserializers::deserialize(*serializer_, "animations", data.animMgr.anims());
	Deserializers::deserialize(*serializer_, "render", data.saveAnim);

	// Deleting backwards without iterators
	for (int i = data.animMgr.anims().size() - 1; i >= 0; i--)
	{
		nctl::UniquePtr<IAnimation> &anim = data.animMgr.anims()[i];
		AnimationGroup *parent = anim->parent();
		if (parent != nullptr)
		{
			parent->anims().pushBack(nctl::move(anim));
			data.animMgr.anims().removeAt(i);
		}
	}
}

void visitAnimations(const IAnimation *anim, nctl::Array<const IAnimation *> &anims)
{
	anims.pushBack(anim);

	if (anim->type() == IAnimation::Type::PARALLEL_GROUP ||
	    anim->type() == IAnimation::Type::SEQUENTIAL_GROUP)
	{
		const AnimationGroup *animGroup = static_cast<const AnimationGroup *>(anim);
		for (unsigned int i = 0; i < animGroup->anims().size(); i++)
			visitAnimations(animGroup->anims()[i].get(), anims);
	}
}

void LuaSaver::save(const char *filename, const Data &data)
{
	SerializerContext context;
	serializer_->setContext(&context);

	serializer_->reset();

	Serializers::serialize(*serializer_, "canvas", data.canvas);
	serializer_->buffer().append("\n");

	const unsigned int numTextures = data.spriteMgr.textures().size();
	if (numTextures > 0)
	{
		context.textureHash = nctl::makeUnique<nctl::HashMap<const Texture *, unsigned int>>(numTextures * 2);

		for (unsigned int i = 0; i < numTextures; i++)
			context.textureHash->insert(data.spriteMgr.textures()[i].get(), i);

		Serializers::serialize(*serializer_, "textures", data.spriteMgr.textures());
		serializer_->buffer().append("\n");

		const unsigned int numSprites = data.spriteMgr.sprites().size();
		if (numSprites > 0)
		{
			context.spriteHash = nctl::makeUnique<nctl::HashMap<const Sprite *, unsigned int>>(numSprites * 2);

			for (unsigned int i = 0; i < numSprites; i++)
				context.spriteHash->insert(data.spriteMgr.sprites()[i].get(), i);

			Serializers::serialize(*serializer_, "sprites", data.spriteMgr.sprites());
			serializer_->buffer().append("\n");
		}
	}

	nctl::Array<const IAnimation *> anims;
	for (unsigned int i = 0; i < data.animMgr.anims().size(); i++)
		visitAnimations(data.animMgr.anims()[i].get(), anims);

	const unsigned int numAnims = anims.size();
	if (numAnims > 0)
	{
		context.animationHash = nctl::makeUnique<nctl::HashMap<const IAnimation *, unsigned int>>(numAnims * 2);
		for (unsigned int i = 0; i < numAnims; i++)
			context.animationHash->insert(anims[i], i);

		Serializers::serialize(*serializer_, "animations", anims);
		serializer_->buffer().append("\n");
	}

	Serializers::serialize(*serializer_, "render", data.saveAnim);

	serializer_->save(filename);
}
