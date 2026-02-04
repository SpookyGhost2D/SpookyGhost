#ifndef CLASS_SPRITEWINDOW
#define CLASS_SPRITEWINDOW

#include <nctl/Array.h>

class UserInterface;
class Sprite;

/// The sprite window class
class SpriteWindow
{
  public:
	explicit SpriteWindow(UserInterface &ui);

	void create();

  private:
	/// Used to keep track of which node can be parent of another node
	struct SpriteStruct
	{
		SpriteStruct()
		    : index(-1), sprite(nullptr) {}
		SpriteStruct(int i, Sprite *sp)
		    : index(i), sprite(sp) {}

		int index;
		Sprite *sprite;
	};

	UserInterface &ui_;

	/// Used to keep track of which node can be the parent of the selected one
	nctl::Array<SpriteStruct> spriteGraph_;

	void visitSprite(Sprite &sprite);
};

#endif
