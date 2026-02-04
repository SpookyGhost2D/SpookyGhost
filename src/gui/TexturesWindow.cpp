#include <ncine/imgui_internal.h>
#include <ncine/InputEvents.h>

#include "gui/TexturesWindow.h"
#include "singletons.h"
#include "gui/gui_labels.h"
#include "gui/UserInterface.h"
#include "gui/FileDialog.h"
#include "AnimationManager.h"
#include "SpriteManager.h"
#include "SpriteEntry.h"
#include "Sprite.h"
#include "Texture.h"

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	#include "textures_strings.h"
#endif

namespace {

void openReloadTextureDialog()
{
	FileDialog::config.directory = theCfg.texturesPath;
	FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
	FileDialog::config.windowTitle = "Reload texture file";
	FileDialog::config.okButton = Labels::Ok;
	FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
	FileDialog::config.extensions = "png\0\0";
	FileDialog::config.action = FileDialog::Action::RELOAD_TEXTURE;
	FileDialog::config.windowOpen = true;
}

}

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

TexturesWindow::TexturesWindow(UserInterface &ui)
    : ui_(ui)
{
#ifdef __EMSCRIPTEN__
	loadTextureLocalFile_.setLoadedCallback([](const nc::EmscriptenLocalFile &localFile, void *userData) {
		TexturesWindow *tw = reinterpret_cast<TexturesWindow *>(userData);
		if (localFile.size() > 0)
			tw->loadTexture(localFile.filename(), localFile.data(), localFile.size());
	}, this);

	reloadTextureLocalFile_.setLoadedCallback([](const nc::EmscriptenLocalFile &localFile, void *userData) {
		TexturesWindow *tw = reinterpret_cast<TexturesWindow *>(userData);
		if (localFile.size() > 0)
			tw->reloadTexture(localFile.filename(), localFile.data(), localFile.size());
	}, this);
#endif
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void TexturesWindow::create()
{
	ImGui::Begin(Labels::Textures);

	if (ImGui::IsWindowHovered() && ui::dropEvent != nullptr)
	{
		for (unsigned int i = 0; i < ui::dropEvent->numPaths; i++)
			loadTexture(ui::dropEvent->paths[i]);
		ui::dropEvent = nullptr;
	}

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
	const bool openBundledEnabled = TexturesStrings::Count > 0;
	if (openBundledEnabled)
	{
		ui::comboString.clear();
		ui::comboString.append(Labels::BundledTextures);
		ui::comboString.setLength(ui::comboString.length() + 1);
		for (unsigned int i = 0; i < TexturesStrings::Count; i++)
		{
			ui::comboString.formatAppend("%s", TexturesStrings::Names[i]);
			ui::comboString.setLength(ui::comboString.length() + 1);
		}
		ui::comboString.setLength(ui::comboString.length() + 1);
		// Append a second '\0' to signal the end of the combo item list
		ui::comboString[ui::comboString.length() - 1] = '\0';

		static int currentComboTexture = 0;
		if (ImGui::Combo("###BundledTextures", &currentComboTexture, ui::comboString.data()) && currentComboTexture > 0)
		{
			loadTexture(nc::fs::joinPath(ui::texturesDataDir, TexturesStrings::Names[currentComboTexture - 1]).data());
			currentComboTexture = 0;
		}
	}
#endif

	if (ImGui::Button(Labels::Load))
	{
#ifndef __EMSCRIPTEN__
		FileDialog::config.directory = theCfg.texturesPath;
		FileDialog::config.windowIcon = Labels::FileDialog_OpenIcon;
		FileDialog::config.windowTitle = "Load texture file";
		FileDialog::config.okButton = Labels::Ok;
		FileDialog::config.selectionType = FileDialog::SelectionType::FILE;
		FileDialog::config.extensions = "png\0\0";
		FileDialog::config.action = FileDialog::Action::LOAD_TEXTURE;
		FileDialog::config.windowOpen = true;
#else
		loadTextureLocalFile_.load();
#endif
	}

	const bool enableButtons = theSpriteMgr->textures().isEmpty() == false;
	ImGui::BeginDisabled(enableButtons == false);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Remove) || (ui_.deleteKeyPressed_ && ImGui::IsWindowHovered()))
		removeTexture();
	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
	if (ImGui::Button(Labels::Reload))
	{
#ifndef __EMSCRIPTEN__
		openReloadTextureDialog();
#else
		reloadTextureLocalFile_.load();
#endif
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	if (theSpriteMgr->textures().isEmpty() == false)
	{
		for (unsigned int i = 0; i < theSpriteMgr->textures().size(); i++)
		{
			Texture &texture = *theSpriteMgr->textures()[i];
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (i == ui_.selectedTextureIndex_)
				nodeFlags |= ImGuiTreeNodeFlags_Selected;

			ImGui::TreeNodeEx(static_cast<void *>(&texture), nodeFlags, "#%u: \"%s\" (%d x %d)",
			                  i, texture.name().data(), texture.width(), texture.height());

			if (ImGui::IsItemHovered() && ImGui::GetCurrentContext()->HoveredIdTimer > ui::ImageTooltipDelay)
			{
				if (texture.width() > 0 && texture.height() > 0)
				{
					const float aspectRatio = texture.width() / static_cast<float>(texture.height());
					float width = ui::ImageTooltipSize;
					float height = ui::ImageTooltipSize;
					if (aspectRatio > 1.0f)
						height = width / aspectRatio;
					else
						width *= aspectRatio;

					ImGui::BeginTooltip();
					ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<intptr_t>(texture.imguiTexId())), ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
					ImGui::EndTooltip();
				}
			}
			if (ImGui::IsItemClicked())
				ui_.selectedTextureIndex_ = i;

			if (ImGui::BeginPopupContextItem())
			{
				ui_.selectedTextureIndex_ = i;

				if (ImGui::MenuItem(Labels::Reload))
				{
#ifndef __EMSCRIPTEN__
					openReloadTextureDialog();
#else
					reloadTextureLocalFile_.load();
#endif
				}
				if (ImGui::MenuItem(Labels::Remove))
					removeTexture();
				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

bool TexturesWindow::loadTexture(const char *filename)
{
	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>(filename);
	const bool hasLoaded = postLoadTexture(texture, filename);

	if (texture->dataSize() > 0)
	{
		theSpriteMgr->textures().pushBack(nctl::move(texture));
		ui_.selectedTextureIndex_ = theSpriteMgr->textures().size() - 1;
	}
	return hasLoaded;
}

bool TexturesWindow::reloadTexture(const char *filename)
{
	nctl::UniquePtr<Texture> &texture = theSpriteMgr->textures()[ui_.selectedTextureIndex_];
	texture->loadFromFile(filename);
	return postLoadTexture(texture, filename);
}

#ifdef __EMSCRIPTEN__
bool TexturesWindow::loadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize)
{
	nctl::UniquePtr<Texture> texture = nctl::makeUnique<Texture>(bufferName, reinterpret_cast<const unsigned char *>(bufferPtr), bufferSize);
	const bool hasLoaded = postLoadTexture(texture, bufferName);

	if (texture->dataSize() > 0)
	{
		theSpriteMgr->textures().pushBack(nctl::move(texture));
		ui_.selectedTextureIndex_ = theSpriteMgr->textures().size() - 1;
	}
	return hasLoaded;
}

bool TexturesWindow::reloadTexture(const char *bufferName, const char *bufferPtr, unsigned long int bufferSize)
{
	nctl::UniquePtr<Texture> &texture = theSpriteMgr->textures()[ui_.selectedTextureIndex_];
	texture->loadFromMemory(bufferName, reinterpret_cast<const unsigned char *>(bufferPtr), bufferSize);
	return postLoadTexture(texture, bufferName);
}
#endif

///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

bool TexturesWindow::postLoadTexture(nctl::UniquePtr<Texture> &texture, const char *name)
{
	if (texture->dataSize() > 0)
	{
		// Check if the texture is in the configuration path or in the data directory
		const nctl::String baseName = nc::fs::baseName(name);
		const nctl::String nameInConfigDir = nc::fs::joinPath(theCfg.texturesPath, baseName);
		const nctl::String nameInDataDir = nc::fs::joinPath(ui::texturesDataDir, baseName);
		if ((nameInConfigDir == name && nc::fs::isReadableFile(nameInConfigDir.data())) ||
		    (nameInDataDir == name && nc::fs::isReadableFile(nameInDataDir.data())))
		{
			// Set the texture name to its basename to allow for relocatable project files
			texture->setName(baseName);
		}

		ui::auxString.format("Loaded texture \"%s\"", name);
		ui_.pushStatusInfoMessage(ui::auxString.data());

		return true;
	}
	else
	{
		ui::auxString.format("Cannot load texture \"%s\"", name);
		ui_.pushStatusErrorMessage(ui::auxString.data());

		return false;
	}
}

void TexturesWindow::recursiveRemoveSpriteWithTexture(SpriteGroup &group, Texture &tex)
{
	// Deleting backwards without iterators
	for (int i = group.children().size() - 1; i >= 0; i--)
	{
		Sprite *sprite = group.children()[i]->toSprite();
		SpriteGroup *spriteGroup = group.children()[i]->toGroup();
		if (sprite && &sprite->texture() == &tex)
		{
			ui_.updateSelectedAnimOnSpriteRemoval(sprite);
			theAnimMgr->removeSprite(sprite);
			group.children().removeAt(i);
		}
		else if (spriteGroup)
			recursiveRemoveSpriteWithTexture(group, tex);
	}
}

void TexturesWindow::removeTexture()
{
	recursiveRemoveSpriteWithTexture(theSpriteMgr->root(), *theSpriteMgr->textures()[ui_.selectedTextureIndex_]);

	theSpriteMgr->textures().removeAt(ui_.selectedTextureIndex_);
	theSpriteMgr->updateSpritesArray();
	if (ui_.selectedTextureIndex_ > 0)
		ui_.selectedTextureIndex_--;
}
