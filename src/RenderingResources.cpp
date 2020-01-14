#include "RenderingResources.h"
#include <ncine/GLShaderProgram.h>
#include <ncine/Application.h>
#include <ncine/AppConfiguration.h>
#include <ncine/IFile.h> // for dataPath()

#include "shader_strings.h"

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

nctl::UniquePtr<nc::GLShaderProgram> RenderingResources::spriteShaderProgram_;
nctl::UniquePtr<nc::GLShaderProgram> RenderingResources::meshSpriteShaderProgram_;

nc::Vector2f RenderingResources::canvasSize_(0.0f, 0.0f);
nc::Matrix4x4f RenderingResources::projectionMatrix_ = nc::Matrix4x4f::Identity;

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

void RenderingResources::setCanvasSize(int width, int height)
{
	canvasSize_.set(width, height);
	projectionMatrix_ = nc::Matrix4x4f::ortho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
}

void RenderingResources::create()
{
	LOGI("Creating rendering resources...");

	const nc::AppConfiguration &appCfg = nc::theApplication().appConfiguration();

	ShaderLoad shadersToLoad[] = {
		{ RenderingResources::spriteShaderProgram_, ShaderStrings::sprite_vs, ShaderStrings::sprite_fs, nc::GLShaderProgram::Introspection::ENABLED },
		{ RenderingResources::meshSpriteShaderProgram_, ShaderStrings::meshsprite_vs, ShaderStrings::sprite_fs, nc::GLShaderProgram::Introspection::ENABLED },
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

void RenderingResources::dispose()
{
	meshSpriteShaderProgram_.reset(nullptr);
	spriteShaderProgram_.reset(nullptr);

	LOGI("Rendering resources disposed");
}
