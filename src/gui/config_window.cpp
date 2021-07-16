#include <ncine/IGfxDevice.h>
#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "singletons.h"
#include "gui/UserInterface.h"
#include "gui/gui_labels.h"
#include "Configuration.h"
#include "LuaSaver.h"

#ifdef __ANDROID__
	#include <ncine/FileSystem.h>
#endif

bool UserInterface::showConfigWindow = false;

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createConfigWindow()
{
	if (showConfigWindow == false)
		return;

	const ImVec2 windowSize = ImVec2(500.0f, 475.0f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	const ImVec2 windowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::Begin(Labels::Configuration, &showConfigWindow, ImGuiWindowFlags_NoDocking);

#ifdef __ANDROID__
	ImGui::Text("Screen Width: %d", nc::theApplication().widthInt());
	ImGui::Text("Screen Height: %d", nc::theApplication().heightInt());
	ImGui::Text("Resizable: false");
	ImGui::Text("Fullscreen: true");
#else
	static int selectedVideoMode = -1;
	const nc::IGfxDevice::VideoMode currentVideoMode = nc::theApplication().gfxDevice().currentVideoMode();
	if (theCfg.fullscreen == false)
	{
		ImGui::SliderInt("Window Width", &theCfg.width, 0, currentVideoMode.width);
		ImGui::SliderInt("Window Height", &theCfg.height, 0, currentVideoMode.height);
		ImGui::Checkbox("Resizable", &theCfg.resizable);
		ImGui::SameLine();
		selectedVideoMode = -1;
	}
	else
	{
		unsigned int currentVideoModeIndex = 0;
		const unsigned int numVideoModes = nc::theApplication().gfxDevice().numVideoModes();
		ui::comboString.clear();
		for (unsigned int i = 0; i < numVideoModes; i++)
		{
			const nc::IGfxDevice::VideoMode &mode = nc::theApplication().gfxDevice().videoMode(i);
			ui::comboString.formatAppend("%ux%u, %uHz", mode.width, mode.height, mode.refreshRate);
			ui::comboString.setLength(ui::comboString.length() + 1);

			if (mode == currentVideoMode)
				currentVideoModeIndex = i;
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		if (selectedVideoMode < 0)
			selectedVideoMode = currentVideoModeIndex;

		ImGui::Combo("Video Mode", &selectedVideoMode, ui::comboString.data());
		theCfg.width = nc::theApplication().gfxDevice().videoMode(selectedVideoMode).width;
		theCfg.height = nc::theApplication().gfxDevice().videoMode(selectedVideoMode).height;
	}

	ImGui::Checkbox("Fullscreen", &theCfg.fullscreen);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Apply))
	{
		nc::theApplication().gfxDevice().setFullScreen(theCfg.fullscreen);
		if (theCfg.fullscreen == false)
			nc::theApplication().gfxDevice().setResolution(theCfg.width, theCfg.height);
		else
			nc::theApplication().gfxDevice().setVideoMode(selectedVideoMode);
	}
	ImGui::SameLine();
	if (ImGui::Button(Labels::Current))
	{
		theCfg.width = nc::theApplication().widthInt();
		theCfg.height = nc::theApplication().heightInt();
		theCfg.fullscreen = nc::theApplication().gfxDevice().isFullScreen();
		theCfg.resizable = nc::theApplication().gfxDevice().isResizable();
		selectedVideoMode = -1;
	}
#endif

	ImGui::NewLine();
#ifdef __ANDROID__
	ImGui::Text("Vertical Sync: true");
#else
	ImGui::Checkbox("Vertical Sync", &theCfg.withVSync);
	nc::theApplication().gfxDevice().setSwapInterval(theCfg.withVSync ? 1 : 0);
#endif
	int frameLimit = theCfg.frameLimit;
	ImGui::SliderInt("Frame Limit", &frameLimit, 0, 240);
	theCfg.frameLimit = frameLimit < 0 ? 0 : frameLimit;

	ImGui::NewLine();
	ImGui::SliderInt("Canvas Width", &theCfg.canvasWidth, 0, 1024);
	ImGui::SliderInt("Canvas Height", &theCfg.canvasHeight, 0, 1024);

	ImGui::NewLine();
	ImGui::SliderFloat("GUI Scaling", &theCfg.guiScaling, 0.5f, 2.0f, "%.1f");
	ImGui::SameLine();
	if (ImGui::Button("Reset##Scaling"))
		theCfg.guiScaling = 1.0f;
	ImGui::GetIO().FontGlobalScale = theCfg.guiScaling;

	ImGui::InputText("Start-up Project", theCfg.startupProjectName.data(), ui::MaxStringLength,
	                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &theCfg.startupProjectName);
	ImGui::Checkbox("Auto Play On Start", &theCfg.autoPlayOnStart);

	ImGui::NewLine();
	ImGui::InputText("Projects Path", theCfg.projectsPath.data(), ui::MaxStringLength,
	                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &theCfg.projectsPath);
	ImGui::InputText("Textures Path", theCfg.texturesPath.data(), ui::MaxStringLength,
	                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &theCfg.texturesPath);
	ImGui::InputText("Scripts Path", theCfg.scriptsPath.data(), ui::MaxStringLength,
	                 ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize, ui::inputTextCallback, &theCfg.scriptsPath);

	ImGui::Checkbox("Show Tips On Start", &theCfg.showTipsOnStart);

	sanitizeConfigValues();

	ImGui::NewLine();
	if (ImGui::Button(Labels::Close))
		showConfigWindow = false;

	// Auto-save on window close
	if (showConfigWindow == false)
	{
		ui::auxString = "config.lua";
#ifdef __ANDROID__
		// On Android the configuration file is saved in the internal storage directory
		ui::auxString = nc::fs::joinPath(ui::androidCfgDir.data(), "config.lua");
#endif
		theSaver->saveCfg(ui::auxString.data(), theCfg);
	}

	ImGui::End();
}

void UserInterface::sanitizeConfigValues()
{
	if (theCfg.width < 640)
		theCfg.width = 640;
	if (theCfg.height < 480)
		theCfg.height = 480;

	if (theCfg.frameLimit > 240)
		theCfg.frameLimit = 240;

	if (theCfg.canvasWidth < 16)
		theCfg.canvasWidth = 16;
	if (theCfg.canvasHeight < 16)
		theCfg.canvasHeight = 16;

	if (theCfg.guiScaling < 0.5f)
		theCfg.guiScaling = 0.5f;
	else if (theCfg.guiScaling > 2.0f)
		theCfg.guiScaling = 2.0f;
}
