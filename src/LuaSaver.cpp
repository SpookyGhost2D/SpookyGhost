#include "LuaSaver.h"
#include "SpriteManager.h"
#include "Texture.h"
#include "ScriptManager.h"
#include "AnimationManager.h"
#include "GridFunctionLibrary.h"
#include "Configuration.h"

#include "Serializers.h"
#include "LuaSerializer.h"
#include "SerializerContext.h"

namespace {

const int ProjectVersion = 4;

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

LuaSaver::LuaSaver(unsigned int bufferSize)
{
	serializer_ = nctl::makeUnique<LuaSerializer>(bufferSize);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

bool LuaSaver::load(const char *filename, Data &data)
{
	DeserializerContext context;
	serializer_->setContext(&context);
	context.textures = &data.spriteMgr.textures();
	context.sprites = &data.spriteMgr.sprites();
	context.scripts = &data.scriptMgr.scripts();
	nctl::Array<nctl::UniquePtr<IAnimation>> anims;
	context.animations = &anims;

	const unsigned int numFunctions = GridFunctionLibrary::gridFunctions().size();
	nctl::HashMap<const char *, const GridFunction *> functionHash(numFunctions * 2);
	context.functionHash = &functionHash;
	for (unsigned int i = 0; i < numFunctions; i++)
	{
		const GridFunction &function = GridFunctionLibrary::gridFunctions()[i];
		functionHash.insert(function.name().data(), &function);
	}

	if (serializer_->load(filename) == false)
		return false;

	data.spriteMgr.textures().clear();
	data.spriteMgr.sprites().clear();
	data.scriptMgr.scripts().clear();
	data.animMgr.anims().clear();

	Deserializers::deserializeGlobal(*serializer_, "version", context.version);
	ASSERT(context.version >= 1);
	Deserializers::deserialize(*serializer_, "canvas", data.canvas);
	Deserializers::deserialize(*serializer_, "textures", *context.textures);
	Deserializers::deserialize(*serializer_, "sprites", *context.sprites);
	if (context.version >= 2)
		Deserializers::deserialize(*serializer_, "scripts", *context.scripts);
	Deserializers::deserialize(*serializer_, "animations", *context.animations);

	if (data.animMgr.anims().capacity() < anims.size())
		data.animMgr.anims().setCapacity(anims.size());
	for (unsigned int i = 0; i < anims.size(); i++)
		anims[i]->parent()->anims().pushBack(nctl::move(anims[i]));

	return true;
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

	Serializers::serializeGlobal(*serializer_, "version", ProjectVersion);
	serializer_->buffer().append("\n");
	Serializers::serialize(*serializer_, "canvas", data.canvas);

	const unsigned int numTextures = data.spriteMgr.textures().size();
	if (numTextures > 0)
	{
		serializer_->buffer().append("\n");
		context.textureHash = nctl::makeUnique<nctl::HashMap<const Texture *, unsigned int>>(numTextures * 2);

		for (unsigned int i = 0; i < numTextures; i++)
			context.textureHash->insert(data.spriteMgr.textures()[i].get(), i);

		Serializers::serialize(*serializer_, "textures", data.spriteMgr.textures());

		const unsigned int numSprites = data.spriteMgr.sprites().size();
		if (numSprites > 0)
		{
			serializer_->buffer().append("\n");
			context.spriteHash = nctl::makeUnique<nctl::HashMap<const Sprite *, unsigned int>>(numSprites * 2);

			for (unsigned int i = 0; i < numSprites; i++)
				context.spriteHash->insert(data.spriteMgr.sprites()[i].get(), i);

			Serializers::serialize(*serializer_, "sprites", data.spriteMgr.sprites());
		}
	}

	const unsigned int numScripts = data.scriptMgr.scripts().size();
	if (numScripts > 0)
	{
		serializer_->buffer().append("\n");
		context.scriptHash = nctl::makeUnique<nctl::HashMap<const Script *, unsigned int>>(numScripts * 2);

		for (unsigned int i = 0; i < numScripts; i++)
			context.scriptHash->insert(data.scriptMgr.scripts()[i].get(), i);

		Serializers::serialize(*serializer_, "scripts", data.scriptMgr.scripts());
	}

	nctl::Array<const IAnimation *> anims;
	for (unsigned int i = 0; i < data.animMgr.anims().size(); i++)
		visitAnimations(data.animMgr.anims()[i].get(), anims);

	const unsigned int numAnims = anims.size();
	if (numAnims > 0)
	{
		serializer_->buffer().append("\n");
		context.animationHash = nctl::makeUnique<nctl::HashMap<const IAnimation *, unsigned int>>(numAnims * 2);
		for (unsigned int i = 0; i < numAnims; i++)
			context.animationHash->insert(anims[i], i);

		Serializers::serialize(*serializer_, "animations", anims);
	}

	serializer_->save(filename);
}

bool LuaSaver::loadCfg(const char *filename, Configuration &cfg)
{
	if (serializer_->load(filename) == false)
		return false;

	Deserializers::deserialize(*serializer_, cfg);

	return true;
}

void LuaSaver::saveCfg(const char *filename, const Configuration &cfg)
{
	serializer_->reset();
	Serializers::serialize(*serializer_, cfg);
	serializer_->save(filename);
}
