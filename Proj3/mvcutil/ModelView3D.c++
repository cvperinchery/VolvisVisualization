// ModelView3D.c++

#include "Controller.h"
#include "ModelView3D.h"
#include "ShaderIF.h"

// shader program id and locations of attribute and uniform variables inside it
GLuint ModelView3D::shaderProgram = 0;
int ModelView3D::numInstances = 0;
ShaderIF* ModelView3D::shaderIF = NULL;

std::string ModelView3D::vShaderSource = "mostExcellent.vsh";
std::string ModelView3D::fShaderSource = "mostExcellent.fsh";

// Attributes that are always per-vertex:
GLint ModelView3D::pvaLoc_mcPosition = -2,
      ModelView3D::pvaLoc_mcNormal = -2;

// Attributes that are always per-primitive:
GLint ModelView3D::ppuLoc_mc_ec = -2,
      ModelView3D::ppuLoc_ec_lds = -2;

ModelView3D::ModelView3D()
{
	if (ModelView3D::shaderIF == NULL)
	{
		ModelView3D::shaderIF = new ShaderIF(vShaderSource, fShaderSource);
		ModelView3D::shaderProgram = shaderIF->getShaderPgmID();
		fetchGLSLVariableLocations();
	}
	ModelView3D::numInstances++;
}

ModelView3D::~ModelView3D()
{
	if (--ModelView3D::numInstances == 0)
	{
		ModelView3D::shaderIF->destroy();
		delete ModelView3D::shaderIF;
		ModelView3D::shaderIF = NULL;
		ModelView3D::shaderProgram = 0;
	}
}

void ModelView3D::fetchGLSLVariableLocations()
{
	if (ModelView3D::shaderProgram > 0)
	{
		// record attribute locations
		ModelView3D::pvaLoc_mcPosition = pvAttribLocation(shaderProgram, "mcPosition");
		ModelView3D::pvaLoc_mcNormal = pvAttribLocation(shaderProgram, "mcNormal");
		// record uniform locations
		ModelView3D::ppuLoc_mc_ec = ppUniformLocation(shaderProgram, "mc_ec");
		ModelView3D::ppuLoc_ec_lds = ppUniformLocation(shaderProgram, "ec_lds");
	}
}

void ModelView3D::setShaderSources(const std::string& vShader, const std::string& fShader)
{
	vShaderSource = vShader;
	fShaderSource = fShader;
}
