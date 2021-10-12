#include "LuaSaver.h"
#include "SpriteEntry.h"
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

const int ProjectVersion = 7;

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
	nctl::Array<nctl::UniquePtr<SpriteEntry>> spriteEntries;
	context.spriteEntries = &spriteEntries;
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

	data.spriteMgr.clear();
	data.scriptMgr.clear();
	data.animMgr.clear();

	Deserializers::deserializeGlobal(*serializer_, "version", context.version);
	ASSERT(context.version >= 1);
	Deserializers::deserialize(*serializer_, "canvas", data.canvas);
	Deserializers::deserialize(*serializer_, "textures", *context.textures);

	nctl::String spriteTableName = "sprites";
	if (context.version >= 7)
		spriteTableName = "sprite_entries";

	// Deserialize all sprites in an array that can be used by the animations
	Deserializers::deserialize(*serializer_, spriteTableName.data(), *context.spriteEntries);

	if (context.version >= 2)
		Deserializers::deserialize(*serializer_, "scripts", *context.scripts);

	Deserializers::deserialize(*serializer_, "animations", *context.animations);
	if (data.animMgr.anims().capacity() < anims.size())
		data.animMgr.anims().setCapacity(anims.size());
	for (unsigned int i = 0; i < anims.size(); i++)
		anims[i]->parent()->anims().pushBack(nctl::move(anims[i]));

	// Stop all animations to get the initial state
	data.animMgr.animGroup().stop();

	// After the animations have been deserialized, the array of sprite entries can be moved to the sprite manager
	if (data.spriteMgr.children().capacity() < spriteEntries.size())
		data.spriteMgr.children().setCapacity(spriteEntries.size());
	for (unsigned int i = 0; i < spriteEntries.size(); i++)
		spriteEntries[i]->parentGroup()->children().pushBack(nctl::move(spriteEntries[i]));
	data.spriteMgr.updateSpritesArray();

	return true;
}

void visitSpriteEntries(const SpriteEntry *spriteEntry, nctl::Array<const SpriteEntry *> &spriteEntries)
{
	spriteEntries.pushBack(spriteEntry);

	if (spriteEntry->isGroup())
	{
		const SpriteGroup *spriteGroup = spriteEntry->toGroup();
		for (unsigned int i = 0; i < spriteGroup->children().size(); i++)
			visitSpriteEntries(spriteGroup->children()[i].get(), spriteEntries);
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

		nctl::Array<const SpriteEntry *> spriteEntries;
		for (unsigned int i = 0; i < data.spriteMgr.children().size(); i++)
			visitSpriteEntries(data.spriteMgr.children()[i].get(), spriteEntries);

		const unsigned int numSpriteEntries = spriteEntries.size();
		if (numSpriteEntries > 0)
		{
			serializer_->buffer().append("\n");
			context.spriteEntryHash = nctl::makeUnique<nctl::HashMap<const SpriteEntry *, unsigned int>>(numSpriteEntries * 2);
			for (unsigned int i = 0; i < numSpriteEntries; i++)
				context.spriteEntryHash->insert(spriteEntries[i], i);

			Serializers::serialize(*serializer_, "sprite_entries", spriteEntries);
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
