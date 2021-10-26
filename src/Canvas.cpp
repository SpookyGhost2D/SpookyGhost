#include <ncine/config.h>

#include "Canvas.h"
#include "RenderingResources.h"
#include <ncine/Application.h>
#include <ncine/GLTexture.h>
#include <ncine/GLFramebufferObject.h>
#include <ncine/TextureSaverPng.h>

#include "shader_strings.h"

#ifdef __MINGW32__
	#undef ERROR
#endif

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Canvas::Canvas()
    : backgroundColor(0.0f, 0.0f, 0.0f, 0.0f),
      texWidth_(0), texHeight_(0), texSizeInBytes_(0)
{
	const nc::IGfxCapabilities &gfxCaps = nc::theServiceLocator().gfxCapabilities();
	maxTextureSize_ = gfxCaps.value(nc::IGfxCapabilities::GLIntValues::MAX_TEXTURE_SIZE);
}

Canvas::Canvas(int texWidth, int texHeight)
    : backgroundColor(0.0f, 0.0f, 0.0f, 0.0f),
      texWidth_(texWidth), texHeight_(texHeight), texSizeInBytes_(0)
{
	const nc::IGfxCapabilities &gfxCaps = nc::theServiceLocator().gfxCapabilities();
	maxTextureSize_ = gfxCaps.value(nc::IGfxCapabilities::GLIntValues::MAX_TEXTURE_SIZE);

	resizeTexture(texWidth, texHeight);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Canvas::resizeTexture(int width, int height)
{
	FATAL_ASSERT(width > 0);
	FATAL_ASSERT(height > 0);
	if (pixels_ == nullptr || width != texWidth_ || height != texHeight_)
	{
		texWidth_ = (width <= maxTextureSize_) ? width : maxTextureSize_;
		texHeight_ = (height <= maxTextureSize_) ? height : maxTextureSize_;
		texSizeInBytes_ = static_cast<unsigned int>(texWidth_ * texHeight_ * 4);

		pixels_ = nctl::makeUnique<unsigned char[]>(texSizeInBytes_);

		texture_ = nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D);
		texture_->texStorage2D(1, GL_RGBA8, width, height);
		texture_->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		texture_->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		fbo_ = nctl::makeUnique<nc::GLFramebufferObject>();
		fbo_->attachTexture(*texture_, GL_COLOR_ATTACHMENT0);
		if (fbo_->isStatusComplete() == false)
			LOGE("Framebuffer object status is not complete\n");
	}
}

void Canvas::bindTexture()
{
	texture_->bind();
}

void Canvas::unbindTexture()
{
	texture_->unbind();
}

void Canvas::bindRead()
{
	fbo_->bind(GL_READ_FRAMEBUFFER);
}

void Canvas::bindDraw()
{
	fbo_->bind(GL_DRAW_FRAMEBUFFER);
}

void Canvas::bind()
{
	RenderingResources::setCanvasSize(texWidth_, texHeight_);
	glViewport(0, 0, texWidth_, texHeight_);
	fbo_->bind(GL_FRAMEBUFFER);
	glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Canvas::unbind()
{
	fbo_->unbind();
	glViewport(0, 0, nc::theApplication().widthInt(), nc::theApplication().heightInt());
}

void Canvas::save(const char *filename)
{
#if !defined(NCINE_WITH_OPENGLES) && !defined(__EMSCRIPTEN__)
	fbo_->unbind();
	texture_->getTexImage(0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_.get());
#else
	fbo_->bind(GL_READ_FRAMEBUFFER);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, texWidth_, texHeight_, GL_RGBA, GL_UNSIGNED_BYTE, pixels_.get());
	fbo_->unbind();
#endif

	nc::TextureSaverPng saver;
	nc::ITextureSaver::Properties props;
	props.width = texWidth_;
	props.height = texHeight_;
	props.pixels = pixels_.get();
	props.format = nc::ITextureSaver::Format::RGBA8;
	saver.saveToFile(props, filename);
}

void *Canvas::imguiTexId()
{
	return reinterpret_cast<void *>(texture_.get());
}
