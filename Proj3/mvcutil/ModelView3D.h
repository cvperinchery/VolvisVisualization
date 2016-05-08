// ModelView3D.h

#ifndef MODELVIEW3D_H
#define MODELVIEW3D_H

#include "ModelView.h"

class ShaderIF;

class ModelView3D : public ModelView
{
public:
	ModelView3D();
	virtual ~ModelView3D();

	// Alter location of shader files
	static void setShaderSources(const std::string& vShader, const std::string& fShader);

protected:
	static ShaderIF* shaderIF;
	static GLuint shaderProgram;
	static int numInstances;

	// GLSL per-vertex attributes for geometry
	static GLint pvaLoc_mcPosition, pvaLoc_mcNormal;

	// Other GLSL per-primitive attributes
	static GLint ppuLoc_mc_ec, ppuLoc_ec_lds;

private:
	static void fetchGLSLVariableLocations();
	static std::string vShaderSource, fShaderSource;
};

#endif
