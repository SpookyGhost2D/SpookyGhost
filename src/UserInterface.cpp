#include <ncine/imgui.h>
#include <ncine/imgui_internal.h>
#include <ncine/Application.h>
#include <ncine/IFile.h>

#include "gui_labels.h"
#include "UserInterface.h"
#include "Canvas.h"
#include "SpriteManager.h"
#include "AnimationManager.h"
#include "PropertyAnimation.h"
#include "ParallelAnimationGroup.h"
#include "SequentialAnimationGroup.h"
#include "GridAnimation.h"
#include "Sprite.h"
#include "Texture.h"

#include "version.h"
#include <ncine/version.h>

namespace {

const char *easingCurveTypes[] = { "Linear", "Quadratic", "Cubic", "Quartic", "Quintic", "Sine", "Exponential", "Circular" };
const char *easingCurveLoopModes[] = { "Disabled", "Rewind", "Ping Pong" };

const char *animationTypes[] = { "Parallel Group", "Sequential Group", "Property", "Grid" };
enum AnimationTypesEnum { PARALLEL_GROUP, SEQUENTIAL_GROUP, PROPERTY, GRID };

const char *propertyTypes[] = { "None", "Position X", "Position Y", "Rotation" };
enum PropertyTypesEnum { NONE, POSITION_X, POSITION_Y, ROTATION };

const char *gridAnimTypes[] = { "Wobble X", "Wooble Y", "Zoom" };

const char *resizePresets[] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "custom" };
enum ResizePresetsEnum { SIZE16, SIZE32, SIZE64, SIZE128, SIZE256, SIZE512, CUSTOM };

static bool showAboutWindow = false;

int inputTextCallback(ImGuiInputTextCallbackData *data)
{
	nctl::String *string = reinterpret_cast<nctl::String *>(data->UserData);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		ASSERT(data->Buf == string->data());
		string->setLength(static_cast<unsigned int>(data->BufTextLen));
		data->Buf = string->data();
	}
	return 0;
}

const char *animStateToString(IAnimation::State state)
{
	switch (state)
	{
		case IAnimation::State::STOPPED: return "Stopped";
		case IAnimation::State::PAUSED: return "Paused";
		case IAnimation::State::PLAYING: return "Playing";
	}
	return "Unknown";
}

void applyDefaultStyle()
{
	ImGuiStyle *style = &ImGui::GetStyle();
	ImVec4 *colors = style->Colors;

	colors[ImGuiCol_Text]                   = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	colors[ImGuiCol_Border]                 = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_CheckMark]              = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Button]                 = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
	colors[ImGuiCol_Header]                 = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_DockingPreview]         = ImVec4(1.000f, 0.391f, 0.000f, 0.781f);
	colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

	style->ChildRounding = 4.0f;
	style->FrameBorderSize = 1.0f;
	style->FrameRounding = 2.0f;
	style->GrabMinSize = 7.0f;
	style->PopupRounding = 2.0f;
	style->ScrollbarRounding = 12.0f;
	style->ScrollbarSize = 13.0f;
	style->TabBorderSize = 1.0f;
	style->TabRounding = 0.0f;
	style->WindowRounding = 4.0f;
}

void applyDarcula()
{
	ImGuiStyle *style = &ImGui::GetStyle();
	style->WindowRounding = 5.3f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;

	style->Colors[ImGuiCol_Text]                  = {0.73333335f, 0.73333335f, 0.73333335f, 1.00f};
	style->Colors[ImGuiCol_TextDisabled]          = {0.34509805f, 0.34509805f, 0.34509805f, 1.00f};
	style->Colors[ImGuiCol_WindowBg]              = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_ChildBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.00f};
	style->Colors[ImGuiCol_PopupBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_Border]                = {0.33333334f, 0.33333334f, 0.33333334f, 0.50f};
	style->Colors[ImGuiCol_BorderShadow]          = {0.15686275f, 0.15686275f, 0.15686275f, 0.00f};
	style->Colors[ImGuiCol_FrameBg]               = {0.16862746f, 0.16862746f, 0.16862746f, 0.54f};
	style->Colors[ImGuiCol_FrameBgHovered]        = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_FrameBgActive]         = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_TitleBg]               = {0.04f, 0.04f, 0.04f, 1.00f};
	style->Colors[ImGuiCol_TitleBgCollapsed]      = {0.16f, 0.29f, 0.48f, 1.00f};
	style->Colors[ImGuiCol_TitleBgActive]         = {0.00f, 0.00f, 0.00f, 0.51f};
	style->Colors[ImGuiCol_MenuBarBg]             = {0.27058825f, 0.28627452f, 0.2901961f, 0.80f};
	style->Colors[ImGuiCol_ScrollbarBg]           = {0.27058825f, 0.28627452f, 0.2901961f, 0.60f};
	style->Colors[ImGuiCol_ScrollbarGrab]         = {0.21960786f, 0.30980393f, 0.41960788f, 0.51f};
	style->Colors[ImGuiCol_ScrollbarGrabHovered]  = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ScrollbarGrabActive]   = {0.13725491f, 0.19215688f, 0.2627451f, 0.91f};
	// style->Colors[ImGuiCol_ComboBg]               = {0.1f, 0.1f, 0.1f, 0.99f};
	style->Colors[ImGuiCol_CheckMark]             = {0.90f, 0.90f, 0.90f, 0.83f};
	style->Colors[ImGuiCol_SliderGrab]            = {0.70f, 0.70f, 0.70f, 0.62f};
	style->Colors[ImGuiCol_SliderGrabActive]      = {0.30f, 0.30f, 0.30f, 0.84f};
	style->Colors[ImGuiCol_Button]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.49f};
	style->Colors[ImGuiCol_ButtonHovered]         = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ButtonActive]          = {0.13725491f, 0.19215688f, 0.2627451f, 1.00f};
	style->Colors[ImGuiCol_Header]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.53f};
	style->Colors[ImGuiCol_HeaderHovered]         = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_HeaderActive]          = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_Separator]             = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorHovered]      = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorActive]       = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_ResizeGrip]            = {1.00f, 1.00f, 1.00f, 0.85f};
	style->Colors[ImGuiCol_ResizeGripHovered]     = {1.00f, 1.00f, 1.00f, 0.60f};
	style->Colors[ImGuiCol_ResizeGripActive]      = {1.00f, 1.00f, 1.00f, 0.90f};
	style->Colors[ImGuiCol_PlotLines]             = {0.61f, 0.61f, 0.61f, 1.00f};
	style->Colors[ImGuiCol_PlotLinesHovered]      = {1.00f, 0.43f, 0.35f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogram]         = {0.90f, 0.70f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogramHovered]  = {1.00f, 0.60f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_TextSelectedBg]        = {0.18431373f, 0.39607847f, 0.79215693f, 0.90f};
}

inline void applyDarkStyle()
{
	ImGuiStyle & style = ImGui::GetStyle();
	ImVec4 *colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 1;

	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding  = ImVec2(6, 4);
	style.ItemSpacing   = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize  = 1;
	style.PopupBorderSize  = 1;
	style.FrameBorderSize  = is3D;

	style.WindowRounding    = 3;
	style.ChildRounding     = 3;
	style.FrameRounding     = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding      = 3;

	#ifdef IMGUI_HAS_DOCK
		style.TabBorderSize = is3D;
		style.TabRounding   = 3;

		colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	#endif
}

void applyYetAnotherDarkStyle()
{
	//imGuiIO.Fonts->AddFontFromFileTTF("../data/Fonts/Ruda-Bold.ttf", 15.0f, &config);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;

	ImVec4 *colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface(Canvas &canvas, SpriteManager &spriteMgr, AnimationManager &animMgr)
    : canvas_(canvas), spriteMgr_(spriteMgr), animMgr_(animMgr)
{
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#ifdef __ANDROID__
	io.FontGlobalScale = 2.0f;
#endif

#ifdef WITH_FONTAWESOME
	// Merge icons from Font Awesome into the default font
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF((nc::IFile::dataPath() + "fonts/" FONT_ICON_FILE_NAME_FAS).data(), 12.0f, &icons_config, icons_ranges);
#endif

	//applyDefaultStyle();
	//applyDarcula();
	applyDarkStyle();
	//applyYetAnotherDarkStyle();
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::signalFrameSaved()
{
	ASSERT(shouldSaveAnim_ == true);

	saveAnimStatus_.numSavedFrames++;
	saveAnimStatus_.filename.format("%s_%03d.png", animFilename_.data(), saveAnimStatus_.numSavedFrames);
	if (saveAnimStatus_.numSavedFrames == saveAnimStatus_.numFrames)
	{
		shouldSaveAnim_ = false;
		saveAnimStatus_.numSavedFrames = 0;
		pushStatusInfoMessage("Animation saved");
	}
}

void UserInterface::pushStatusInfoMessage(const char *message)
{
	statusMessage_.format("%s%s", Labels::INFO_MARKER, message);
	lastStatus_ = nc::TimeStamp::now();
}

void UserInterface::pushStatusErrorMessage(const char *message)
{
	statusMessage_.format("%s%s", Labels::ERROR_MARKER, message);
	lastStatus_ = nc::TimeStamp::now();
}

void UserInterface::closeAboutWindow()
{
	showAboutWindow = false;
}

void UserInterface::createGui()
{
	createDockingSpace();

	ImGui::Begin("SpookyGhost");

	createCanvasGui();
	createSpritesGui();
	createAnimationsGui();
	createRenderGui();

	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImGuiCond_FirstUseEver);
	ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_HorizontalScrollbar);// | ImGuiWindowFlags_NoBackground);
	ImGui::Image(canvas_.imguiTexId(), ImVec2(canvas_.texWidth() * canvasZoom_, canvas_.texHeight() * canvasZoom_), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::End();

	if (lastStatus_.secondsSince() >= 2.0f)
		statusMessage_.clear();
	ImGui::Begin("Status");
	ImGui::Text("%s", statusMessage_.data());
	ImGui::End();

	if (showAboutWindow)
	{
		ImGui::Begin("About", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);
#if 0
		ImVec2 cursorPos = ImGui::GetCursorPos();
		cursorPos.x = (ImGui::GetWindowSize().x - 100.0f) * 0.5f;
		ImGui::SetCursorPos(cursorPos);
		ImGui::Image(spriteMgr_.textures()[0]->imguiTexId(), ImVec2(100, 100));
		ImGui::Spacing();
#endif
#ifdef WITH_GIT_VERSION
		ImGui::Text("SpookyGhost %s (%s)", VersionStrings::Version, VersionStrings::GitBranch);
#endif
		ImGui::Text("SpookyGhost compiled on %s at %s", __DATE__, __TIME__);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Based on nCine %s (%s)", nc::VersionStrings::Version, nc::VersionStrings::GitBranch);
		ImGui::Text("nCine compiled on %s at %s", nc::VersionStrings::CompilationDate, nc::VersionStrings::CompilationTime);
		ImGui::End();
	}
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createDockingSpace()
{
	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable == 0)
		return;

	const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
	                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	                                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool open = true;
	ImGui::Begin("DockSpace", &open, windowFlags);

	ImGui::PopStyleVar(3);

	const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	//const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode |
	//                                          ImGuiDockNodeFlags_AutoHideTabBar;
	ImGuiID dockspaceId = ImGui::GetID("TheDockSpace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

	//createInitialDocking(dockspaceId);
	createMenuBar();

	ImGui::End();
}

void UserInterface::createInitialDocking(ImGuiID dockspaceId)
{
	//if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr)
	//	return;

	//ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None); //ImGuiDockNodeFlags_Dockspace
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	//ImGui::DockBuilderSetNodeSize(dockspaceId, ImVec2(1.0f, 1.0f));

	ImGuiID dockMainId = dockspaceId;
	ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.20f, nullptr, &dockMainId);
	ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.20f, nullptr, &dockMainId);

	ImGui::DockBuilderDockWindow("SpookyGhost", dockIdLeft);
	ImGui::DockBuilderDockWindow("Canvas", dockIdRight);
	ImGui::DockBuilderFinish(dockspaceId);
}

void UserInterface::createMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem(Labels::New, "CTRL + N")) { animMgr_.clear(); }
			if (ImGui::MenuItem(Labels::Open)) { pushStatusErrorMessage("Open and Save are not implemented yet"); }
			if (ImGui::MenuItem(Labels::Save)) { pushStatusErrorMessage("Open and Save are not implemented yet"); }
			if (ImGui::MenuItem(Labels::Quit, "CTRL + Q")) { nc::theApplication().quit(); }
			ImGui::EndMenu();
		}
		else if (ImGui::BeginMenu("?"))
		{
			if (ImGui::MenuItem(Labels::About))
				showAboutWindow = true;

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void UserInterface::createCanvasGui()
{
	if (ImGui::CollapsingHeader(Labels::Canvas))
	{
		ImGui::Text("Zoom:");
		ImGui::SameLine();
		if (ImGui::Button("1X"))
			canvasZoom_ = 1.0f;
		ImGui::SameLine();
		if (ImGui::Button("2X"))
			canvasZoom_ = 2.0f;
		ImGui::SameLine();
		if (ImGui::Button("4X"))
			canvasZoom_ = 4.0f;
		ImGui::SameLine();
		if (ImGui::Button("8X"))
			canvasZoom_ = 8.0f;

		nc::Vector2i desiredCanvasSize;
		static int currentComboResize = static_cast<int>(ResizePresetsEnum::SIZE256); // TOD: hard-coded initial state
		ImGui::Combo("Presets", &currentComboResize, resizePresets, IM_ARRAYSIZE(resizePresets));
		if (currentComboResize == ResizePresetsEnum::CUSTOM)
			ImGui::InputInt2("Custom Size", customCanvasSize_.data());
		else
			customCanvasSize_ = canvas_.size();
		switch (currentComboResize)
		{
			case ResizePresetsEnum::SIZE16:
				desiredCanvasSize.set(16, 16);
				break;
			case ResizePresetsEnum::SIZE32:
				desiredCanvasSize.set(32, 32);
				break;
			case ResizePresetsEnum::SIZE64:
				desiredCanvasSize.set(64, 64);
				break;
			case ResizePresetsEnum::SIZE128:
				desiredCanvasSize.set(128, 128);
				break;
			case ResizePresetsEnum::SIZE256:
				desiredCanvasSize.set(256, 256);
				break;
			case ResizePresetsEnum::SIZE512:
				desiredCanvasSize.set(512, 512);
				break;
			case ResizePresetsEnum::CUSTOM:
				desiredCanvasSize = customCanvasSize_;
				break;
		}

		if (desiredCanvasSize.x < 4)
			desiredCanvasSize.x = 4;
		else if (desiredCanvasSize.x > canvas_.maxTextureSize())
			desiredCanvasSize.x = canvas_.maxTextureSize();
		if (desiredCanvasSize.y < 4)
			desiredCanvasSize.y = 4;
		else if (desiredCanvasSize.y > canvas_.maxTextureSize())
			desiredCanvasSize.y = canvas_.maxTextureSize();

		ImGui::SameLine();
		if (ImGui::Button(Labels::Apply) &&
		    (canvas_.size().x != desiredCanvasSize.x || canvas_.size().y != desiredCanvasSize.y))
		{
			canvas_.resizeTexture(desiredCanvasSize);
		}

		ImGui::Text("Size: %d x %d", canvas_.texWidth(), canvas_.texHeight());
		ImGui::ColorEdit4("Background", canvas_.backgroundColor.data(), ImGuiColorEditFlags_AlphaBar);
	}
}

void UserInterface::createSpritesGui()
{
	// TODO: Load image
	// TODO: Set texrect
	// TODO: Show current info like position, rotation, frame, etc...

	static int selectedTextureIndex = 0;
	if (ImGui::CollapsingHeader(Labels::Sprites))
	{
		if (spriteMgr_.textures().isEmpty() == false)
		{
			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.textures().size(); i++)
			{
				Texture &texture = *spriteMgr_.textures()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, texture.name().data(), texture.width(), texture.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Selected Texture", &selectedTextureIndex, comboString_.data());
		}
		ImGui::InputText("Filename", texFilename_.data(), MaxStringLength,
						 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &texFilename_);
		ImGui::SameLine();
		if (ImGui::Button(Labels::Load) && texFilename_.isEmpty() == false)
		{
			if (nc::IFile::access(texFilename_.data(), nc::IFile::AccessMode::READABLE))
			{
				spriteMgr_.textures().pushBack(nctl::makeUnique<Texture>(texFilename_.data()));
				auxString_.format("Loaded texture \"%s\"", texFilename_.data());
			}
			else
				auxString_.format("Cannot load texture \"%s\"", texFilename_.data());

			pushStatusInfoMessage(auxString_.data());
		}

		ImGui::Separator();

		if (ImGui::Button(Labels::New))
		{
			if (selectedTextureIndex >= 0 && selectedTextureIndex < spriteMgr_.textures().size())
			{
				Texture &tex = *spriteMgr_.textures()[selectedTextureIndex];
				spriteMgr_.sprites().pushBack(nctl::makeUnique<Sprite>(&tex));
			}
		}

		if (spriteMgr_.sprites().isEmpty() == false)
		{
			ImGui::SameLine();
			if (ImGui::Button(Labels::Remove))
			{
				if (selectedSpriteIndex_ >= 0 && selectedSpriteIndex_ < spriteMgr_.sprites().size())
					spriteMgr_.sprites().removeAt(selectedSpriteIndex_);
				if (selectedSpriteIndex_ > 0)
					selectedSpriteIndex_--;
			}

			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Selected Sprite", &selectedSpriteIndex_, comboString_.data());

			Sprite &sprite = *spriteMgr_.sprites()[selectedSpriteIndex_];

			Texture &tex = sprite.texture();
			ImGui::Text("Texture: %s (%dx%d)", tex.name().data(), tex.width(), tex.height());

			ImGui::InputText("Name", sprite.name.data(), Sprite::MaxNameLength,
							 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &sprite.name);

			nc::Vector2f position(sprite.x, sprite.y);
			ImGui::SliderFloat2("Position", position.data(), 0.0f, canvas_.texWidth());
			sprite.x = roundf(position.x);
			sprite.y = roundf(position.y);

			ImGui::SliderFloat("Rotation", &sprite.rotation, 0.0f, 360.0f);

			const float texWidth = static_cast<float>(tex.width());
			const float texHeight = static_cast<float>(tex.height());
			ImVec2 size(texWidth * 4, texHeight * 4);
			ImVec2 uv0(0.0f, 0.0f);
			ImVec2 uv1(1.0, 1.0f);
			ImGui::Image(sprite.imguiTexId(), size, uv0, uv1);
		}

#if 0
		Texture &tex = sprite_.texture();
		const float texWidth = static_cast<float>(tex.width());
		const float texHeight = static_cast<float>(tex.height());
		ImVec2 size(texWidth, texHeight);
		ImVec2 uv0(0.0f, 0.0f);
		ImVec2 uv1(1.0, 1.0f);

		nc::Recti texRect = sprite_.texRect();
		size = ImVec2(texRect.w, texRect.h);
		uv0.x = texRect.x / texWidth;
		uv0.y = texRect.y / texHeight;
		uv1.x = (texRect.x + texRect.w) / texWidth;
		uv1.y = (texRect.y + texRect.h) / texHeight;

		bool isFlippedX = sprite_.isFlippedX();
		if (isFlippedX)
			nctl::swap(uv0.x, uv1.x);
		bool isFlippedY = sprite_.isFlippedY();
		if (isFlippedY)
			nctl::swap(uv0.y, uv1.y);


		ImGui::Image(sprite_.imguiTexId(), size, uv0, uv1);

		int minX = texRect.x;
		int maxX = minX + texRect.w;
		ImGui::DragIntRange2("Rect X", &minX, &maxX, 1.0f, 0, tex.width());
		int minY = texRect.y;
		int maxY = minY + texRect.h;
		ImGui::DragIntRange2("Rect Y", &minY, &maxY, 1.0f, 0, tex.height());
		texRect.x = minX;
		texRect.w = maxX - minX;
		texRect.y = minY;
		texRect.h = maxY - minY;
		ImGui::SameLine();

		if (ImGui::Button("Reset"))
			texRect = nc::Recti(0, 0, tex.width(), tex.height());

		if (s.texRect.x != spriteState_.texRect.x || s.texRect.y != spriteState_.texRect.y ||
		    s.texRect.w != spriteState_.texRect.w || s.texRect.h != spriteState_.texRect.h)
		{
			particleSystem->setTexRect(spriteState_.texRect);
			s.texRect = spriteState_.texRect;
		}

		ImGui::Checkbox("Flipped X", &isFlippedX);
		ImGui::SameLine();
		ImGui::Checkbox("Flipped Y", &isFlippedY);

		if (sprite_.isFlippedX() != isFlippedX)
			sprite_.setFlippedX(isFlippedX);
		if (sprite_.isFlippedY() != isFlippedY)
			sprite_.setFlippedY(isFlippedY);

		ImGui::SliderFloat2("Anchor Point", spriteState_.anchorPoint.data(), 0.0f, 1.0f);
		static int currentAnchorSelection = 0;
		if (ImGui::Combo("Anchor Presets", &currentAnchorSelection, anchorPointItems, IM_ARRAYSIZE(anchorPointItems)))
		{
			switch (currentAnchorSelection)
			{
				case 0:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorCenter;
					break;
				case 1:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorBottomLeft;
					break;
				case 2:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorTopLeft;
					break;
				case 3:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorBottomRight;
					break;
				case 4:
					spriteState_.anchorPoint = nc::BaseSprite::AnchorTopRight;
					break;
			}
		}
		if (s.anchorPoint.x != spriteState_.anchorPoint.x ||
		    s.anchorPoint.y != spriteState_.anchorPoint.y)
		{
			particleSystem->setAnchorPoint(spriteState_.anchorPoint);
			s.anchorPoint = spriteState_.anchorPoint;
		}
#endif
	}
}

void UserInterface::createAnimationsGui()
{
	if (ImGui::CollapsingHeader(Labels::Animations))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button(Labels::Add))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
					animMgr_.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				case AnimationTypesEnum::PROPERTY:
					animMgr_.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				case AnimationTypesEnum::GRID:
					animMgr_.anims().pushBack(nctl::makeUnique<GridAnimation>());
					break;
			}
		}

		ImGui::SameLine();
		if (ImGui::Button(Labels::Clear))
			animMgr_.clear();
		ImGui::Separator();

		for (unsigned int i = 0; i < animMgr_.anims().size(); i++)
			createRecursiveAnimationsGui(*animMgr_.anims()[i].get());
	}
}

void UserInterface::createRenderGui()
{
	if (ImGui::CollapsingHeader(Labels::Render))
	{
		ImGui::InputText("Filename prefix", animFilename_.data(), MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &animFilename_);

		int fps = static_cast<int>(saveAnimStatus_.fps);
		ImGui::InputInt("FPS", &fps);
		saveAnimStatus_.fps = static_cast<float>(fps);
		ImGui::SliderInt("Num Frames", &saveAnimStatus_.numFrames, 1, 10 * saveAnimStatus_.fps); // Hard-coded limit
		float duration = saveAnimStatus_.numFrames * saveAnimStatus_.inverseFps();
		ImGui::SliderFloat("Duration", &duration, 0.0f, 10.0f, "%.3fs"); // Hard-coded limit

		saveAnimStatus_.numFrames = duration * saveAnimStatus_.fps;
		if (saveAnimStatus_.numFrames < 1)
			saveAnimStatus_.numFrames = 1;

		if (shouldSaveAnim_)
		{
			const unsigned int numSavedFrames = saveAnimStatus_.numSavedFrames;
			const float fraction = numSavedFrames / static_cast<float>(saveAnimStatus_.numFrames);
			auxString_.format("Frame: %d/%d", numSavedFrames, saveAnimStatus_.numFrames);
			ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), auxString_.data());
		}
		else if (ImGui::Button("Save to PNG") && shouldSaveAnim_ == false)
		{
			if (animFilename_.isEmpty())
				pushStatusErrorMessage("Set a filename prefix before saving an animation");
			else
			{
				saveAnimStatus_.filename.format("%s_%03d.png", animFilename_.data(), saveAnimStatus_.numSavedFrames);
				shouldSaveAnim_ = true;
			}
		}
	}
}

void UserInterface::createRecursiveAnimationsGui(IAnimation &anim)
{
	ImGui::PushID(reinterpret_cast<const void*>(&anim));
	switch (anim.type())
	{
		case IAnimation::Type::PROPERTY:
		{
			PropertyAnimation &propertyAnim = static_cast<PropertyAnimation &>(anim);
			createPropertyAnimationGui(propertyAnim);
			break;
		}
		case IAnimation::Type::GRID:
		{
			GridAnimation &gridAnim = static_cast<GridAnimation &>(anim);
			createGridAnimationGui(gridAnim);
			break;
		}
		case IAnimation::Type::PARALLEL_GROUP:
		{
			AnimationGroup &animGroup = static_cast<AnimationGroup &>(anim);
			createAnimationGroupGui(animGroup);
			break;
		}
		case IAnimation::Type::SEQUENTIAL_GROUP:
		{
			AnimationGroup &animGroup = static_cast<AnimationGroup &>(anim);
			createAnimationGroupGui(animGroup);
			break;
		}
	}
	ImGui::PopID();
}

void UserInterface::createAnimationGroupGui(AnimationGroup &animGroup)
{
	ASSERT(animGroup.type() == IAnimation::Type::PARALLEL_GROUP ||
	       animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP);

	if (animGroup.type() == IAnimation::Type::PARALLEL_GROUP)
		auxString_ = "Parallel Animation";
	else if (animGroup.type() == IAnimation::Type::SEQUENTIAL_GROUP)
		auxString_ = "Sequential Animation";

	if (ImGui::TreeNodeEx(auxString_.data(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int currentComboAnimType = 0;
		ImGui::Combo("Type", &currentComboAnimType, animationTypes, IM_ARRAYSIZE(animationTypes));
		ImGui::SameLine();
		if (ImGui::Button(Labels::Add))
		{
			switch (currentComboAnimType)
			{
				case AnimationTypesEnum::PARALLEL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<ParallelAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::SEQUENTIAL_GROUP:
				{
					animGroup.anims().pushBack(nctl::makeUnique<SequentialAnimationGroup>());
					break;
				}
				case AnimationTypesEnum::PROPERTY:
				{
					animGroup.anims().pushBack(nctl::makeUnique<PropertyAnimation>());
					break;
				}
				case AnimationTypesEnum::GRID:
				{
					animGroup.anims().pushBack(nctl::makeUnique<GridAnimation>());
					break;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(Labels::Clear))
		{
			animGroup.stop();
			animGroup.anims().clear();
		}

		ImGui::Text("State: %s", animStateToString(animGroup.state()));
		if (ImGui::Button(Labels::Stop))
			animGroup.stop();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Pause))
			animGroup.pause();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Play))
			animGroup.play();
		ImGui::Separator();

		for (unsigned int i = 0; i < animGroup.anims().size(); i++)
		{
			createRecursiveAnimationsGui(*animGroup.anims()[i].get());
			auxString_.format("%s##%u", Labels::Remove, i);
			if (ImGui::Button(auxString_.data()))
			{
				animGroup.anims()[i]->stop();
				animGroup.anims().removeAt(i);
			}
		}

		ImGui::TreePop();
	}
}

void UserInterface::createPropertyAnimationGui(PropertyAnimation &anim)
{
	ASSERT(anim.type() == IAnimation::Type::PROPERTY);

	if (ImGui::TreeNodeEx("Property Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int spriteIndex = 0;
		if (spriteMgr_.sprites().isEmpty() == false)
		{
			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Sprites", &spriteIndex, comboString_.data());

			Sprite &sprite = *spriteMgr_.sprites()[spriteIndex];

			static int currentComboProperty = -1;
			const nctl::String &propertyName = anim.propertyName();
			currentComboProperty = PropertyTypesEnum::NONE;
			if (anim.property() != nullptr)
			{
				for (unsigned int i = 0; i < IM_ARRAYSIZE(propertyTypes); i++)
				{
					if (propertyName == propertyTypes[i])
					{
						currentComboProperty = static_cast<PropertyTypesEnum>(i);
						break;
					}
				}
			}
			ImGui::Combo("Property", &currentComboProperty, propertyTypes, IM_ARRAYSIZE(propertyTypes));
			anim.setPropertyName(propertyTypes[currentComboProperty]);
			switch (currentComboProperty)
			{
				case PropertyTypesEnum::NONE:
					break;
				case PropertyTypesEnum::POSITION_X:
					anim.setProperty(&sprite.x);
					break;
				case PropertyTypesEnum::POSITION_Y:
					anim.setProperty(&sprite.y);
					break;
				case PropertyTypesEnum::ROTATION:
					anim.setProperty(&sprite.rotation);
					break;
			}
		}
		else
			ImGui::TextDisabled("No sprite currently loaded");

		int currentComboCurveType = static_cast<int>(anim.curve().type());
		ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
		anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

		int currentComboLoopMode = static_cast<int>(anim.curve().loopMode());
		ImGui::Combo("Loop Mode", &currentComboLoopMode, easingCurveLoopModes, IM_ARRAYSIZE(easingCurveLoopModes));
		anim.curve().setLoopMode(static_cast<EasingCurve::LoopMode>(currentComboLoopMode));

		ImGui::SliderFloat("Shift", &anim.curve().shift(), -500.0f, 500.0f);
		ImGui::SameLine();
		auxString_.format("%s##Shift", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.curve().shift() = 0.0f;
		ImGui::SliderFloat("Scale", &anim.curve().scale(), -500.0f, 500.0f);
		ImGui::SameLine();
		auxString_.format("%s##Scale", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.curve().scale() = 1.0f;

		ImGui::Separator();
		ImGui::SliderFloat("Speed", &anim.speed(), 0.0f, 5.0f);
		ImGui::SameLine();
		auxString_.format("%s##Speed", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.speed() = 1.0f;
		ImGui::SliderFloat("Start", &anim.curve().start(), 0.0f, 1.0f);
		ImGui::SliderFloat("End", &anim.curve().end(), 0.0f, 1.0f);
		ImGui::SliderFloat("Time", &anim.curve().time(), anim.curve().start(), anim.curve().end());

		if (anim.curve().start() > anim.curve().end() ||
		    anim.curve().end() < anim.curve().start())
		{
			anim.curve().start() = anim.curve().end();
		}
		if (anim.curve().time() < anim.curve().start())
			anim.curve().time() = anim.curve().start();
		else if (anim.curve().time() > anim.curve().end())
			anim.curve().time() = anim.curve().end();

		ImGui::Text("State: %s", animStateToString(anim.state()));
		if (ImGui::Button(Labels::Stop))
			anim.stop();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Pause))
			anim.pause();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Play))
			anim.play();

		ImGui::TreePop();
	}
}

void UserInterface::createGridAnimationGui(GridAnimation &anim)
{
	ASSERT(anim.type() == IAnimation::Type::GRID);

	if (ImGui::TreeNodeEx("Grid Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int spriteIndex = spriteMgr_.spriteIndex(anim.sprite());
		if (spriteMgr_.sprites().isEmpty() == false)
		{
			if (spriteIndex < 0)
				spriteIndex = 0;

			comboString_.clear();
			for (unsigned int i = 0; i < spriteMgr_.sprites().size(); i++)
			{
				Sprite &sprite = *spriteMgr_.sprites()[i];
				comboString_.formatAppend("#%u: %s (%d x %d)", i, sprite.name.data(), sprite.width(), sprite.height());
				comboString_.setLength(comboString_.length() + 1);
			}
			comboString_.setLength(comboString_.length() + 1);
			// Append a second '\0' to signal the end of the combo item list
			comboString_[comboString_.length() - 1] = '\0';

			ImGui::Combo("Sprites", &spriteIndex, comboString_.data());
			anim.setSprite(spriteMgr_.sprites()[spriteIndex].get());
		}
		else
			ImGui::TextDisabled("No sprite currently loaded");

		static int currentComboType = -1;
		currentComboType = static_cast<int>(anim.gridAnimationType());
		ImGui::Combo("Type", &currentComboType, gridAnimTypes, IM_ARRAYSIZE(gridAnimTypes));
		anim.setGridAnimationType(static_cast<GridAnimation::AnimationType>(currentComboType));

		int currentComboCurveType = static_cast<int>(anim.curve().type());
		ImGui::Combo("Easing Curve", &currentComboCurveType, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
		anim.curve().setType(static_cast<EasingCurve::Type>(currentComboCurveType));

		int currentComboLoopMode = static_cast<int>(anim.curve().loopMode());
		ImGui::Combo("Loop Mode", &currentComboLoopMode, easingCurveLoopModes, IM_ARRAYSIZE(easingCurveLoopModes));
		anim.curve().setLoopMode(static_cast<EasingCurve::LoopMode>(currentComboLoopMode));

		ImGui::SliderFloat("Shift", &anim.curve().shift(), -500.0f, 500.0f);
		ImGui::SameLine();
		auxString_.format("%s##Shift", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.curve().shift() = 0.0f;
		ImGui::SliderFloat("Scale", &anim.curve().scale(), -500.0f, 500.0f);
		ImGui::SameLine();
		auxString_.format("%s##Scale", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.curve().scale() = 1.0f;

		ImGui::Separator();
		ImGui::SliderFloat("Speed", &anim.speed(), 0.0f, 5.0f);
		ImGui::SameLine();
		auxString_.format("%s##Speed", Labels::Reset);
		if (ImGui::Button(auxString_.data()))
			anim.speed() = 1.0f;
		ImGui::SliderFloat("Start", &anim.curve().start(), 0.0f, 1.0f);
		ImGui::SliderFloat("End", &anim.curve().end(), 0.0f, 1.0f);
		ImGui::SliderFloat("Time", &anim.curve().time(), anim.curve().start(), anim.curve().end());

		if (anim.curve().start() > anim.curve().end() ||
		    anim.curve().end() < anim.curve().start())
		{
			anim.curve().start() = anim.curve().end();
		}
		if (anim.curve().time() < anim.curve().start())
			anim.curve().time() = anim.curve().start();
		else if (anim.curve().time() > anim.curve().end())
			anim.curve().time() = anim.curve().end();

		ImGui::Text("State: %s", animStateToString(anim.state()));
		if (ImGui::Button(Labels::Stop))
			anim.stop();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Pause))
			anim.pause();
		ImGui::SameLine();
		if (ImGui::Button(Labels::Play))
			anim.play();

		ImGui::TreePop();
	}
}
