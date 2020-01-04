#include <ncine/imgui.h>

#include "UserInterface.h"
#include "Canvas.h"
#include "AnimationManager.h"
//#include "LuaSerializer.h"

namespace {

const char *easingCurveTypes[] = { "Linear", "Quad" };
const char *easingCurveLoopModes[] = { "Disabled", "Rewind", "Ping Pong" };


int inputTextCallback(ImGuiInputTextCallbackData *data)
{
	nctl::String *string = reinterpret_cast<nctl::String *>(data->UserData);
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		ASSERT(data->Buf == string->data());
		string->setLength(data->BufTextLen);
		data->Buf = string->data();
	}
	return 0;
}

const char *animStateToString(Animation::State state)
{
	switch (state)
	{
		case Animation::State::STOPPED: return "Stopped";
		case Animation::State::PAUSED: return "Paused";
		case Animation::State::PLAYING: return "Playing";
	}
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

UserInterface::UserInterface(Canvas &canvas, AnimationManager &animMgr)
    : auxString_(MaxStringLength), filename_(MaxStringLength),
      canvas_(canvas), animMgr_(animMgr)
{
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

#ifdef __ANDROID__
	io.FontGlobalScale = 2.0f;
#endif
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void UserInterface::createGuiMainWindow()
{
	ImGui::Begin("SpookyGhost");

	if (ImGui::CollapsingHeader("Texture"))
	{
		ImGui::Text("Size: %d x %d", canvas_.texWidth(), canvas_.texHeight());
		//ImGui::Combo("Upload Mode", &vfConf.textureUploadMode, "glTexSubImage2D\0Pixel Buffer Object\0PBO with Mapping\0\0");
		//ImGui::SliderFloat("Delay", &vfConf.textureCopyDelay, 0.0f, 60.0f, "%.1f");
		//ImGui::SameLine();
		//ImGui::Checkbox("Enabled", &vfConf.progressiveCopy);
	}

	if (ImGui::CollapsingHeader("Animations"))
	{
		for (unsigned int i = 0; i < animMgr_.anims().size(); i++)
		{
			auxString_.format("Animation %u", i);
			if (ImGui::TreeNodeEx(auxString_.data(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				Animation &anim = animMgr_.anims()[i];
				int type = static_cast<int>(anim.curve().type());
				ImGui::Combo("Easing Curve", &type, easingCurveTypes, IM_ARRAYSIZE(easingCurveTypes));
				anim.curve().setType(static_cast<EasingCurve::Type>(type));

				int loopMode = static_cast<int>(anim.curve().loopMode());
				ImGui::Combo("Loop Mode", &loopMode, easingCurveLoopModes, IM_ARRAYSIZE(easingCurveLoopModes));
				anim.curve().setLoopMode(static_cast<EasingCurve::LoopMode>(loopMode));

				ImGui::SliderFloat("C0", &anim.curve().coeff0(), 0.0f, 500.0f);
				ImGui::SliderFloat("C1", &anim.curve().coeff1(), 0.0f, 500.0f);
				ImGui::SliderFloat("C2", &anim.curve().coeff2(), 0.0f, 500.0f);

				ImGui::Separator();
				float time = anim.curve().time();
				ImGui::SliderFloat("Time", &time, 0.0f, 1.0f);
				anim.curve().setTime(time);

				ImGui::Text("State: %s", animStateToString(anim.state()));
				if (ImGui::Button("Stop"))
					anim.stop();
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
					anim.pause();
				ImGui::SameLine();
				if (ImGui::Button("Play"))
					anim.play();

				ImGui::TreePop();
			}
		}
	}

	if (ImGui::CollapsingHeader("Render"))
	{
		ImGui::InputText("Filename prefix", filename_.data(), MaxStringLength,
		                 ImGuiInputTextFlags_CallbackResize, inputTextCallback, &filename_);

		if (ImGui::Button("Save to PNG"))
		{
			// TODO: implement
		}
	}

	ImGui::End();

	static float canvasZoom = 1.0f;
	bool open = true;
	ImGui::SetNextWindowSize(ImVec2(canvas_.texWidth() * canvasZoom, canvas_.texHeight() * canvasZoom), ImGuiCond_FirstUseEver);
	ImGui::Begin("Canvas", &open, ImGuiWindowFlags_HorizontalScrollbar);// | ImGuiWindowFlags_NoBackground);
	if (ImGui::Button("1X"))
		canvasZoom = 1.0f;
	ImGui::SameLine();
	if (ImGui::Button("2X"))
		canvasZoom = 2.0f;
	ImGui::SameLine();
	if (ImGui::Button("4X"))
		canvasZoom = 4.0f;
	ImGui::Separator();
	ImGui::Image(canvas_.imguiTexId(), ImVec2(canvas_.texWidth() * canvasZoom, canvas_.texHeight() * canvasZoom), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::End();
}

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

