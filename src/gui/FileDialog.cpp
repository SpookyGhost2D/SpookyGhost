#ifdef WITH_FONTAWESOME
	#include "IconsFontAwesome5.h"
#endif

#include "gui/gui_labels.h"
#include "gui/gui_common.h"
#include "gui/FileDialog.h"

#include <nctl/Array.h>
#include <nctl/algorithms.h>
#include <ncine/Application.h>

#ifdef __ANDROID__
	#include <ncine/AndroidApplication.h>
#endif

namespace {

static const char *sortingStrings[6] = {
	Labels::FileDialog_Name_Asc,
	Labels::FileDialog_Name_Desc,
	Labels::FileDialog_Size_Asc,
	Labels::FileDialog_Size_Desc,
	Labels::FileDialog_Date_Asc,
	Labels::FileDialog_Date_Desc
};

}

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

FileDialog::Config FileDialog::config;

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

bool FileDialog::create(Config &config, nctl::String &selection)
{
	if (config.windowOpen == false)
		return false;

	static nctl::String filePath(nc::fs::MaxPathLength);
	static nctl::Array<DirEntry> dirEntries(64);
	static nctl::Array<nctl::String> baseNames(64);

	static nctl::String auxString(512);
	static nctl::String inputTextString(256);
	static char permissionsString[4];
	permissionsString[3] = '\0';

	if (config.directory.isEmpty() || nc::fs::isDirectory(config.directory.data()) == false)
	{
#ifdef __ANDROID__
		config.directory.assign(nc::fs::externalStorageDir());
#else
		config.directory.assign(nc::fs::currentDir());
#endif
	}
	else
		config.directory.assign(nc::fs::absolutePath(config.directory.data()));

	ImGui::SetNextWindowSize(ImVec2(550.0f, 350.0f), ImGuiCond_Once);
	auxString.format("%s%s", config.windowIcon, config.windowTitle);
	if (config.modalPopup)
	{
		ImGui::OpenPopup(auxString.data());
		ImGui::BeginPopupModal(auxString.data());
	}
	else
		ImGui::Begin(auxString.data(), &config.windowOpen, ImGuiWindowFlags_NoDocking);

	if (config.showControls)
	{
#ifndef _WIN32
		ImGui::Checkbox("Permissions", &config.showPermissions);
		ImGui::SameLine();
#endif
		ImGui::Checkbox("Size", &config.showSize);
		ImGui::SameLine();
		ImGui::Checkbox("Date", &config.showDate);
		ImGui::SameLine();
		ImGui::Checkbox("Show Hidden", &config.showHidden);
		ImGui::SameLine();
		static int currentComboSortingType = 0;
		ImGui::PushItemWidth(100.0f);
		ImGui::Combo(Labels::FileDialog_Sorting, &currentComboSortingType, sortingStrings, IM_ARRAYSIZE(sortingStrings));
		ImGui::PopItemWidth();
		config.sorting = static_cast<Sorting>(currentComboSortingType);
	}

	if (nc::fs::logicalDrives())
	{
		char driveLetter[3] = "C:";
		static int currentComboLogicalDrive = 0;
		const char *logicalDriveStrings = nc::fs::logicalDriveStrings();
		ImGui::PushItemWidth(50.0f);
		if (ImGui::Combo("##LogicalDrives", &currentComboLogicalDrive, logicalDriveStrings, IM_ARRAYSIZE(logicalDriveStrings)))
		{
			for (int i = 0; i < currentComboLogicalDrive; i++)
				logicalDriveStrings += strnlen(logicalDriveStrings, 4) + 1;
			if (driveLetter[0] != '\0')
			{
				driveLetter[0] = logicalDriveStrings[0];
				config.directory = driveLetter;
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}

	nctl::String dirName = config.directory;
	baseNames.clear();
	while (dirName != nc::fs::dirName(dirName.data()))
	{
		if (nc::fs::baseName(dirName.data()).isEmpty() == false)
			baseNames.pushBack(nc::fs::baseName(dirName.data()));
		dirName = nc::fs::dirName(dirName.data());
	}
	baseNames.pushBack(dirName);

	dirName.clear();
	for (int i = baseNames.size() - 1; i >= 0; i--)
	{
		dirName = nc::fs::joinPath(dirName.data(), baseNames[i].data());
		if (ImGui::Button(baseNames[i].data()))
		{
			config.directory = dirName;
			if (config.selectionType != SelectionType::NEW_FILE)
				inputTextString.clear();
		}
		if (i > 0)
			ImGui::SameLine();
	}

	ImGui::BeginChild("File View", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 4.0f));

	nc::fs::Directory dir(config.directory.data());
	dirEntries.clear();
	while (const char *entryName = dir.readNext())
	{
		DirEntry entry;
		filePath = nc::fs::joinPath(config.directory, entryName);
		entry.name = entryName;
		entry.isHidden = nc::fs::isHidden(filePath.data());
		entry.size = nc::fs::fileSize(filePath.data());
		entry.date = nc::fs::lastModificationTime(filePath.data());
		entry.permissions = nc::fs::permissions(filePath.data());
		entry.isDirectory = nc::fs::isDirectory(filePath.data());

		if (config.extensions == nullptr || entry.isDirectory)
			dirEntries.pushBack(entry);
		else
		{
			const char *extension = config.extensions;
			while (extension != nullptr && *extension != '\0')
			{
				if (nc::fs::hasExtension(entry.name.data(), extension))
				{
					dirEntries.pushBack(entry);
					extension = nullptr;
				}
				else
					extension += strnlen(extension, 4) + 1;
			}
		}
	}
	dir.close();

	nctl::quicksort(dirEntries.begin(), dirEntries.end(), [&config](const DirEntry &entry1, const DirEntry &entry2)
	{
		if (config.sortDirectoriesfirst)
		{
			if (entry1.isDirectory == false && entry2.isDirectory)
				return false;
			else if (entry2.isDirectory == false && entry1.isDirectory)
				return true;
		}

		switch (config.sorting)
		{
			case Sorting::NAME_ASC:
				return (entry1.name <= entry2.name);
			case Sorting::NAME_DESC:
				return (entry1.name > entry2.name);
			case Sorting::SIZE_ASC:
				return (entry1.size <= entry2.size);
			case Sorting::SIZE_DESC:
				return (entry1.size > entry2.size);
			case Sorting::DATE_ASC:
				if (entry1.date.year != entry2.date.year)
					return entry1.date.year <= entry2.date.year;
				else if (entry1.date.month != entry2.date.month)
					return entry1.date.month <= entry2.date.month;
				else if (entry1.date.day != entry2.date.day)
					return entry1.date.day <= entry2.date.day;
				else if (entry1.date.hour != entry2.date.hour)
					return entry1.date.hour <= entry2.date.hour;
				else if (entry1.date.minute != entry2.date.minute)
					return entry1.date.minute <= entry2.date.minute;
				else
					return entry1.date.second <= entry2.date.second;
			case Sorting::DATE_DESC:
				if (entry1.date.year != entry2.date.year)
					return entry1.date.year > entry2.date.year;
				else if (entry1.date.month != entry2.date.month)
					return entry1.date.month > entry2.date.month;
				else if (entry1.date.day != entry2.date.day)
					return entry1.date.day > entry2.date.day;
				else if (entry1.date.hour != entry2.date.hour)
					return entry1.date.hour > entry2.date.hour;
				else if (entry1.date.minute != entry2.date.minute)
					return entry1.date.minute > entry2.date.minute;
				else
					return entry1.date.second > entry2.date.second;
		}

		return (entry1.name <= entry2.name);
	});

	bool selected = false;
	for (unsigned int i = 0; i < dirEntries.size(); i++)
	{
		const DirEntry &entry = dirEntries[i];
		if (entry.isHidden && config.showHidden == false)
			continue;

		auxString.clear();
		auxString.formatAppend("%s", entry.isDirectory ? Labels::FileDialog_FolderIcon : Labels::FileDialog_FileIcon);
		if (config.showPermissions)
		{
			permissionsString[0] = (entry.permissions & nc::fs::Permission::READ) ? 'r' : '-';
			permissionsString[1] = (entry.permissions & nc::fs::Permission::WRITE) ? 'w' : '-';
			permissionsString[2] = (entry.permissions & nc::fs::Permission::EXECUTE) ? 'x' : '-';
			auxString.formatAppend(" %s", permissionsString);
		}
		if (config.showSize)
			auxString.formatAppend(" %8ld", entry.size);
		if (config.showDate)
		{
			auxString.formatAppend(" %.2d/%.2d/%.4d %.2d:%.2d", entry.date.day, entry.date.month,
			                       entry.date.year, entry.date.hour, entry.date.minute);
		}
		auxString.formatAppend(" %s", entry.name.data());

		ImU32 color = config.fileColor;
		if (entry.isDirectory)
			color = config.dirColor;
		else if (entry.permissions & nc::fs::Permission::EXECUTE)
			color = config.exeColor;

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (entry.name == inputTextString)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (config.colors)
			ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TreeNodeEx(entry.name.data(), nodeFlags, "%s", auxString.data());
		if (config.colors)
			ImGui::PopStyleColor();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (config.selectionType == SelectionType::DIRECTORY)
			{
				if (entry.isDirectory)
				{
					config.directory = nc::fs::joinPath(config.directory, entry.name.data());
					config.directory = nc::fs::absolutePath(config.directory.data());
					inputTextString.clear();
				}
			}
			else
			{
				// Perform a selection by double clicking
				selection = nc::fs::joinPath(config.directory, entry.name);
				inputTextString.clear();
				if (config.selectionType == SelectionType::DIRECTORY)
					config.directory = selection;

				selected = true;
				config.windowOpen = false;
				if (config.modalPopup)
					ImGui::CloseCurrentPopup();
			}
		}
		else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			if (config.selectionType == SelectionType::DIRECTORY)
			{
				if (entry.isDirectory && entry.name != "." && entry.name != "..")
					inputTextString = entry.name;
			}
			else
			{
				if (entry.isDirectory)
				{
					config.directory = nc::fs::joinPath(config.directory, entry.name.data());
					config.directory = nc::fs::absolutePath(config.directory.data());
					if (config.selectionType == SelectionType::FILE)
						inputTextString.clear();
				}
				else
					inputTextString = entry.name;
			}
		}
	}

	ImGui::EndChild();

	ImGui::Separator();
	ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue;
	if (config.selectionType != SelectionType::NEW_FILE)
		inputTextFlags |= ImGuiInputTextFlags_ReadOnly;

	if (config.action == Action::SAVE_PROJECT)
		ImGui::PushItemWidth(250.0f);
	if (ImGui::InputText("Name", inputTextString.data(), nc::fs::MaxPathLength, inputTextFlags, ui::inputTextCallback, &inputTextString))
	{
		// Perform a selection by pressing the Enter key
		selection = nc::fs::joinPath(config.directory, inputTextString);
		inputTextString.clear();
		if (config.selectionType == SelectionType::DIRECTORY)
			config.directory = selection;

		selected = true;
		config.windowOpen = false;
		if (config.modalPopup)
			ImGui::CloseCurrentPopup();
	}
	if (config.action == Action::SAVE_PROJECT)
		ImGui::PopItemWidth();

	if (config.action == Action::SAVE_PROJECT)
	{
		ImGui::SameLine();
		ImGui::Checkbox("Overwrite", &config.allowOverwrite);
	}

	ImGui::SameLine();
	if (ImGui::Button(config.okButton))
	{
		// Perform a selection by pressing the OK button
		selection = nc::fs::joinPath(config.directory, inputTextString);
		inputTextString.clear();
		if (config.selectionType == SelectionType::DIRECTORY)
			config.directory = selection;

		selected = true;
		config.windowOpen = false;
		if (config.modalPopup)
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button(Labels::Cancel))
	{
		selection.clear();
		inputTextString.clear();

		config.windowOpen = false;
		if (config.modalPopup)
			ImGui::CloseCurrentPopup();
	}
#ifdef __ANDROID__
	static bool softInputPreviousState = false;
	static bool softInputState = false;
	ImGui::SameLine();
	ImGui::Checkbox("Soft Input", &softInputState);
	if (softInputState != softInputPreviousState)
	{
		static_cast<nc::AndroidApplication &>(nc::theApplication()).toggleSoftInput();
		softInputPreviousState = softInputState;
	}
#endif

	if (config.modalPopup)
		ImGui::EndPopup();
	else
		ImGui::End();
	return selected;
}
