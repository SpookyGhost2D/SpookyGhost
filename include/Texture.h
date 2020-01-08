#ifndef CLASS_TEXTURE
#define CLASS_TEXTURE

#include <nctl/UniquePtr.h>
#include <ncine/Vector2.h>

namespace ncine {

class GLTexture;
class ITextureLoader;

}

namespace nc = ncine;

/// The texture wrapper class
class Texture
{
  public:
	Texture(const char *filename);
	Texture(const char *filename, int width, int height);

	inline nc::Vector2i size() const { return nc::Vector2i(width_, height_); }
	inline int width() const { return width_; }
	inline int height() const { return height_; }

	inline unsigned int numChannels() const { return numChannels_; }
	inline unsigned int dataSize() const { return dataSize_; }

	void bind();

  private:
	nctl::UniquePtr<nc::GLTexture> glTexture_;
	int width_;
	int height_;
	unsigned int numChannels_;
	unsigned long dataSize_;

	void load(const char *filename, int width, int height);
	void load(const nc::ITextureLoader &texLoader, int width, int height);

	friend class Sprite;
};

#endif
