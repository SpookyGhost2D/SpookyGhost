#ifdef WITH_FONTAWESOME
	#include "IconsFontAwesome5.h"
#endif

#include "gui/gui_labels.h"
#include "gui/gui_common.h"
#include "gui/FileDialog.h"

#include <nctl/Array.h>
#include <nctl/algorithms.h>
#include <ncine/Application.h>
#include <ncine/imgui_internal.h>

#ifdef __ANDROID__
	#include <ncine/AndroidApplication.h>
#endif

namespace {

const char *sortingStrings[6] = {
	Labels::FileDialog_Name_Asc,
	Labels::FileDialog_Name_Desc,
	Labels::FileDialog_Size_Asc,
	Labels::FileDialog_Size_Desc,
	Labels::FileDialog_Date_Asc,
	Labels::FileDialog_Date_Desc
};

struct DragPinnedDirectoryPayload
{
	unsigned int index;
};

const float PinnedDirectoryTooltipDelay = 0.5f;
const float PinnedDirectoriesColumnWeight = 0.25f;

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
		config.directory = ui::androidSaveDir;
#else
		config.directory = nc::fs::currentDir();
#endif
	}
#ifndef __ANDROID__
	else
		config.directory = nc::fs::absolutePath(config.directory.data());
#endif

	const ImVec2 windowSize = ImVec2(650.0f, 350.0f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
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
		if (config.pinnedDirectories != nullptr)
		{
			ImGui::Checkbox("Show Pinned", &config.showPinnedDirectories);
			ImGui::SameLine();
		}
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

	const float additionalChildVSpacing = (config.showPinnedDirectories && config.pinnedDirectories != nullptr) ? 6.0f : 4.0f;
	if (config.showPinnedDirectories && config.pinnedDirectories != nullptr)
	{
		nctl::Array<nctl::String> &pinnedDirectories = *config.pinnedDirectories;

		ImGui::BeginTable("PinnedDirectoriesAndBrowser", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV);
		const float columnOneWeight = pinnedDirectories.isEmpty() ? PinnedDirectoriesColumnWeight : -1.0f;
		const float columnTwoWeight = pinnedDirectories.isEmpty() ? 1.0f - PinnedDirectoriesColumnWeight : -1.0f;
		const ImGuiTableColumnFlags columnOneFlag = pinnedDirectories.isEmpty() ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("PinnedDirectories", columnOneFlag, columnOneWeight);
		ImGui::TableSetupColumn("Browser", ImGuiTableColumnFlags_WidthStretch, columnTwoWeight);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		if (ImGui::Button(Labels::FileDialog_Pin))
		{
			bool alreadyPinned = false;
			for (unsigned int i = 0; i < pinnedDirectories.size(); i++)
			{
				if (config.directory == pinnedDirectories[i])
				{
					alreadyPinned = true;
					break;
				}
			}
			if (alreadyPinned == false)
				pinnedDirectories.pushBack(config.directory);
		}
		ImGui::SameLine();
		const bool enableUnpinButton = pinnedDirectories.isEmpty() == false;
		ImGui::BeginDisabled(enableUnpinButton == false);
		if (ImGui::Button(Labels::FileDialog_Unpin))
		{
			for (unsigned int i = 0; i < pinnedDirectories.size(); i++)
			{
				if (config.directory == pinnedDirectories[i])
				{
					pinnedDirectories.removeAt(i);
					break;
				}
			}
		}
		ImGui::EndDisabled();

		ImGui::BeginChild("Pinned Directories List", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() - additionalChildVSpacing));

		int directoryToRemove = -1;
		for (unsigned int i = 0; i < pinnedDirectories.size(); i++)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

			if (config.directory == pinnedDirectories[i])
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			// Append the index to have a unique ID when two paths have the same base name
			auxString.format("%s %s##%d", Labels::FileDialog_FolderIcon, nc::fs::baseName(pinnedDirectories[i].data()).data(), i);

			if (config.colors)
				ImGui::PushStyleColor(ImGuiCol_Text, config.dirColor);

			ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
			ImGui::TreeNodeEx(auxString.data(), nodeFlags);
			ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());

			if (config.colors)
				ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > PinnedDirectoryTooltipDelay)
			{
				ImGui::BeginTooltip();
				if (config.colors)
					ImGui::PushStyleColor(ImGuiCol_Text, config.dirColor);

				auxString.format("%s %s", Labels::FileDialog_FolderIcon, pinnedDirectories[i].data());
				ImGui::Text("%s", auxString.data());

				if (config.colors)
					ImGui::PopStyleColor();
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem(Labels::FileDialog_Unpin))
					pinnedDirectories.removeAt(i);
				ImGui::EndPopup();
			}

			if (ImGui::IsItemClicked())
			{
				if (nc::fs::isDirectory(pinnedDirectories[i].data()))
					config.directory = pinnedDirectories[i].data();
				else
					directoryToRemove = i;
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				DragPinnedDirectoryPayload dragPayload = { i };
				ImGui::SetDragDropPayload("PINNEDDIRECTORY_TREENODE", &dragPayload, sizeof(DragPinnedDirectoryPayload));

				if (config.colors)
					ImGui::PushStyleColor(ImGuiCol_Text, config.dirColor);

				ImGui::Text("%s", auxString.data());

				if (config.colors)
					ImGui::PopStyleColor();

				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PINNEDDIRECTORY_TREENODE"))
				{
					IM_ASSERT(payload->DataSize == sizeof(DragPinnedDirectoryPayload));
					const DragPinnedDirectoryPayload &dragPayload = *reinterpret_cast<const DragPinnedDirectoryPayload *>(payload->Data);

					nctl::String dragPinnedDirectory(nctl::move(pinnedDirectories[dragPayload.index]));
					pinnedDirectories.removeAt(dragPayload.index);
					pinnedDirectories.insertAt(i, nctl::move(dragPinnedDirectory));

					ImGui::EndDragDropTarget();
				}
			}
		}
		if (directoryToRemove >= 0)
			pinnedDirectories.removeAt(directoryToRemove);

		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
	}

	if (nc::fs::logicalDrives())
	{
		char driveLetter[3] = "C:";
		static int currentComboLogicalDrive = 0;
		const char *logicalDriveStrings = nc::fs::logicalDriveStrings();
		ImGui::PushItemWidth(50.0f);
		if (ImGui::Combo("##LogicalDrives", &currentComboLogicalDrive, logicalDriveStrings))
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
		// Append the index to have a unique ID when two paths have the same base name
		auxString.format("%s##%d", baseNames[i].data(), i);
		if (ImGui::Button(auxString.data()))
		{
			config.directory = dirName;
			if (config.selectionType != SelectionType::NEW_FILE)
				inputTextString.clear();
		}
		if (i > 0)
			ImGui::SameLine();
	}

	ImGui::BeginChild("File View", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() - additionalChildVSpacing));

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

	if (config.showPinnedDirectories && config.pinnedDirectories != nullptr)
		ImGui::EndTable();

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

#ifdef _WIN32
	if (selected)
	{
		// Replace backslashes with slashes to avoid characters escaping
		for (unsigned int i = 0; i < selection.length(); i++)
		{
			if (selection[i] == '\\')
				selection[i] = '/';
		}
	}
#endif

	if (config.modalPopup)
		ImGui::EndPopup();
	else
		ImGui::End();
	return selected;
}
