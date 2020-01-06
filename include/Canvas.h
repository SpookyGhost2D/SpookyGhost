#ifndef CLASS_CANVAS
#define CLASS_CANVAS

#include <nctl/UniquePtr.h>
#include <ncine/Vector2.h>
#include <ncine/Colorf.h>

namespace ncine {

class GLTexture;
class GLFramebufferObject;

}

namespace nc = ncine;

/// The canvas texture to display the results
class Canvas
{
  public:
	nc::Colorf backgroundColor;

	Canvas(int width, int height);

	inline void resizeTexture(const nc::Vector2i &size) { resizeTexture(size.x, size.y); }
	void resizeTexture(int width, int height);

	void bind();
	void unbind();
	void save(const char *filename);

	inline int maxTextureSize() const { return maxTextureSize_; }

	inline int texWidth() const { return texWidth_; }
	inline int texHeight() const { return texHeight_; }
	inline nc::Vector2i size() const { return nc::Vector2i(texWidth_, texHeight_); }

	inline unsigned int texSizeInBytes() const { return texSizeInBytes_; }
	inline unsigned char *texPixels() { return pixels_.get(); }

	void *imguiTexId();

  private:
	int maxTextureSize_;
	int texWidth_;
	int texHeight_;
	unsigned int texSizeInBytes_;

	nctl::UniquePtr<unsigned char[]> pixels_;
	nctl::UniquePtr<nc::GLTexture> texture_;

	nctl::UniquePtr<nc::GLFramebufferObject> fbo_;
};

#endif
