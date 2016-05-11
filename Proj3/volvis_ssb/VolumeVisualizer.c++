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
GLint VolumeVisualizer::ppuLoc_bgr = -2;
GLint VolumeVisualizer::ppuLoc_bgg = -2;
GLint VolumeVisualizer::ppuLoc_bgb = -2;
GLint VolumeVisualizer::ppuLoc_colorMap12 = -2;
GLint VolumeVisualizer::ppuLoc_colorMap23 = -2;
GLint VolumeVisualizer::ppuLoc_c1a = -2;
GLint VolumeVisualizer::ppuLoc_c2a = -2;
GLint VolumeVisualizer::ppuLoc_c3a = -2;
GLint VolumeVisualizer::ppuLoc_c1r = -2;
GLint VolumeVisualizer::ppuLoc_c2r = -2;
GLint VolumeVisualizer::ppuLoc_c3r = -2;
GLint VolumeVisualizer::ppuLoc_c1g = -2;
GLint VolumeVisualizer::ppuLoc_c2g = -2;
GLint VolumeVisualizer::ppuLoc_c3g = -2;
GLint VolumeVisualizer::ppuLoc_c1b = -2;
GLint VolumeVisualizer::ppuLoc_c2b = -2;
GLint VolumeVisualizer::ppuLoc_c3b = -2;

GLint VolumeVisualizer::ppuLoc_stepSize = -2;

VolumeVisualizer::VolumeVisualizer(int nRowsIn, int nColsIn, int nSheetsIn,
	double rowScaleFactor, double colScaleFactor, double sheetScaleFactor,
	int* attrArrayIn) :
		nRows(nRowsIn), nCols(nColsIn), nSheets(nSheetsIn),
		cellSizeX(colScaleFactor), cellSizeY(rowScaleFactor),
		cellSizeZ(sheetScaleFactor), attrArray(attrArrayIn),
		rayFunction(0), rayFunctionParameter(100),
		bgr(1.0), bgg(1.0), bgb(1.0),
		colorMap12(85), colorMap23(170), c1a(1.0), c2a(1.0), c3a(1.0),
		c1r(1.0), c2r(1.0), c3r(1.0), c1g(1.0), c2g(1.0), c3g(1.0), c1b(1.0), c2b(1.0), c3b(1.0),
		stepSize(0.9)
{
	if (numInstances == 1)
		// This is the first one created; lookup extra variables
		fetchGLSLVariableLocations();

	//Hard code in background colors
	bgcolor = new float[15];
	bgcolor[0] = .98;
	bgcolor[1] = .70;
	bgcolor[2] = .80;
	bgcolor[3] = .87;
	bgcolor[4] = .99;
	bgcolor[5] = .70;
	bgcolor[6] = .80;
	bgcolor[7] = .92;
	bgcolor[8] = .79;
	bgcolor[9] = .85;
	bgcolor[10] = .68;
	bgcolor[11] = .89;
	bgcolor[12] = .77;
	bgcolor[13] = .89;
	bgcolor[14] = .65;
	bgcount = 0;

	color = new float[27];
	color[0] = .1;
	color[1] = .62;
	color[2] = .47;
	color[3] = .65;
	color[4] = .80;
	color[5] = .89;
	color[6] = .98;
	color[7] = .55;
	color[8] = .38;
	color[9] = .85;
	color[10] = .37;
	color[11] = 0.0;
	color[12] = .12;
	color[13] = .47;
	color[14] = .7;
	color[15] = .4;
	color[16] = .36;
	color[17] = .64;
	color[18] = .43;
	color[19] = .44;
	color[20] = .7;
	color[21] = .69;
	color[22] = .87;
	color[23] = .54;
	color[24] = .55;
	color[25] = .62;
	color[26] = .79;
	colorCount = 0;

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
	delete bgcolor;
	delete color;
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
		ppuLoc_bgr = ppUniformLocation(shaderProgram, "bgr");
		ppuLoc_bgg = ppUniformLocation(shaderProgram, "bgg");
		ppuLoc_bgb = ppUniformLocation(shaderProgram, "bgb");
		ppuLoc_colorMap12 = ppUniformLocation(shaderProgram, "colorMap12");
		ppuLoc_colorMap23 = ppUniformLocation(shaderProgram, "colorMap23");
		ppuLoc_c1a = ppUniformLocation(shaderProgram, "c1a");
		ppuLoc_c2a = ppUniformLocation(shaderProgram, "c2a");
		ppuLoc_c3a = ppUniformLocation(shaderProgram, "c3a");
		ppuLoc_c1r = ppUniformLocation(shaderProgram, "c1r");
		ppuLoc_c2r = ppUniformLocation(shaderProgram, "c2r");
		ppuLoc_c3r = ppUniformLocation(shaderProgram, "c3r");
		ppuLoc_c1g = ppUniformLocation(shaderProgram, "c1g");
		ppuLoc_c2g = ppUniformLocation(shaderProgram, "c2g");
		ppuLoc_c3g = ppUniformLocation(shaderProgram, "c3g");
		ppuLoc_c1b = ppUniformLocation(shaderProgram, "c1b");
		ppuLoc_c2b = ppUniformLocation(shaderProgram, "c2b");
		ppuLoc_c3b = ppUniformLocation(shaderProgram, "c3b");
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
	else if(key == '0'){ // regular box
		rayFunction = 0;
	}
	else if(key == '1'){ // binary
		rayFunction = 1;
	}
	else if(key == '2'){ // max
		rayFunction = 2;
	}
	else if(key == '3'){ // average
		rayFunction = 3;
	}
	else if(key == '4'){ // sum
		rayFunction = 4;
	}
	else if(key == '5'){ // accumulation rgb
		rayFunction = 5;
	}
	else if(key == '6'){ // accumulation rgb + background color
		rayFunction = 6;
	}
	else if(key == '7'){
		rayFunction = 7;
	}
	else if(key == '8'){
		rayFunction = 8;
	}
	else if(key == '9'){ // color
		rayFunction = 9;
	}
	else if (key == 'p'){
		std::cout << rayFunctionParameter << std::endl;
	}
	else if (key == 'n'){ // decreases  the separation point between colors 1 and 2
		if(--colorMap12 <= 0){
			colorMap12 = 0;
		}
		handled = true;
	}
	else if (key == 'm'){// increases  the separation point between colors 1 and 2
		if(++colorMap12 > colorMap23){
			colorMap12 = colorMap23;
		}
		handled = true;
	}
	else if (key == ','){// decreases the separation point between colors 2 and 3
		if(--colorMap23 < colorMap12){
			colorMap23 = colorMap12;
		}
		handled = true;
	}
	else if (key == '.'){// increases the separation point between colors 2 and 3
		if(++	colorMap23 > 255){
			colorMap23 = 255;
		}
		handled = true;
	}
	else if (key == 'h'){ // cycles through transfer function colors
		if(++colorCount > 2){
			colorCount = 0;
		}
		c1r = color[colorCount*3];
		c1g = color[(colorCount*3) +1];
		c1b = color[(colorCount*3) +2];
		c2r = color[(colorCount*3) +9];
		c2g = color[(colorCount*3) +10];
		c2b = color[(colorCount*3) +11];
		c3r = color[(colorCount*3) +18];
		c3g = color[(colorCount*3) +19];
		c3b = color[(colorCount*3) +20];
	}
	else if (key == 'j'){ // increases opaqueness of color 1
		c1a = c1a +.02;
		if(c1a > .10){
			c1a = 0.0;
		}
	}
	else if (key == 'k'){ // increases opaqueness of color 2
		c2a = c2a +.02;
		if(c2a > .20){
			c2a = 0.0;
		}
	}
	else if (key == 'l'){ // increases opaqueness of color 3
		c3a = c3a +.02;
		if(c3a > .20){
			c3a = 0.0;
		}
	}

	else if(key == 'b'){//change background color
		if(++bgcount > 4){
			bgcount = 0;
		}
		bgr = bgcolor[bgcount];
		//std::cout << bgr << std::endl;
		bgg = bgcolor[bgcount+5];
		bgb = bgcolor[bgcount+10];
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
	glUniform1f(ppuLoc_bgr, bgr);
	glUniform1f(ppuLoc_bgg, bgg);
	glUniform1f(ppuLoc_bgb, bgb);
	glUniform1f(ppuLoc_colorMap12, colorMap12);
	glUniform1f(ppuLoc_colorMap23, colorMap23);
	glUniform1f(ppuLoc_c1a, c1a);
	glUniform1f(ppuLoc_c2a, c2a);
	glUniform1f(ppuLoc_c3a, c3a);
	glUniform1f(ppuLoc_c1r, c1r);
	glUniform1f(ppuLoc_c2r, c2r);
	glUniform1f(ppuLoc_c3r, c3r);
	glUniform1f(ppuLoc_c1g, c1g);
	glUniform1f(ppuLoc_c2g, c2g);
	glUniform1f(ppuLoc_c3g, c3g);
	glUniform1f(ppuLoc_c1b, c1b);
	glUniform1f(ppuLoc_c2b, c2b);
	glUniform1f(ppuLoc_c3b, c3b);
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
