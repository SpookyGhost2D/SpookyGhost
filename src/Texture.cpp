#include "Texture.h"
#include <ncine/GLTexture.h>
#include <ncine/ITextureLoader.h>

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Texture::Texture(const char *filename)
    : glTexture_(nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D)),
      name_(MaxNameLength), width_(0), height_(0), numChannels_(0), dataSize_(0)
{
	loadFromFile(filename);
}

Texture::Texture(const char *bufferName, const unsigned char *bufferPtr, unsigned long int bufferSize)
    : glTexture_(nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D)),
      name_(MaxNameLength), width_(0), height_(0), numChannels_(0), dataSize_(0)
{
	loadFromMemory(bufferName, bufferPtr, bufferSize);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

bool Texture::loadFromFile(const char *filename)
{
	glTexture_->bind();
	nctl::UniquePtr<nc::ITextureLoader> texLoader = nc::ITextureLoader::createFromFile(filename);
	if (texLoader->hasLoaded() == false)
		return false;

	initialize(*texLoader);
	load(*texLoader);
	name_ = filename;
	return true;
}

bool Texture::loadFromMemory(const char *bufferName, const unsigned char *bufferPtr, unsigned long int bufferSize)
{
	glTexture_->bind();
	nctl::UniquePtr<nc::ITextureLoader> texLoader = nc::ITextureLoader::createFromMemory(bufferName, bufferPtr, bufferSize);
	if (texLoader->hasLoaded() == false)
		return false;

	initialize(*texLoader);
	load(*texLoader);
	name_ = bufferName;
	return true;
}

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

void Texture::initialize(const nc::ITextureLoader &texLoader)
{
	const nc::IGfxCapabilities &gfxCaps = nc::theServiceLocator().gfxCapabilities();
	const int maxTextureSize = gfxCaps.value(nc::IGfxCapabilities::GLIntValues::MAX_TEXTURE_SIZE);
	FATAL_ASSERT_MSG_X(texLoader.width() <= maxTextureSize, "Texture width %d is bigger than device maximum %d", texLoader.width(), maxTextureSize);
	FATAL_ASSERT_MSG_X(texLoader.height() <= maxTextureSize, "Texture height %d is bigger than device maximum %d", texLoader.height(), maxTextureSize);

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

	if (withTexStorage && dataSize_ > 0 &&
	    (width_ != texLoader.width() || height_ != texLoader.height() || numChannels_ != texFormat.numChannels()))
	{
		// The OpenGL texture needs to be recreated as its storage is immutable
		glTexture_ = nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D);
	}

	if (withTexStorage)
		glTexture_->texStorage2D(texLoader.mipMapCount(), texFormat.internalFormat(), texLoader.width(), texLoader.height());
	else
		glTexture_->texImage2D(0, texFormat.internalFormat(), texLoader.width(), texLoader.height(), texFormat.format(), texFormat.type(), nullptr);

	width_ = texLoader.width();
	height_ = texLoader.height();
	numChannels_ = texFormat.numChannels();
	dataSize_ = texLoader.dataSize();
}

void Texture::load(const nc::ITextureLoader &texLoader)
{
#if (defined(__ANDROID__) && GL_ES_VERSION_3_0) || defined(WITH_ANGLE) || defined(__EMSCRIPTEN__)
	const bool withTexStorage = true;
#else
	const nc::IGfxCapabilities &gfxCaps = nc::theServiceLocator().gfxCapabilities();
	const bool withTexStorage = gfxCaps.hasExtension(nc::IGfxCapabilities::GLExtensions::ARB_TEXTURE_STORAGE);
#endif

	const nc::TextureFormat &texFormat = texLoader.texFormat();

	if (withTexStorage)
		glTexture_->texSubImage2D(0, 0, 0, texLoader.width(), texLoader.height(), texFormat.format(), texFormat.type(), texLoader.pixels());
	else
		glTexture_->texImage2D(0, texFormat.internalFormat(), texLoader.width(), texLoader.height(), texFormat.format(), texFormat.type(), texLoader.pixels());
}
