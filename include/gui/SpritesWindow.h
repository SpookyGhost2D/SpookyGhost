#ifndef CLASS_SPRITESWINDOW
#define CLASS_SPRITESWINDOW

class UserInterface;
class SpriteGroup;
class SpriteEntry;

/// The sprites window class
class SpritesWindow
{
  public:
	explicit SpritesWindow(UserInterface &ui);

	void create();

  private:
	struct DragSpriteEntryPayload
	{
		SpriteGroup &parent;
		unsigned int index;
	};

	UserInterface &ui_;

	void createSpriteListEntry(SpriteEntry &entry, unsigned int index);

	void cloneSprite();
	void cloneSpriteGroup();
	void removeSprite();
	void recursiveRemoveSpriteGroup(SpriteGroup &group);
	void removeSpriteGroup();
};

#endif
