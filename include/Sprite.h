#ifndef CLASS_SPRITE
#define CLASS_SPRITE

#include <nctl/Array.h>
#include <nctl/String.h>
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

	struct VertexPosition
	{
		float x, y;
	};

	static const unsigned int MaxNameLength = 64;
	nctl::String name;

	float x, y;
	float rotation;
	nc::Vector2f anchorPoint;
	nc::Vector2f scaleFactor;

	Sprite(Texture *texture);

	void transform();
	void updateRender();
	void render();

	inline int width() const { return width_; }
	inline int height() const { return height_; }

	inline nc::Recti texRect() const { return texRect_; }
	void setTexRect(const nc::Recti &texRect);

	inline const Texture &texture() const { return *texture_; }
	inline Texture &texture() { return *texture_; }
	void setTexture(Texture *texture);

	void loadTexture(const char *filename);

	inline bool isFlippedX() const { return flippedX_; }
	void setFlippedX(bool flippedX);

	inline bool isFlippedY() const { return flippedY_; }
	void setFlippedY(bool flippedY);

	inline const nctl::Array<VertexPosition> &vertexRestPositions() const { return restPositions_; }
	inline const nctl::Array<Vertex> &interleavedVertices() const { return interleavedVertices_; }
	inline nctl::Array<Vertex> &interleavedVertices() { return interleavedVertices_; }

	void testAnim(float value); // DELETE

	void *imguiTexId();

  private:
	int width_;
	int height_;

	nc::Matrix4x4f modelView_;
	Texture *texture_;
	nc::Recti texRect_;

	//nc::Colorf color_;

	bool flippedX_;
	bool flippedY_;

	nctl::Array<Vertex> interleavedVertices_;
	nctl::Array<VertexPosition> restPositions_;
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

	void setSize(int width, int height);
	void changeSize(int widht, int height);
	void resetVertices(int widht, int height);
	void resetIndices(int widht, int height);
};

#endif
