#include "RenderResources.h"
#include <ncine/Application.h>
#include <ncine/AppConfiguration.h>
#include <ncine/IFile.h> // for dataPath()

#include "shader_strings.h"

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<nc::GLShaderProgram> RenderResources::textureShaderProgram_;
nctl::UniquePtr<nc::GLShaderProgram> RenderResources::spriteShaderProgram_;
nctl::UniquePtr<nc::GLShaderProgram> RenderResources::meshSpriteShaderProgram_;

nc::Vector2f RenderResources::canvasSize_(0.0f, 0.0f);
nc::Matrix4x4f RenderResources::projectionMatrix_ = nc::Matrix4x4f::Identity;

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

namespace {

	struct ShaderLoad
	{
		nctl::UniquePtr<nc::GLShaderProgram> &shaderProgram;
		const char *vertexShader;
		const char *fragmentShader;
		nc::GLShaderProgram::Introspection introspection;
	};

}

void RenderResources::setCanvasSize(int width, int height)
{
	canvasSize_.set(width, height);
	projectionMatrix_ = nc::Matrix4x4f::ortho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
}

void RenderResources::create()
{
	LOGI("Creating rendering resources...");

	const nc::AppConfiguration &appCfg = nc::theApplication().appConfiguration();

	ShaderLoad shadersToLoad[] = {
		{ RenderResources::textureShaderProgram_, ShaderStrings::texture_vs, ShaderStrings::texture_fs, nc::GLShaderProgram::Introspection::ENABLED },
		{ RenderResources::spriteShaderProgram_, ShaderStrings::sprite_vs, ShaderStrings::sprite_fs, nc::GLShaderProgram::Introspection::ENABLED },
	    { RenderResources::meshSpriteShaderProgram_, ShaderStrings::meshsprite_vs, ShaderStrings::sprite_fs, nc::GLShaderProgram::Introspection::ENABLED },
	};

	const nc::GLShaderProgram::QueryPhase queryPhase = appCfg.deferShaderQueries ? nc::GLShaderProgram::QueryPhase::DEFERRED : nc::GLShaderProgram::QueryPhase::IMMEDIATE;
	const unsigned int numShaderToLoad = (sizeof(shadersToLoad) / sizeof(*shadersToLoad));
	for (unsigned int i = 0; i < numShaderToLoad; i++)
	{
		const ShaderLoad &shaderToLoad = shadersToLoad[i];

		shaderToLoad.shaderProgram = nctl::makeUnique<nc::GLShaderProgram>(queryPhase);
		shaderToLoad.shaderProgram->attachShaderFromString(GL_VERTEX_SHADER, shaderToLoad.vertexShader);
		shaderToLoad.shaderProgram->attachShaderFromString(GL_FRAGMENT_SHADER, shaderToLoad.fragmentShader);
		shaderToLoad.shaderProgram->link(shaderToLoad.introspection);
		FATAL_ASSERT(shaderToLoad.shaderProgram->status() != nc::GLShaderProgram::Status::LINKING_FAILED);
	}

	LOGI("Rendering resources created");
}

void RenderResources::dispose()
{
	meshSpriteShaderProgram_.reset(nullptr);
	spriteShaderProgram_.reset(nullptr);
	textureShaderProgram_.reset(nullptr);

	LOGI("Rendering resources disposed");
}
