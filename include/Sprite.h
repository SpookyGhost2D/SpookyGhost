#ifndef CLASS_SPRITE
#define CLASS_SPRITE

#include <nctl/Array.h>
#include <ncine/Rect.h>
#include <ncine/Matrix4x4.h>
#include <ncine/Colorf.h>

namespace ncine {

class GLShaderProgram;
class GLShaderUniforms;
class GLShaderAttributes;
class GLBufferObject;

}

namespace nc = ncine;

class Texture;

/// The sprite to animate
class Sprite
{
  public:
	struct Vertex
	{
		float x, y;
		float u, v;
	};

	float x, y;
	float rotation;
	nc::Vector2f anchorPoint;
	nc::Vector2f scaleFactor;

	Sprite(Texture *texture);

	void transform();
	void updateRender();
	void render();

	void setTexture(Texture *texture);

	void testAnim(float value);

  private:
	nc::Matrix4x4f modelView_;
	Texture *texture_;
	nc::Recti texRect_;

	//nc::Colorf color_;

	//bool flippedX_;
	//bool flippedY_;

	nctl::Array<Vertex> interleavedVertices_;
	nctl::Array<unsigned short> indices_;

	static const int UniformsBufferSize = 256;
	unsigned char uniformsBuffer_[UniformsBufferSize];

	nc::GLShaderProgram *spriteShaderProgram_;
	nctl::UniquePtr<nc::GLShaderUniforms> spriteShaderUniforms_;
	nctl::UniquePtr<nc::GLShaderAttributes> spriteShaderAttributes_;

	nc::GLShaderProgram *meshSpriteShaderProgram_;
	nctl::UniquePtr<nc::GLShaderUniforms> meshSpriteShaderUniforms_;
	nctl::UniquePtr<nc::GLShaderAttributes> meshSpriteShaderAttributes_;

	nctl::UniquePtr<nc::GLBufferObject> vbo_;
	nctl::UniquePtr<nc::GLBufferObject> ibo_;

	void resetVertices();
	void resetIndices();
};

#endif
