#ifndef CLASS_RENDERRESOURCES
#define CLASS_RENDERRESOURCES

#include <ncine/GLShaderProgram.h>
#include <nctl/UniquePtr.h>
#include <ncine/Matrix4x4.h>


// TODO: RENAME

namespace nc = ncine;

/// The class that creates and handles application common OpenGL rendering resources
class RenderResources
{
  public:
	static inline nc::GLShaderProgram *textureShaderProgram() { return spriteShaderProgram_.get(); }
	static inline nc::GLShaderProgram *spriteShaderProgram() { return spriteShaderProgram_.get(); }
	static inline nc::GLShaderProgram *meshSpriteShaderProgram() { return meshSpriteShaderProgram_.get(); }
	static inline const nc::Matrix4x4f &projectionMatrix() { return projectionMatrix_; }

	static inline const nc::Vector2f &canvasSize() { return canvasSize_; }
	static void setCanvasSize(int width, int height);

	static void create();
	static void dispose();

  private:
	static nctl::UniquePtr<nc::GLShaderProgram> textureShaderProgram_;
	static nctl::UniquePtr<nc::GLShaderProgram> spriteShaderProgram_;
	static nctl::UniquePtr<nc::GLShaderProgram> meshSpriteShaderProgram_;

	static nc::Vector2f canvasSize_;
	static nc::Matrix4x4f projectionMatrix_;

	/// Static class, deleted constructor
	RenderResources() = delete;
	/// Static class, deleted copy constructor
	RenderResources(const RenderResources &other) = delete;
	/// Static class, deleted assignement operator
	RenderResources &operator=(const RenderResources &other) = delete;
};

#endif
