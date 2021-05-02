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

	enum class BlendingPreset
	{
		DISABLED, ///< uses `GL_ONE` and `GL_ZERO`
		ALPHA, ///< uses `GL_SRC_ALPHA` and `GL_ONE_MINUS_SRC_ALPHA`
		PREMULTIPLIED_ALPHA, ///< uses `GL_ONE` and `GL_ONE_MINUS_SRC_ALPHA`
		ADDITIVE, ///< uses `GL_SRC_ALPHA` and `GL_ONE`
		MULTIPLY ///< uses `GL_DST_COLOR` and `GL_ZERO`
	};

	static const unsigned int MaxNameLength = 64;
	nctl::String name;

	bool visible;
	float x, y;
	float rotation;
	nc::Vector2f scaleFactor;
	nc::Vector2f anchorPoint;
	nc::Colorf color;
	/// Used by the GUI to exclude children from becoming parents of their parents
	bool visited;
	/// Anchor point used by some grid animation functions
	nc::Vector2f gridAnchorPoint;

	Sprite(Texture *texture);

	nctl::UniquePtr<Sprite> clone() const;

	void transform();
	void updateRender();
	void render();
	void resetGrid();

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

	inline BlendingPreset blendingPreset() const { return blendingPreset_; }
	inline void setBlendingPreset(BlendingPreset blendingPreset) { blendingPreset_ = blendingPreset; }

	inline const nctl::Array<Vertex> &vertexRestPositions() const { return restPositions_; }
	inline const nctl::Array<Vertex> &interleavedVertices() const { return interleavedVertices_; }
	inline nctl::Array<Vertex> &interleavedVertices() { return interleavedVertices_; }

	void *imguiTexId();

	void incrementGridAnimCounter();
	void decrementGridAnimCounter();

	inline const nctl::Array<Sprite *> children() const { return children_; }
	inline const Sprite *parent() const { return parent_; }
	inline Sprite *parent() { return parent_; }
	void setParent(Sprite *parent);

	inline int absWidth() const { return static_cast<int>(width_ * absScaleFactor_.x); }
	inline int absHeight() const { return static_cast<int>(height_ * absScaleFactor_.y); }

	inline const nc::Vector2f absPosition() const { return absPosition_; }
	inline void setAbsPosition(const nc::Vector2f position) { setAbsPosition(position.x, position.y); }
	void setAbsPosition(float xx, float yy);

	inline const nc::Vector2f &absScaleFactor() const { return absScaleFactor_; }
	inline float absRotation() const { return absRotation_; }
	inline const nc::Colorf &absColor() const { return absColor_; }

  private:
	int width_;
	int height_;

	nc::Matrix4x4f localMatrix_;
	nc::Matrix4x4f worldMatrix_;

	nc::Vector2f absPosition_;
	nc::Vector2f absScaleFactor_;
	float absRotation_;
	nc::Colorf absColor_;

	Texture *texture_;
	nc::Recti texRect_;
	/// Texture rectangle that takes flipping into account
	nc::Recti flippingTexRect_;

	bool flippedX_;
	bool flippedY_;

	BlendingPreset blendingPreset_;

	int gridAnimationsCounter_;

	nctl::Array<Vertex> interleavedVertices_;
	nctl::Array<Vertex> restPositions_;
	nctl::Array<unsigned int> indices_;
	nctl::Array<unsigned short> shortIndices_;

	Sprite *parent_;
	nctl::Array<Sprite *> children_;

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
	void initGrid(int width, int height);
	void resetVertices();
	void resetIndices();

	bool addChild(Sprite *sprite);
	bool removeChild(Sprite *sprite);
};

#endif
