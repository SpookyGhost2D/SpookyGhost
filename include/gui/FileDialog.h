#ifndef CLASS_FILEDIALOG
#define CLASS_FILEDIALOG

#include <nctl/String.h>
#include <ncine/FileSystem.h>

namespace nc = ncine;

class FileDialog
{
  public:
	enum class Sorting
	{
		NAME_ASC,
		NAME_DESC,
		SIZE_ASC,
		SIZE_DESC,
		DATE_ASC,
		DATE_DESC
	};

	enum class SelectionType
	{
		DIRECTORY,
		FILE,
		NEW_FILE
	};

	enum class Action
	{
		OPEN_PROJECT,
		SAVE_PROJECT,
		LOAD_TEXTURE,
		RELOAD_TEXTURE,
		LOAD_SCRIPT,
		RENDER_DIR
	};

	struct Config
	{
		nctl::String directory = nctl::String(nc::fs::MaxPathLength);
		bool windowOpen = false;
		bool modalPopup = true;
		const char *windowIcon = "";
		const char *windowTitle = "File Browser";
		const char *okButton = "OK";

		bool showControls = true;
		bool showHidden = false;
		bool showSize = true;
		bool showDate = true;
#ifdef _WIN32
		bool showPermissions = false;
#else
		bool showPermissions = true;
#endif
		Sorting sorting = Sorting::NAME_ASC;

		bool sortDirectoriesfirst = true;
		SelectionType selectionType = SelectionType::FILE;
		const char *extensions = nullptr;

		bool colors = true;
		unsigned int fileColor = IM_COL32(212, 213, 213, 255);
		unsigned int dirColor = IM_COL32(61, 174, 233, 255);
		unsigned int exeColor = IM_COL32(28, 215, 151, 255);

		Action action = Action::OPEN_PROJECT;
		bool allowOverwrite = false;
	};

	static Config config;
	static bool create(Config &config, nctl::String &selection);

  private:
	struct DirEntry
	{
		nctl::String name = nctl::String(nc::fs::MaxPathLength);
		bool isHidden = false;
		long int size = 0;
		nc::fs::FileDate date = {};
		int permissions = 0;
		bool isDirectory = false;
	};
};

#endif
