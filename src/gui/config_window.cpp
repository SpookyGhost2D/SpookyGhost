#include <ncine/IGfxDevice.h>
#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "singletons.h"
#include "gui/UserInterface.h"
#include "gui/gui_labels.h"
#include "Configuration.h"
#include "LuaSaver.h"

bool UserInterface::showConfigWindow = false;

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createConfigWindow()
{
	if (showConfigWindow == false)
		return;

	nc::IGfxDevice &gfxDevice = nc::theApplication().gfxDevice();
	const float scalingFactor = theCfg.autoGuiScaling ? gfxDevice.windowScalingFactor() : theCfg.guiScaling;
	const ImVec2 guiWindowSize = ImVec2(500.0f * scalingFactor, 475.0f * scalingFactor);
	ImGui::SetNextWindowSize(guiWindowSize, ImGuiCond_Once);
	const ImVec2 guiWindowPos = ImVec2(ImGui::GetWindowViewport()->Size.x * 0.5f, ImGui::GetWindowViewport()->Size.y * 0.5f);
	ImGui::SetNextWindowPos(guiWindowPos, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
	ImGui::Begin(Labels::Configuration, &showConfigWindow, ImGuiWindowFlags_NoDocking);

#ifdef __ANDROID__
	ImGui::Text("Screen Resolution: %d x %d", gfxDevice.width(), gfxDevice.height());
	ImGui::TextUnformatted("Resizable: false");
	ImGui::TextUnformatted("Fullscreen: true");
#else
	static bool fullScreen = theCfg.fullScreen;
	static int selectedVideoMode[nc::IGfxDevice::MaxMonitors] = {};
	const unsigned int monitorIndex = gfxDevice.windowMonitorIndex();
	const nc::IGfxDevice::VideoMode &currentVideoMode = gfxDevice.currentVideoMode(monitorIndex);
	const nc::IGfxDevice::Monitor &monitor = gfxDevice.monitor(monitorIndex);

	ImGui::BeginDisabled(gfxDevice.isFullScreen());
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Window Position: %d, %d", gfxDevice.windowPositionX(), gfxDevice.windowPositionY());
	ImGui::SameLine();
	if (ImGui::Button(Labels::CenterWindow))
	{
		const nc::Vector2i windowSize(gfxDevice.width(), gfxDevice.height());
		const nc::Vector2i screenResolution(currentVideoMode.width, currentVideoMode.height);
		gfxDevice.setWindowPosition(monitor.position + (screenResolution - windowSize) / 2);
	}
	ImGui::EndDisabled();
	theCfg.windowPositionX = gfxDevice.windowPositionX();
	theCfg.windowPositionY = gfxDevice.windowPositionY();

	ImGui::BeginDisabled(fullScreen);
	ImGui::SliderInt("Window Width", &theCfg.width, 0, currentVideoMode.width / scalingFactor);
	ImGui::SliderInt("Window Height", &theCfg.height, 0, currentVideoMode.height / scalingFactor);
	const nc::Vector2i scaledWindowSize(theCfg.width * scalingFactor, theCfg.height * scalingFactor);
	if (scalingFactor != 1.0f)
		ImGui::Text("Scaled Size: %d x %d (factor: %.2f)", scaledWindowSize.x, scaledWindowSize.y, scalingFactor);
	ImGui::EndDisabled();

	ui::auxString.format("Monitor #%u: \"%s\"", monitorIndex, monitor.name);
	if (monitorIndex == gfxDevice.primaryMonitorIndex())
		ui::auxString.append(" [Primary]");
	ImGui::TextUnformatted(ui::auxString.data());

	const unsigned int numVideoModes = monitor.numVideoModes;
	ui::comboString.clear();
	for (unsigned int modeIndex = 0; modeIndex < numVideoModes; modeIndex++)
	{
		const nc::IGfxDevice::VideoMode &mode = monitor.videoModes[modeIndex];
		ui::comboString.formatAppend("#%u: %u x %u, %.2f Hz", modeIndex, mode.width, mode.height, mode.refreshRate);
		ui::comboString.setLength(ui::comboString.length() + 1);
	}
	ui::comboString.setLength(ui::comboString.length() + 1);
	// Append a second '\0' to signal the end of the combo item list
	ui::comboString[ui::comboString.length() - 1] = '\0';

	if (selectedVideoMode[monitorIndex] < 0)
		selectedVideoMode[monitorIndex] = 0;
	if (selectedVideoMode[monitorIndex] > monitor.numVideoModes - 1)
		selectedVideoMode[monitorIndex] = monitor.numVideoModes - 1;

	ImGui::Combo("Video Mode", &selectedVideoMode[monitorIndex], ui::comboString.data());
	ImGui::Checkbox("Full Screen", &fullScreen);

	ImGui::BeginDisabled(fullScreen);
	ImGui::SameLine();
	if (ImGui::Checkbox("Resizable", &theCfg.resizable) && theCfg.resizable != gfxDevice.isResizable())
		pushStatusInfoMessage("Restart the application to see the changes");
	ImGui::EndDisabled();

	const bool shouldChangeVideoMode = (monitor.videoModes[selectedVideoMode[monitorIndex]] != currentVideoMode);
	const bool shouldChangeWindowSize = (gfxDevice.resolution().x != scaledWindowSize.x || gfxDevice.resolution().y != scaledWindowSize.y);
	const bool canApply = (gfxDevice.isFullScreen() != fullScreen) || (fullScreen && shouldChangeVideoMode) || shouldChangeWindowSize;
	ImGui::BeginDisabled(canApply == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Apply))
	{
		if (fullScreen)
		{
			gfxDevice.setVideoMode(selectedVideoMode[monitorIndex]);
			gfxDevice.setFullScreen(fullScreen);

			if (shouldChangeVideoMode)
				openVideoModePopup(); // The popup saves the configuration
			else
			{
				theCfg.width = currentVideoMode.width;
				theCfg.height = currentVideoMode.height;
				theCfg.refreshRate = 0.0f;
				theCfg.fullScreen = fullScreen;
			}
		}
		else
		{
			gfxDevice.setFullScreen(fullScreen);
			theCfg.refreshRate = 0.0f;
			const nc::Vector2i halfScreenResolution(currentVideoMode.width / 2, currentVideoMode.height / 2);
			gfxDevice.setWindowPosition(monitor.position + halfScreenResolution - scaledWindowSize / 2);
			gfxDevice.setWindowSize(scaledWindowSize);
			theCfg.fullScreen = fullScreen;
		}
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	const bool currentEnabled = (shouldChangeVideoMode || gfxDevice.isFullScreen() != fullScreen ||
	                             shouldChangeWindowSize || gfxDevice.isResizable() != theCfg.resizable);
	ImGui::BeginDisabled(currentEnabled == false);
	if (ImGui::Button(Labels::Current))
	{
		for (unsigned int i = 0; i < monitor.numVideoModes; i++)
		{
			if (monitor.videoModes[i] == currentVideoMode)
			{
				selectedVideoMode[monitorIndex] = i;
				break;
			}
		}
		theCfg.width = gfxDevice.width() / scalingFactor;
		theCfg.height = gfxDevice.height() / scalingFactor;
		fullScreen = gfxDevice.isFullScreen();
		theCfg.resizable = gfxDevice.isResizable();
	}
	ImGui::EndDisabled();
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
	if (ImGui::Checkbox("Automatic GUI Scaling", &theCfg.autoGuiScaling))
		theCfg.guiScaling = gfxDevice.windowScalingFactor();
	ImGui::BeginDisabled(theCfg.autoGuiScaling);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Reset))
		theCfg.guiScaling = 1.0f;
	ImGui::SliderFloat("GUI Scaling", &theCfg.guiScaling, 0.5f, 2.0f, "%.2f");
	ImGui::EndDisabled();

	if (ImGui::GetStyle().FontScaleMain != theCfg.guiScaling)
		changeScalingFactor(theCfg.guiScaling);

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
		theSaver->saveCfg(theCfg);

	ImGui::End();
}

void UserInterface::sanitizeConfigValues()
{
	if (theCfg.width < 640)
		theCfg.width = 640;
	if (theCfg.height < 480)
		theCfg.height = 480;
	if (theCfg.refreshRate < 0.0f)
		theCfg.refreshRate = 0.0f;

	if (theCfg.frameLimit > 240)
		theCfg.frameLimit = 240;

	if (theCfg.canvasWidth < 16)
		theCfg.canvasWidth = 16;
	if (theCfg.canvasHeight < 16)
		theCfg.canvasHeight = 16;

	if (theCfg.autoGuiScaling == false)
	{
		if (theCfg.guiScaling < 0.5f)
			theCfg.guiScaling = 0.5f;
		else if (theCfg.guiScaling > 2.0f)
			theCfg.guiScaling = 2.0f;
	}
}
