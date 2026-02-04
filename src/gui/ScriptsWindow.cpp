#include <ncine/imgui_internal.h>
#include <ncine/InputEvents.h>

#include "gui/ScriptsWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "gui/FileDialog.h"
#include "Script.h"
#include "ScriptManager.h"
#include "AnimationManager.h"

#include "scripts_strings.h"

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

ScriptsWindow::ScriptsWindow(UserInterface &ui)
    : ui_(ui)
{
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void ScriptsWindow::create()
{
	ImGui::Begin(Labels::Scripts);

	if (ImGui::IsWindowHovered() && ui::dropEvent != nullptr)
	{
		for (unsigned int i = 0; i < ui::dropEvent->numPaths; i++)
			loadScript(ui::dropEvent->paths[i]);
		ui::dropEvent = nullptr;
	}

	const bool openBundledEnabled = ScriptsStrings::Count > 0;
	if (openBundledEnabled)
	{
		ui::comboString.clear();
		ui::comboString.append(Labels::BundledScripts);
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < ScriptsStrings::Count; i++)
		{
			ui::comboString.formatAppend("%s", ScriptsStrings::Names[i]);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		static int currentComboScript = 0;
		if (ImGui::Combo("###BundledScripts", &currentComboScript, ui::comboString.data()) && currentComboScript > 0)
		{
			loadScript(nc::fs::joinPath(ui::scriptsDataDir, ScriptsStrings::Names[currentComboScript - 1]).data());
			currentComboScript = 0;
		}
	}

	if (ImGui::Button(Labels::Load))
	{
		FileDialog::config.directory = theCfg.scriptsPath;
		FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
		FileDialog::config.windowTitle = "Load script file";
		FileDialog::config.okButton = Labels::Ok;
		FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
		FileDialog::config.extensions = "lua\0\0";
		FileDialog::config.action = FileDialog::Action::LOAD_SCRIPT;
		FileDialog::config.windowOpen = true;
	}

	const bool enableButtons = theScriptingMgr->scripts().isEmpty() == false;
	ImGui::BeginDisabled(enableButtons == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Remove) || (ui_.deleteKeyPressed_ && ImGui::IsWindowHovered()))
		removeScript();

	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Reload))
		reloadScript();
	ImGui::EndDisabled();

	ImGui::Separator();

	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		for (unsigned int i = 0; i < theScriptingMgr->scripts().size(); i++)
		{
			Script &script = *theScriptingMgr->scripts()[i];
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (i == ui_.selectedScriptIndex_)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ui::auxString.format("#%u: \"%s\" %s", i, nc::fs::baseName(script.name().data()).data(), script.canRun() ? Labels::CheckIcon : Labels::TimesIcon);
			ImGui::TreeNodeEx(static_cast<void *>(&script), nodeFlags, "%s", ui::auxString.data());
			if (ImGui::IsItemClicked())
				ui_.selectedScriptIndex_ = i;

			if (ImGui::IsItemHovered() && script.canRun() == false)
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(450.0f);
				ImGui::TextColored(ImVec4(0.925f, 0.243f, 0.251f, 1.0f), "%s", script.errorMsg());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (ImGui::BeginPopupContextItem())
			{
				ui_.selectedScriptIndex_ = i;

				if (ImGui::MenuItem(Labels::Reload))
					reloadScript();
				if (ImGui::MenuItem(Labels::Remove))
					removeScript();
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

bool ScriptsWindow::loadScript(const char *filename) // TODO: Move back to ui?
{
	nctl::UniquePtr<Script> script = nctl::makeUnique<Script>();
	const bool hasLoaded = script->load(filename);

	theScriptingMgr->scripts().pushBack(nctl::move(script));
	// Check if the script is in the configuration path or in the data directory
	const nctl::String baseName = nc::fs::baseName(filename);
	const nctl::String nameInConfigDir = nc::fs::joinPath(theCfg.scriptsPath, baseName);
	const nctl::String nameInDataDir = nc::fs::joinPath(ui::scriptsDataDir, baseName);
	if ((nameInConfigDir == filename && nc::fs::isReadableFile(nameInConfigDir.data())) ||
	    (nameInDataDir == filename && nc::fs::isReadableFile(nameInDataDir.data())))
	{
		// Set the script name to its basename to allow for relocatable project files
		theScriptingMgr->scripts().back()->setName(baseName);
	}
	ui_.selectedScriptIndex_ = theScriptingMgr->scripts().size() - 1;

	if (hasLoaded)
	{
		if (theScriptingMgr->scripts().back()->canRun())
		{
			ui::auxString.format("Loaded script \"%s\"", filename);
			ui_.pushStatusInfoMessage(ui::auxString.data());
		}
		else
		{
			ui::auxString.format("Loaded script \"%s\", but it cannot run", filename);
			ui_.pushStatusErrorMessage(ui::auxString.data());
		}
	}
	else
	{
		ui::auxString.format("Cannot load script \"%s\"", filename);
		ui_.pushStatusErrorMessage(ui::auxString.data());
	}

	return hasLoaded;
}

void ScriptsWindow::reloadScript() // TODO: Move back to ui?
{
	if (theScriptingMgr->scripts().isEmpty() == false)
	{
		Script *script = theScriptingMgr->scripts()[ui_.selectedScriptIndex_].get();
		script->reload();
		theAnimMgr->reloadScript(script);

		ui::auxString.format("Reloaded script \"%s\"\n", script->name().data());
		ui_.pushStatusInfoMessage(ui::auxString.data());
	}
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void ScriptsWindow::removeScript()
{
	Script *selectedScript = theScriptingMgr->scripts()[ui_.selectedScriptIndex_].get();
	theAnimMgr->removeScript(selectedScript);

	theScriptingMgr->scripts().removeAt(ui_.selectedScriptIndex_);
	if (ui_.selectedScriptIndex_ > 0)
		ui_.selectedScriptIndex_--;
}
