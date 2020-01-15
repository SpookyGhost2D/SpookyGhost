#include <stddef.h> // for offsetof()
#include "Sprite.h"
#include "Texture.h"
#include "RenderingResources.h"
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
    : name(MaxNameLength), rotation(0.0f), scaleFactor(1.0f, 1.0f),
      anchorPoint(0.0f, 0.0f), color(nc::Colorf::White),
      width_(0), height_(0), modelView_(nc::Matrix4x4f::Identity),
      texture_(nullptr), texRect_(0, 0, 0, 0), flippedX_(false), flippedY_(false),
      blendingPreset_(BlendingPreset::ALPHA),
      interleavedVertices_(0), restPositions_(0), indices_(0)
{
	spriteShaderProgram_ = RenderingResources::spriteShaderProgram();
	spriteShaderUniforms_ = nctl::makeUnique<nc::GLShaderUniforms>(spriteShaderProgram_);
	spriteShaderUniforms_->setUniformsDataPointer(uniformsBuffer_);
	spriteShaderUniforms_->uniform("uTexture")->setIntValue(0);
	spriteShaderAttributes_ = nctl::makeUnique<nc::GLShaderAttributes>(spriteShaderProgram_);

	FATAL_ASSERT(UniformsBufferSize >= spriteShaderProgram_->uniformsSize());

	meshSpriteShaderProgram_ = RenderingResources::meshSpriteShaderProgram();
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
	const float texWidth = static_cast<float>(texture_->width());
	const float texHeight = static_cast<float>(texture_->height());
	const float texScaleX = texRect_.w / texWidth;
	const float texBiasX = texRect_.x / texWidth;
	const float texScaleY = texRect_.h / texHeight;
	const float texBiasY = texRect_.y / texHeight;

	spriteShaderUniforms_->uniform("color")->setFloatVector(color.data());
	spriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
	spriteShaderUniforms_->uniform("spriteSize")->setFloatValue(texRect_.w, texRect_.h);
	spriteShaderUniforms_->uniform("projection")->setFloatVector(RenderingResources::projectionMatrix().data());
	spriteShaderUniforms_->uniform("modelView")->setFloatVector(modelView_.data());
	spriteShaderUniforms_->commitUniforms();

	meshSpriteShaderUniforms_->uniform("color")->setFloatVector(color.data());
	meshSpriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
	meshSpriteShaderUniforms_->uniform("spriteSize")->setFloatValue(texRect_.w, texRect_.h);
	meshSpriteShaderUniforms_->uniform("projection")->setFloatVector(RenderingResources::projectionMatrix().data());
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
#if 1
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

void Sprite::resetGrid()
{
	for (unsigned int i = 0; i < restPositions_.size(); i++)
	{
		interleavedVertices_[i].x = restPositions_[i].x;
		interleavedVertices_[i].y = restPositions_[i].y;
	}
}

void Sprite::setTexture(Texture *texture)
{
	FATAL_ASSERT(texture);
	texture_ = texture;
	texRect_ = nc::Recti(0, 0, texture->width(), texture->height());
	setSize(texRect_.w, texRect_.h);
}

void Sprite::setTexRect(const nc::Recti &rect)
{
	texRect_ = rect;
	setSize(abs(rect.w), abs(rect.h));

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

void Sprite::loadTexture(const char *filename)
{
	texture_->load(filename, 0, 0);
	texRect_ = nc::Recti(0, 0, texture_->width(), texture_->height());
	setSize(texRect_.w, texRect_.h);
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

void *Sprite::imguiTexId()
{
	return reinterpret_cast<void *>(texture_->imguiTexId());
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void Sprite::setSize(int width, int height)
{
	ASSERT(width > 0 && height > 0);
	if (width == width_ && height == height_)
		return;

	width_ = width;
	height_ = height;

	const unsigned int verticesCapacity = (width + 1) * (height + 1);
	if (interleavedVertices_.capacity() < verticesCapacity)
	{
		interleavedVertices_.setCapacity(verticesCapacity);
		restPositions_.setCapacity(verticesCapacity);
		const long int vboBytes = interleavedVertices_.capacity() * sizeof(Vertex);
		vbo_->bufferData(vboBytes, nullptr, GL_STATIC_DRAW);
	}
	const unsigned int indicesCapacity = (width + 1) * (height + 1) * 2;
	if (indices_.capacity() < indicesCapacity)
	{
		indices_.setCapacity(indicesCapacity);
		const long int iboBytes = indices_.capacity() * sizeof(unsigned short);
		ibo_->bufferData(iboBytes, nullptr, GL_STATIC_DRAW);
	}

	resetVertices();
	ASSERT(interleavedVertices_.capacity() == verticesCapacity);
	resetIndices();
	ASSERT(indices_.capacity() == indicesCapacity);
}

void Sprite::resetVertices()
{
	interleavedVertices_.clear();
	const float deltaX = 1.0f / static_cast<float>(width_);
	const float deltaY = 1.0f / static_cast<float>(height_);

	for (int y = 0; y < height_ + 1; y++)
	{
		for (int x = 0; x < width_ + 1; x++)
		{
			const unsigned int index = static_cast<unsigned int>(x + y * (width_ + 1));
			Vertex &v = interleavedVertices_[index];
			VertexPosition &restPos = restPositions_[index];
			v.x = -0.5f + static_cast<float>(x) * deltaX;
			v.y = -0.5f + static_cast<float>(y) * deltaY;
			v.u = static_cast<float>(x) * deltaX;
			v.v = static_cast<float>(y) * deltaY;
			restPos.x = v.x;
			restPos.y = v.y;
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
	const int gridWidth = width_ + 1;
	const int gridHeight = height_ + 1;

	unsigned short vertexIndex = gridWidth;
	unsigned int arrayIndex = 0;
	for (unsigned short i = 0; i < gridHeight - 1; i++)
	{
		for (unsigned short j = 0; j < gridWidth; j++)
		{
			indices_[arrayIndex++] = vertexIndex + j;
			if (j == 0 && i != 0) // degenerate vertex
				indices_[arrayIndex++] = vertexIndex + j;
			indices_[arrayIndex++] = vertexIndex + j - gridWidth;
		}
		if (i != gridWidth - 2) // degenerate vertex
		{
			indices_[arrayIndex] = indices_[arrayIndex - 1];
			arrayIndex++;
		}
		vertexIndex += gridWidth;
	}

#if 0
	nctl::String indicesString(1024);
	for (unsigned int i = 0; i < indices_.size(); i++)
		indicesString.formatAppend(" %u,", indices_[i]);
	LOGE_X("Indices:%s", indicesString.data());
#endif
}
