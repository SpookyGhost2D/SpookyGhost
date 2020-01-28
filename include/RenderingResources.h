#ifndef CLASS_RENDERINGRESOURCES
#define CLASS_RENDERINGRESOURCES

#include <nctl/UniquePtr.h>
#include <ncine/Matrix4x4.h>

namespace ncine {

class GLShaderProgram;

}

namespace nc = ncine;

/// The class that creates and handles some common rendering resources
class RenderingResources
{
  public:
	static inline nc::GLShaderProgram *spriteShaderProgram() { return spriteShaderProgram_.get(); }
	static inline nc::GLShaderProgram *meshSpriteShaderProgram() { return meshSpriteShaderProgram_.get(); }
	static inline const nc::Matrix4x4f &projectionMatrix() { return projectionMatrix_; }

	static inline const nc::Vector2f &canvasSize() { return canvasSize_; }
	static void setCanvasSize(int width, int height);

	static void create();
	static void dispose();

  private:
	static nctl::UniquePtr<nc::GLShaderProgram> spriteShaderProgram_;
	static nctl::UniquePtr<nc::GLShaderProgram> meshSpriteShaderProgram_;

	static nc::Vector2f canvasSize_;
	static nc::Matrix4x4f projectionMatrix_;

	/// Static class, deleted constructor
	RenderingResources() = delete;
	/// Static class, deleted copy constructor
	RenderingResources(const RenderingResources &other) = delete;
	/// Static class, deleted assignement operator
	RenderingResources &operator=(const RenderingResources &other) = delete;
};

#endif
