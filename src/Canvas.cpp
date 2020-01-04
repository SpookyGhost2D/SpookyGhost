#include "Canvas.h"
#include "RenderResources.h"
#include <ncine/Application.h>
#include <ncine/Matrix4x4.h>
#include <ncine/GLShaderUniforms.h>
#include <ncine/GLShaderAttributes.h>
#include <ncine/GLTexture.h>
#include <ncine/GLFramebufferObject.h>
#include <ncine/GLBufferObject.h>
#include <ncine/TextureSaverPng.h>

#include "shader_strings.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Canvas::Canvas()
    : texWidth_(0), texHeight_(0), texSizeInBytes_(0), mapPtr_(nullptr)
{
	texProgram_ = RenderResources::textureShaderProgram();

	texUniforms_ = nctl::makeUnique<nc::GLShaderUniforms>(texProgram_);
	texUniforms_->setUniformsDataPointer(uniformsBuffer_);
	texUniforms_->uniform("uTexture")->setIntValue(0);
	texAttributes_ = nctl::makeUnique<nc::GLShaderAttributes>(texProgram_);

	FATAL_ASSERT(UniformsBufferSize >= texProgram_->uniformsSize());

	pbo_ = nctl::makeUnique<nc::GLBufferObject>(GL_PIXEL_UNPACK_BUFFER);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Canvas::initTexture(int width, int height)
{
	resizeTexture(width, height);

	texUniforms_->uniform("size")->setFloatValue(width, height);

	nc::Matrix4x4f projection = nc::Matrix4x4f::ortho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
	texUniforms_->uniform("projection")->setFloatVector(projection.data());
	nc::Matrix4x4f modelView = nc::Matrix4x4f::translation(width * 0.5f, height * 0.5f, 0.0f);
	texUniforms_->uniform("modelView")->setFloatVector(modelView.data());
	texUniforms_->commitUniforms();
}

void Canvas::resizeTexture(int width, int height)
{
	FATAL_ASSERT(width > 0);
	FATAL_ASSERT(height > 0);
	if (pixels_ == nullptr || width != texWidth_ || height != texHeight_)
	{
		texWidth_ = width;
		texHeight_ = height;
		texSizeInBytes_ = static_cast<unsigned int>(texWidth_ * texHeight_ * 4);

		pixels_ = nctl::makeUnique<unsigned char[]>(texSizeInBytes_);
		pbo_->bufferData(texSizeInBytes_, nullptr, GL_STREAM_DRAW);
		pbo_->unbind();

		texture_ = nctl::makeUnique<nc::GLTexture>(GL_TEXTURE_2D);
		texture_->texStorage2D(1, GL_RGBA8, width, height);
		texture_->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		texture_->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		fbo_ = nctl::makeUnique<nc::GLFramebufferObject>();
		fbo_->attachTexture(*texture_, GL_COLOR_ATTACHMENT0);
		fbo_->attachRenderbuffer(GL_DEPTH_COMPONENT16, width, height, GL_DEPTH_ATTACHMENT);
		if (fbo_->isStatusComplete() == false)
			LOGE("Framebuffer object status is not complete\n");
	}
}

void Canvas::progressiveUpdate()
{
	FATAL_ASSERT(config_.textureUploadMode >= 0);
	FATAL_ASSERT(config_.textureUploadMode < TextureUploadModes::COUNT);

	//glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (config_.textureUploadMode == TextureUploadModes::PBO_MAPPING)
	{
		FATAL_ASSERT(mapPtr_ == nullptr);
		mapPtr_ = static_cast<GLubyte *>(pbo_->mapBufferRange(0, texSizeInBytes_, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
	}

	texProgram_->use();
	texture_->bind();
	if (config_.textureUploadMode == TextureUploadModes::TEXSUBIMAGE)
	{
		pbo_->unbind();
		texture_->texSubImage2D(0, 0, 0, texWidth_, texHeight_, GL_RGB, GL_UNSIGNED_BYTE, pixels_.get());
	}
	else if (config_.textureUploadMode == TextureUploadModes::PBO)
	{
		pbo_->bufferData(texSizeInBytes_, nullptr, GL_STREAM_DRAW); // Orphaning
		pbo_->bufferSubData(0, texSizeInBytes_, pixels_.get());
		texture_->texSubImage2D(0, 0, 0, texWidth_, texHeight_, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	else if (config_.textureUploadMode == TextureUploadModes::PBO_MAPPING)
	{
		FATAL_ASSERT(mapPtr_);
		pbo_->flushMappedBufferRange(0, texSizeInBytes_);
		pbo_->unmap();
		mapPtr_ = nullptr;

		texture_->texSubImage2D(0, 0, 0, texWidth_, texHeight_, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::bind()
{
	RenderResources::setCanvasSize(texWidth_, texHeight_);
	glViewport(0, 0, texWidth_, texHeight_);
	fbo_->bind(GL_FRAMEBUFFER);
	//glClearColor(0.2f, 0.2f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Canvas::unbind()
{
	GLenum invalidAttachment = GL_DEPTH_ATTACHMENT;
	fbo_->invalidate(1, &invalidAttachment);
	fbo_->unbind();
	glViewport(0, 0, nc::theApplication().widthInt(), nc::theApplication().heightInt());
}

void Canvas::save(const char *filename)
{
	//fbo_->bind(GL_FRAMEBUFFER);
	//glReadBuffer(GL_COLOR_ATTACHMENT0);
	//glReadPixels(0, 0, texWidth_, texHeight_, GL_RGB8, GL_UNSIGNED_BYTE, pixels_.get());
	//fbo_->unbind();

	fbo_->unbind();
	texture_->getTexImage(0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_.get());

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
