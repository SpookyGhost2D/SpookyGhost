#ifndef CLASS_TEXTURESWINDOW
#define CLASS_TEXTURESWINDOW

#include <nctl/UniquePtr.h>

#ifdef __EMSCRIPTEN__
	#include <ncine/EmscriptenLocalFile.h>
	namespace nc = ncine;
#endif

class UserInterface;
class Texture;
class SpriteGroup;

/// The textures window class
class TexturesWindow
{
  public:
	explicit TexturesWindow(UserInterface &ui);

	void create();

	bool loadTexture(const char *filename);
	bool reloadTexture(const char *filename);
#ifdef __EMSCRIPTEN__
	bool loadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
	bool reloadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize);
#endif

  private:
	UserInterface &ui_;

#ifdef __EMSCRIPTEN__
	nc::EmscriptenLocalFile loadTextureLocalFile_;
	nc::EmscriptenLocalFile reloadTextureLocalFile_;
#endif

	bool postLoadTexture(nctl::UniquePtr<Texture> &texture, const char *name);

	void recursiveRemoveSpriteWithTexture(SpriteGroup &group, Texture &tex);
	void removeTexture();
};

#endif
