// VolumeVisualizer.c++ - a concrete 3D subclass of ModelView
#include <iostream>
#include <fstream>

#include "VolumeVisualizer.h"

// PVAs and PPUs for both scalar field and vector field:
GLint VolumeVisualizer::ppuLoc_nRows = -2;
GLint VolumeVisualizer::ppuLoc_nCols = -2;
GLint VolumeVisualizer::ppuLoc_nSheets = -2;
GLint VolumeVisualizer::ppuLoc_cellSizeX = -2;
GLint VolumeVisualizer::ppuLoc_cellSizeY = -2;
GLint VolumeVisualizer::ppuLoc_cellSizeZ = -2;
GLint VolumeVisualizer::ppuLoc_rayFunction = -2;
GLint VolumeVisualizer::ppuLoc_rayFunctionParameter = -2;
GLint VolumeVisualizer::ppuLoc_stepSize = -2;

VolumeVisualizer::VolumeVisualizer(int nRowsIn, int nColsIn, int nSheetsIn,
	double rowScaleFactor, double colScaleFactor, double sheetScaleFactor,
	int* attrArrayIn) :
		nRows(nRowsIn), nCols(nColsIn), nSheets(nSheetsIn),
		cellSizeX(colScaleFactor), cellSizeY(rowScaleFactor),
		cellSizeZ(sheetScaleFactor), attrArray(attrArrayIn),
		rayFunction(0), rayFunctionParameter(100), stepSize(0.9)
{
	if (numInstances == 1)
		// This is the first one created; lookup extra variables
		fetchGLSLVariableLocations();

	// (nCols, nRows, nSheets) index the vertices. Hence the block dimensions
	// are 1.0 smaller than those:
	BasicShape* voxelGridCube = BasicShape::makeBlock(cryph::AffPoint::origin,
		cryph::AffVector::xu, colScaleFactor * (nCols-1.0),
		cryph::AffVector::yu, rowScaleFactor * (nRows-1.0),
		cryph::AffVector::zu, sheetScaleFactor * (nSheets-1.0));
	voxelGridCubeMV = new BasicShapeModelView(voxelGridCube);

	sendBufferData();
}

VolumeVisualizer::~VolumeVisualizer()
{
}

void VolumeVisualizer::fetchGLSLVariableLocations()
{
	if (shaderProgram > 0)
	{
		ppuLoc_nRows = ppUniformLocation(shaderProgram, "nRows");
		ppuLoc_nCols = ppUniformLocation(shaderProgram, "nCols");
		ppuLoc_nSheets = ppUniformLocation(shaderProgram, "nSheets");
		ppuLoc_cellSizeX = ppUniformLocation(shaderProgram, "cellSizeX");
		ppuLoc_cellSizeY = ppUniformLocation(shaderProgram, "cellSizeY");
		ppuLoc_cellSizeZ = ppUniformLocation(shaderProgram, "cellSizeZ");
		ppuLoc_rayFunction = ppUniformLocation(shaderProgram, "rayFunction");
		ppuLoc_rayFunctionParameter = ppUniformLocation(shaderProgram, "rayFunctionParameter");
		ppuLoc_stepSize = ppUniformLocation(shaderProgram, "stepSize");
	}
}

void VolumeVisualizer::getMCBoundingBox(double* xyzLimits) const
{
	voxelGridCubeMV->getMCBoundingBox(xyzLimits);
}

void VolumeVisualizer::handleCommand(unsigned char key, double ldsX, double ldsY)
{
	bool handled = false;
	if (key == 'c'){
		if(++rayFunction >= 6){
			rayFunction = 0;
		}
		handled = true;
	}
	else if (key == '+' || key == '=') //increase rayFunctionParameter
	{
		if(++rayFunctionParameter >= 255){
			rayFunctionParameter = 0;
		}
		handled = true;
	}
	else if(key == '-')//decrease rayFunctionParameter
	{
		if(--rayFunctionParameter <= 0){
			rayFunctionParameter = 255;
		}
		handled = true;
	}
	// Handle events you want, setting "handled" to true if you handle one...

	if (!handled)
		ModelView3D::handleCommand(key, ldsX, ldsY);
}

void VolumeVisualizer::handleFunctionKey(int whichFunctionKey, double ldsX, double ldsY,
        int mods)
{
	bool handled = false;

	// Handle events you want, setting "handled" to true if you handle one...

	if (!handled)
		ModelView3D::handleFunctionKey(whichFunctionKey, ldsX, ldsY, mods);
}

void VolumeVisualizer::handleSpecialKey(Controller::SpecialKey key,
	double ldsX, double ldsY, int mods)
{
	bool handled = false;

	// Handle events you want, setting "handled" to true if you handle one...

	if (!handled)
		ModelView3D::handleSpecialKey(key, ldsX, ldsY, mods);
}

void VolumeVisualizer::render()
{
	GLint pgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);

	if (shaderProgram <= 0)
		return;

	glUseProgram(shaderProgram);

	glUniform1i(ppuLoc_nRows, nRows);
	glUniform1i(ppuLoc_nCols, nCols);
	glUniform1i(ppuLoc_nSheets, nSheets);
	glUniform1f(ppuLoc_cellSizeX, cellSizeX);
	glUniform1f(ppuLoc_cellSizeY, cellSizeY);
	glUniform1f(ppuLoc_cellSizeZ, cellSizeZ);
	glUniform1i(ppuLoc_rayFunction, rayFunction);
	glUniform1f(ppuLoc_rayFunctionParameter, rayFunctionParameter);
	glUniform1f(ppuLoc_stepSize, stepSize);

	voxelGridCubeMV->render();

	glUseProgram(pgm);
}

void VolumeVisualizer::sendBufferData()
{
	int numVoxelVertices = nCols * nRows * nSheets;
	int min = attrArray[0];
	int max = attrArray[0];
	for (int i=1 ; i<numVoxelVertices ; i++)
		if (attrArray[i] < min)
			min = attrArray[i];
		else if (attrArray[i] > max)
			max = attrArray[i];
	std::cout << min << " <= voxelValue <= " << max << '\n';
	int numBytes = numVoxelVertices * sizeof(int);

	// The scalar field:
	glGenBuffers(1, vboVoxelGrid);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vboVoxelGrid[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, numBytes, attrArray, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vboVoxelGrid[0]);
}
