#include <stddef.h> // for offsetof()
#include "Sprite.h"
#include "Texture.h"
#include "RenderResources.h"
#include <ncine/Matrix4x4.h>
#include <ncine/RenderResources.h>
#include <ncine/GLShaderProgram.h>
#include <ncine/GLShaderUniforms.h>
#include <ncine/GLShaderAttributes.h>

#include "shader_strings.h"

namespace {

struct VertexFormat
{
	GLfloat position[2];
	GLfloat texcoords[2];
};

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Sprite::Sprite(Texture *texture)
    : rotation(0.0f), anchorPoint(0.0f, 0.0f), scaleFactor(1.0f, 1.0f),
      modelView_(nc::Matrix4x4f::Identity), texture_(nullptr),
      texRect_(0, 0, 0, 0), flippedX_(false), flippedY_(false), //color_(nc::Colorf::White)
      interleavedVertices_(0), indices_(0)
{
	spriteShaderProgram_ = RenderResources::spriteShaderProgram();
	spriteShaderUniforms_ = nctl::makeUnique<nc::GLShaderUniforms>(spriteShaderProgram_);
	spriteShaderUniforms_->setUniformsDataPointer(uniformsBuffer_);
	spriteShaderUniforms_->uniform("uTexture")->setIntValue(0);
	spriteShaderAttributes_ = nctl::makeUnique<nc::GLShaderAttributes>(spriteShaderProgram_);

	FATAL_ASSERT(UniformsBufferSize >= spriteShaderProgram_->uniformsSize());

	meshSpriteShaderProgram_ = RenderResources::meshSpriteShaderProgram();
	meshSpriteShaderUniforms_ = nctl::makeUnique<nc::GLShaderUniforms>(meshSpriteShaderProgram_);
	meshSpriteShaderUniforms_->setUniformsDataPointer(uniformsBuffer_);
	meshSpriteShaderUniforms_->uniform("uTexture")->setIntValue(0);
	meshSpriteShaderAttributes_ = nctl::makeUnique<nc::GLShaderAttributes>(meshSpriteShaderProgram_);
	meshSpriteShaderAttributes_->attribute("aPosition")->setVboParameters(sizeof(VertexFormat), reinterpret_cast<void *>(offsetof(VertexFormat, position)));
	meshSpriteShaderAttributes_->attribute("aTexCoords")->setVboParameters(sizeof(VertexFormat), reinterpret_cast<void *>(offsetof(VertexFormat, texcoords)));

	FATAL_ASSERT(UniformsBufferSize >= spriteShaderProgram_->uniformsSize());

	vbo_ = nctl::makeUnique<nc::GLBufferObject>(GL_ARRAY_BUFFER);
	ibo_ = nctl::makeUnique<nc::GLBufferObject>(GL_ELEMENT_ARRAY_BUFFER);

	setTexture(texture);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Sprite::transform()
{
	modelView_ = nc::Matrix4x4f::translation(x, y, 0.0f);
	modelView_.rotateZ(rotation);
	modelView_.scale(scaleFactor.x, scaleFactor.y, 1.0f);
	modelView_.translate(-anchorPoint.x, -anchorPoint.y, 0.0f);
}

void Sprite::updateRender()
{
	const nc::Vector2i texSize = texture_->size();
	const float texScaleX = texRect_.w / float(texSize.x);
	const float texBiasX = texRect_.x / float(texSize.x);
	const float texScaleY = texRect_.h / float(texSize.y);
	const float texBiasY = texRect_.y / float(texSize.y);

	spriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
	spriteShaderUniforms_->uniform("spriteSize")->setFloatValue(texSize.x, texSize.y);
	spriteShaderUniforms_->uniform("projection")->setFloatVector(RenderResources::projectionMatrix().data());
	spriteShaderUniforms_->uniform("modelView")->setFloatVector(modelView_.data());
	spriteShaderUniforms_->commitUniforms();

	meshSpriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
	meshSpriteShaderUniforms_->uniform("spriteSize")->setFloatValue(texSize.x, texSize.y);
	meshSpriteShaderUniforms_->uniform("projection")->setFloatVector(RenderResources::projectionMatrix().data());
	meshSpriteShaderUniforms_->uniform("modelView")->setFloatVector(modelView_.data());
	meshSpriteShaderUniforms_->commitUniforms();

	const long int vboBytes = interleavedVertices_.size() * sizeof(Vertex);
	vbo_->bufferData(vboBytes, interleavedVertices_.data(), GL_STATIC_DRAW);
	const long int iboBytes = indices_.size() * sizeof(GLushort);
	ibo_->bufferData(iboBytes, indices_.data(), GL_STATIC_DRAW);
}

void Sprite::render()
{
	texture_->bind();
#if 0
	meshSpriteShaderProgram_->use();
	vbo_->bind();
	ibo_->bind();
	meshSpriteShaderAttributes_->defineVertexFormat(vbo_.get(), ibo_.get());
	glDrawElements(GL_TRIANGLE_STRIP, indices_.size(), GL_UNSIGNED_SHORT, nullptr);
	vbo_->unbind();
	ibo_->unbind();
#else
	spriteShaderProgram_->use();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}

void Sprite::setTexture(Texture *texture)
{
	FATAL_ASSERT(texture);
	texture_ = texture;
	texRect_ = nc::Recti(0, 0, texture->width(), texture->height());
	const unsigned int verticesCapacity = (texture->width() + 1) * (texture->height() + 1) + 1;
	interleavedVertices_.setCapacity(verticesCapacity);
	const unsigned int indicesCapacity = (texture->width() + 1) * (texture->height() + 1) * 2;
	indices_.setCapacity(indicesCapacity);

	const long int vboBytes = interleavedVertices_.capacity() * sizeof(Vertex);
	vbo_->bufferData(vboBytes, nullptr, GL_STATIC_DRAW);
	const long int iboBytes = indices_.capacity() * sizeof(unsigned short);
	ibo_->bufferData(iboBytes, nullptr, GL_STATIC_DRAW);

	resetVertices();
	ASSERT(interleavedVertices_.capacity() == verticesCapacity);
	resetIndices();
	ASSERT(indices_.capacity() == indicesCapacity);
}

void Sprite::setTexRect(const nc::Recti &rect)
{
	texRect_ = rect;
	//setSize(static_cast<float>(rect.w), static_cast<float>(rect.h));

	if (flippedX_)
	{
		texRect_.x += texRect_.w;
		texRect_.w *= -1;
	}

	if (flippedY_)
	{
		texRect_.y += texRect_.h;
		texRect_.h *= -1;
	}
}

void Sprite::setFlippedX(bool flippedX)
{
	if (flippedX_ != flippedX)
	{
		texRect_.x += texRect_.w;
		texRect_.w *= -1;
		flippedX_ = flippedX;
	}
}

void Sprite::setFlippedY(bool flippedY)
{
	if (flippedY_ != flippedY)
	{
		texRect_.y += texRect_.h;
		texRect_.h *= -1;
		flippedY_ = flippedY;
	}
}

void Sprite::testAnim(float value)
{
	const int width = texture_->width();
	const int height = texture_->height();
	const float deltaX = 1.0f / static_cast<float>(width);
	const float deltaY = 1.0f / static_cast<float>(height);

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Vertex &v = interleavedVertices_[index];
			v.x = -0.5f + static_cast<float>(x) * deltaX;
			v.y = -0.5f + static_cast<float>(y) * deltaY + 0.25f * sin(value + 2 * x * deltaX);
		}
	}
}

void *Sprite::imguiTexId()
{
	return reinterpret_cast<void *>(texture_);
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void Sprite::resetVertices()
{
	interleavedVertices_.clear();
	const int width = texture_->width();
	const int height = texture_->height();
	const float deltaX = 1.0f / static_cast<float>(width);
	const float deltaY = 1.0f / static_cast<float>(height);

	for (int y = 0; y < height + 1; y++)
	{
		for (int x = 0; x < width + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width + 1));
			Vertex &v = interleavedVertices_[index];
			v.x = -0.5f + static_cast<float>(x) * deltaX;
			v.y = -0.5f + static_cast<float>(y) * deltaY;
			v.u = static_cast<float>(x) * deltaX;
			v.v = static_cast<float>(y) * deltaY;
		}
	}

#if 0
	nctl::String verticesString(1024 * 10);
	for (unsigned int i = 0; i < interleavedVertices_.size(); i++)
		verticesString.formatAppend("#%u <%.2f, %.2f>\n", i, interleavedVertices_[i].x, interleavedVertices_[i].y);
	LOGE_X("Vertices:\n%s", verticesString.data());
#endif
}

void Sprite::resetIndices()
{
	indices_.clear();
	const int width = texture_->width() + 1;
	const int height = texture_->height() + 1;

	unsigned short vertexIndex = width;
	unsigned int arrayIndex = 0;
	for (unsigned short i = 0; i < height - 1; i++)
	{
		for (unsigned short j = 0; j < width; j++)
		{
			indices_[arrayIndex++] = vertexIndex + j;
			if (j == 0 && i != 0) // degenerate vertex
				indices_[arrayIndex++] = vertexIndex + j;
			indices_[arrayIndex++] = vertexIndex + j - width;
		}
		if (i != width - 2) // degenerate vertex
		{
			indices_[arrayIndex] = indices_[arrayIndex - 1];
			arrayIndex++;
		}
		vertexIndex += width;
	}

#if 0
	nctl::String indicesString(1024);
	for (unsigned int i = 0; i < indices_.size(); i++)
		indicesString.formatAppend(" %u,", indices_[i]);
	LOGE_X("Indices:%s", indicesString.data());
#endif
}

