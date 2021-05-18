#include "Texture.h"
#include <ncine/GLTexture.h>
#include <ncine/ITextureLoader.h>

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Texture::Texture(const char *filename)
    : Texture(filename, 0, 0)
{
}

Texture::Texture(const char *filename, int width, int height)
    : glTexture_(nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D)),
      name_(MaxNameLength), width_(0), height_(0), numChannels_(0), dataSize_(0)
{
	loadFromFile(filename, width, height);
}

Texture::Texture(const char *bufferName, const unsigned char *bufferPtr, unsigned long int bufferSize)
    : Texture(bufferName, bufferPtr, bufferSize, 0, 0)
{
}

Texture::Texture(const char *bufferName, const unsigned char *bufferPtr, unsigned long int bufferSize, int width, int height)
    : glTexture_(nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D)),
      name_(MaxNameLength), width_(0), height_(0), numChannels_(0), dataSize_(0)
{
	loadFromMemory(bufferName, bufferPtr, bufferSize, width, height);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Texture::bind()
{
	glTexture_->bind();
}

void *Texture::imguiTexId()
{
	return reinterpret_cast<void *>(glTexture_.get());
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool Texture::loadFromFile(const char *filename, int width, int height)
{
	glTexture_->bind();
	nctl::UniquePtr<nc::ITextureLoader> texLoader = nc::ITextureLoader::createFromFile(filename);
	if (texLoader->hasLoaded() == false)
		return false;

	load(*texLoader.get(), width, height);
	name_ = filename;
	return true;
}

bool Texture::loadFromMemory(const char *bufferName, const unsigned char *bufferPtr, unsigned long int bufferSize, int width, int height)
{
	glTexture_->bind();
	nctl::UniquePtr<nc::ITextureLoader> texLoader = nc::ITextureLoader::createFromMemory(bufferName, bufferPtr, bufferSize);
	if (texLoader->hasLoaded() == false)
		return false;

	load(*texLoader.get(), width, height);
	name_ = bufferName;
	return true;
}

void Texture::load(const nc::ITextureLoader &texLoader, int width, int height)
{
	// Loading a texture without overriding the size detected by the loader
	if (width == 0 || height == 0)
	{
		width = texLoader.width();
		height = texLoader.height();
	}

	const nc::IGfxCapabilities &gfxCaps = nc::theServiceLocator().gfxCapabilities();
	const int maxTextureSize = gfxCaps.value(nc::IGfxCapabilities::GLIntValues::MAX_TEXTURE_SIZE);
	FATAL_ASSERT_MSG_X(width <= maxTextureSize, "Texture width %d is bigger than device maximum %d", width, maxTextureSize);
	FATAL_ASSERT_MSG_X(height <= maxTextureSize, "Texture height %d is bigger than device maximum %d", height, maxTextureSize);

	glTexture_->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexture_->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexture_->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexture_->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	const nc::TextureFormat &texFormat = texLoader.texFormat();

#if (defined(__ANDROID__) && GL_ES_VERSION_3_0) || defined(WITH_ANGLE) || defined(__EMSCRIPTEN__)
	const bool withTexStorage = true;
#else
	const bool withTexStorage = gfxCaps.hasExtension(nc::IGfxCapabilities::GLExtensions::ARB_TEXTURE_STORAGE);
#endif

	if (withTexStorage)
	{
		glTexture_->texStorage2D(texLoader.mipMapCount(), texFormat.internalFormat(), width, height);
		glTexture_->texSubImage2D(0, 0, 0, width, height, texFormat.format(), texFormat.type(), texLoader.pixels());
	}
	else
		glTexture_->texImage2D(0, texFormat.internalFormat(), width, height, texFormat.format(), texFormat.type(), texLoader.pixels());

	width_ = width;
	height_ = height;
	numChannels_ = texFormat.numChannels();
	dataSize_ = texLoader.dataSize();
}
