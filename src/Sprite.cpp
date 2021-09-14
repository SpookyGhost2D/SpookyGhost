#include <stddef.h> // for offsetof()
#include "Sprite.h"
#include "Texture.h"
#include "RenderingResources.h"
#include "AnimationManager.h"
#include "singletons.h"
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
    : name(MaxNameLength), visible(true), x(0.0f), y(0.0f), rotation(0.0f), scaleFactor(1.0f, 1.0f),
      anchorPoint(0.0f, 0.0f), color(nc::Colorf::White), visited(false), gridAnchorPoint(0.0f, 0.0f),
      width_(0), height_(0), localMatrix_(nc::Matrix4x4f::Identity), worldMatrix_(nc::Matrix4x4f::Identity),
      absPosition_(0.0f, 0.0f), absScaleFactor_(1.0f, 1.0f), absRotation_(0.0f), absColor_(nc::Colorf::White),
      texture_(nullptr), texRect_(0, 0, 0, 0), flippingTexRect_(0, 0, 0, 0),
      flippedX_(false), flippedY_(false),
      rgbBlendingPreset_(BlendingPreset::ALPHA), alphaBlendingPreset_(BlendingPreset::ALPHA),
      gridAnimationsCounter_(0), interleavedVertices_(0), restPositions_(0),
      indices_(0), shortIndices_(0), parent_(nullptr), children_(4)
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
	// Move the sprite in the top-left corner
	x = texRect().w / 2;
	y = texRect().h / 2;
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<Sprite> Sprite::clone() const
{
	nctl::UniquePtr<Sprite> sprite = nctl::makeUnique<Sprite>(texture_);
	sprite->name = name;
	sprite->visible = visible;
	sprite->x = x;
	sprite->y = y;
	sprite->rotation = rotation;
	sprite->scaleFactor = scaleFactor;
	sprite->anchorPoint = anchorPoint;
	sprite->color = color;
	sprite->visited = visited;
	sprite->gridAnchorPoint = gridAnchorPoint;

	sprite->flippedX_ = flippedX_;
	sprite->flippedY_ = flippedY_;
	sprite->setTexRect(texRect_);
	sprite->setRgbBlendingPreset(rgbBlendingPreset_);
	sprite->setAlphaBlendingPreset(alphaBlendingPreset_);

	// Vertices and indices don't need to be copied

	sprite->setParent(parent_);

	return sprite;
}

void Sprite::transform()
{
	localMatrix_ = nc::Matrix4x4f::translation(x, y, 0.0f);
	localMatrix_.rotateZ(rotation);
	localMatrix_.scale(scaleFactor.x, scaleFactor.y, 1.0f);
	localMatrix_.translate(-anchorPoint.x, -anchorPoint.y, 0.0f);

	absScaleFactor_ = scaleFactor;
	absRotation_ = rotation;
	absColor_ = color;

	if (parent_)
	{
		worldMatrix_ = parent_->worldMatrix_ * localMatrix_;

		absScaleFactor_ *= parent_->absScaleFactor_;
		absRotation_ += parent_->absRotation_;
		absColor_ *= parent_->absColor_;
	}
	else
		worldMatrix_ = localMatrix_;

	absPosition_.set(worldMatrix_[3][0], worldMatrix_[3][1]);
}

void Sprite::updateRender()
{
	const float texWidth = static_cast<float>(texture_->width());
	const float texHeight = static_cast<float>(texture_->height());
	const float texScaleX = flippingTexRect_.w / texWidth;
	const float texBiasX = (flippingTexRect_.x + 0.5f) / (texWidth + 0.5f);
	const float texScaleY = flippingTexRect_.h / texHeight;
	const float texBiasY = (flippingTexRect_.y + 0.5f) / (texHeight + 0.5f);

	if (gridAnimationsCounter_ == 0)
	{
		spriteShaderUniforms_->uniform("color")->setFloatVector(absColor_.data());
		spriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
		spriteShaderUniforms_->uniform("spriteSize")->setFloatValue(width_, height_);
		spriteShaderUniforms_->uniform("projection")->setFloatVector(RenderingResources::projectionMatrix().data());
		spriteShaderUniforms_->uniform("modelView")->setFloatVector(worldMatrix_.data());
		spriteShaderUniforms_->commitUniforms();
	}
	else
	{
		meshSpriteShaderUniforms_->uniform("color")->setFloatVector(absColor_.data());
		meshSpriteShaderUniforms_->uniform("texRect")->setFloatValue(texScaleX, texBiasX, texScaleY, texBiasY);
		meshSpriteShaderUniforms_->uniform("spriteSize")->setFloatValue(width_, height_);
		meshSpriteShaderUniforms_->uniform("projection")->setFloatVector(RenderingResources::projectionMatrix().data());
		meshSpriteShaderUniforms_->uniform("modelView")->setFloatVector(worldMatrix_.data());
		meshSpriteShaderUniforms_->commitUniforms();

		const long int vboBytes = interleavedVertices_.size() * sizeof(Vertex);
		vbo_->bufferData(vboBytes, interleavedVertices_.data(), GL_STATIC_DRAW);
	}
}

void Sprite::render()
{
	texture_->bind();

	if (gridAnimationsCounter_ == 0)
	{
		spriteShaderProgram_->use();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	else
	{
		meshSpriteShaderProgram_->use();
		vbo_->bind();
		ibo_->bind();
		meshSpriteShaderAttributes_->defineVertexFormat(vbo_.get(), ibo_.get());

		if (shortIndices_.isEmpty() == false)
			glDrawElements(GL_TRIANGLE_STRIP, shortIndices_.size(), GL_UNSIGNED_SHORT, nullptr);
		else
			glDrawElements(GL_TRIANGLE_STRIP, indices_.size(), GL_UNSIGNED_INT, nullptr);

		vbo_->unbind();
		ibo_->unbind();
	}
}

void Sprite::resetGrid()
{
	ASSERT(interleavedVertices_.size() == restPositions_.size());
	for (unsigned int i = 0; i < restPositions_.size(); i++)
		interleavedVertices_[i] = restPositions_[i];
}

void Sprite::setTexture(Texture *texture)
{
	FATAL_ASSERT(texture);
	texture_ = texture;
	setTexRect(nc::Recti(0, 0, texture_->width(), texture_->height()));
}

void Sprite::setTexRect(const nc::Recti &rect)
{
	if (rect == texRect_ || rect.w == 0 || rect.h == 0)
		return;

	texRect_ = rect;
	flippingTexRect_ = texRect_;
	setSize(rect.w, rect.h);

	if (flippedX_)
	{
		flippingTexRect_.x += flippingTexRect_.w;
		flippingTexRect_.w *= -1;
	}

	if (flippedY_)
	{
		flippingTexRect_.y += flippingTexRect_.h;
		flippingTexRect_.h *= -1;
	}

	theAnimMgr->initScriptsForSprite(this);
}

void Sprite::loadTexture(const char *filename)
{
	texture_->loadFromFile(filename);
	setTexRect(nc::Recti(0, 0, texture_->width(), texture_->height()));
}

void Sprite::setFlippedX(bool flippedX)
{
	if (flippedX_ != flippedX)
	{
		flippingTexRect_.x += flippingTexRect_.w;
		flippingTexRect_.w *= -1;
		flippedX_ = flippedX;
	}
}

void Sprite::setFlippedY(bool flippedY)
{
	if (flippedY_ != flippedY)
	{
		flippingTexRect_.y += flippingTexRect_.h;
		flippingTexRect_.h *= -1;
		flippedY_ = flippedY;
	}
}

void *Sprite::imguiTexId()
{
	return reinterpret_cast<void *>(texture_->imguiTexId());
}

void Sprite::incrementGridAnimCounter()
{
	if (gridAnimationsCounter_ == 0)
		initGrid(width_, height_);
	gridAnimationsCounter_++;
}

void Sprite::decrementGridAnimCounter()
{
	gridAnimationsCounter_--;
	FATAL_ASSERT(gridAnimationsCounter_ >= 0);
	if (gridAnimationsCounter_ == 0)
	{
		interleavedVertices_.clear();
		restPositions_.clear();
		indices_.clear();
		shortIndices_.clear();
	}
}

void Sprite::setParent(Sprite *parent)
{
	if (parent == this || parent == parent_)
		return;

	if (parent_)
		parent_->removeChild(this);
	if (parent)
		parent->addChild(this);
	parent_ = parent;
}

void Sprite::setAbsPosition(float xx, float yy)
{
	if (parent_)
	{
		nc::Matrix4x4f inverseParent = nc::Matrix4x4f::scaling(1.0f / parent_->absScaleFactor_.x, 1.0f / parent_->absScaleFactor_.y, 1.0f);
		inverseParent.rotateZ(-parent_->absRotation_);
		inverseParent.translate(-parent_->absPosition_.x, -parent_->absPosition_.y, 0.0f);

		nc::Vector4f position(xx, yy, 1.0f, 1.0f);
		position = position * inverseParent;
		x = position.x;
		y = position.y;
	}
	else
	{
		x = xx;
		y = yy;
	}
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

	if (gridAnimationsCounter_ > 0)
		initGrid(width, height);
}

void Sprite::initGrid(int width, int height)
{
	const unsigned int verticesCapacity = (width + 1) * (height + 1);
	if (interleavedVertices_.capacity() < verticesCapacity)
	{
		interleavedVertices_.setCapacity(verticesCapacity);
		restPositions_.setCapacity(verticesCapacity);
		const long int vboBytes = interleavedVertices_.size() * sizeof(Vertex);
		vbo_->bufferData(vboBytes, nullptr, GL_STATIC_DRAW);
	}

	// Upper bound for number of indices
	const unsigned int indicesCapacity = (width + 2) * height * 2;
	if (indices_.capacity() < indicesCapacity)
		indices_.setCapacity(indicesCapacity);

	resetVertices();
	ASSERT(interleavedVertices_.capacity() >= verticesCapacity);
	resetIndices();
	ASSERT(indices_.capacity() >= indicesCapacity);

	const unsigned int indicesSize = indices_.size();
	const long int iboBytes = (indicesSize < 65536)
	                              ? indicesSize * sizeof(unsigned short)
	                              : indicesSize * sizeof(unsigned int);
	if (indicesSize < 65536)
	{
		shortIndices_.clear();
		if (shortIndices_.capacity() < indicesCapacity)
			shortIndices_.setCapacity(indicesCapacity);
		for (unsigned int i = 0; i < indicesSize; i++)
			shortIndices_.pushBack(static_cast<unsigned short>(indices_[i]));
		ibo_->bufferData(iboBytes, shortIndices_.data(), GL_STATIC_DRAW);
	}
	else
		ibo_->bufferData(iboBytes, indices_.data(), GL_STATIC_DRAW);
}

void Sprite::resetVertices()
{
	interleavedVertices_.clear();
	restPositions_.clear();
	const float deltaX = 1.0f / static_cast<float>(width_);
	const float deltaY = 1.0f / static_cast<float>(height_);

	for (int y = 0; y < height_ + 1; y++)
	{
		for (int x = 0; x < width_ + 1; x++)
		{
			Vertex v;
			v.x = -0.5f + static_cast<float>(x) * deltaX;
			v.y = -0.5f + static_cast<float>(y) * deltaY;
			v.u = static_cast<float>(x) * deltaX;
			v.v = static_cast<float>(y) * deltaY;
			interleavedVertices_.pushBack(v);
			restPositions_.pushBack(v);
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
	const unsigned int gridWidth = static_cast<unsigned int>(width_ + 1);

	unsigned int vertexIndex = gridWidth;
	for (unsigned int i = 0; i < height_; i++)
	{
		for (unsigned int j = 0; j < gridWidth; j++)
		{
			indices_.pushBack(vertexIndex + j);
			if (j == 0 && i != 0) // degenerate vertex
				indices_.pushBack(vertexIndex + j);
			indices_.pushBack(vertexIndex + j - gridWidth);
		}
		if (i != gridWidth - 2) // degenerate vertex
			indices_.pushBack(indices_.back());
		vertexIndex += gridWidth;
	}

#if 0
	nctl::String indicesString(1024);
	for (unsigned int i = 0; i < indices_.size(); i++)
		indicesString.formatAppend(" %u,", indices_[i]);
	LOGE_X("Indices:%s", indicesString.data());
#endif
}

bool Sprite::addChild(Sprite *sprite)
{
	if (sprite == this || sprite == nullptr)
		return false;

	children_.pushBack(sprite);
	return true;
}

bool Sprite::removeChild(Sprite *sprite)
{
	if (sprite == this || sprite == nullptr || children_.isEmpty() ||
	    (sprite && sprite->parent() != this))
		return false;

	int index = -1;
	for (unsigned int i = 0; i < children_.size(); i++)
	{
		if (children_[i] == sprite)
		{
			index = i;
			break;
		}
	}

	if (index > -1)
	{
		children_.removeAt(index);
		return true;
	}
	else
		return false;
}
