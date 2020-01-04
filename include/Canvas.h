#ifndef CLASS_CANVAS
#define CLASS_CANVAS

#include <nctl/UniquePtr.h>
#include <ncine/TimeStamp.h>

namespace ncine {

class GLShaderProgram;
class GLShaderUniforms;
class GLShaderAttributes;
class GLTexture;
class GLFramebufferObject;
class GLBufferObject;

}

namespace nc = ncine;

/// The canvas texture to display the results
class Canvas
{
  public:
	struct TextureUploadModes
	{
		enum
		{
			TEXSUBIMAGE = 0,
			PBO,
			PBO_MAPPING,

			COUNT
		};
	};

	struct Configuration
	{
		bool progressiveCopy = true;
		int textureUploadMode = TextureUploadModes::TEXSUBIMAGE;
		float textureCopyDelay = 0.0f;
	};

	Canvas();

	inline const Configuration &config() const { return config_; }
	inline Configuration &config() { return config_; }

	void initTexture(int width, int height);
	void resizeTexture(int width, int height);
	void progressiveUpdate();

	void bind();
	void unbind();
	void save(const char *filename);

	inline int texWidth() const { return texWidth_; }
	inline int texHeight() const { return texHeight_; }
	inline unsigned int texSizeInBytes() const { return texSizeInBytes_; }
	inline unsigned char *texPixels() { return (mapPtr_ ? mapPtr_ : pixels_.get()); }

	/// Returns the user data opaque pointer for ImGui's ImTextureID
	void *imguiTexId();

  private:
	nc::TimeStamp lastUpdateTime_;
	float lastTextureCopy_;

	Configuration config_;
	int texWidth_;
	int texHeight_;
	unsigned int texSizeInBytes_;

	static const int UniformsBufferSize = 256;
	unsigned char uniformsBuffer_[UniformsBufferSize];

	nctl::UniquePtr<unsigned char[]> pixels_;

	nc::GLShaderProgram *texProgram_;
	nctl::UniquePtr<nc::GLShaderUniforms> texUniforms_;
	nctl::UniquePtr<nc::GLShaderAttributes> texAttributes_;
	nctl::UniquePtr<nc::GLTexture> texture_;

	nctl::UniquePtr<nc::GLFramebufferObject> fbo_;
	nctl::UniquePtr<nc::GLBufferObject> pbo_;
	unsigned char *mapPtr_;
};

#endif
