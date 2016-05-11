// VolumeVisualizer.h - a concrete 3D subclass of ModelView

#ifndef VOLUMEVISUALIZER_H
#define VOLUMEVISUALIZER_H

#include <string>

#include "ModelView3D.h"
#include "BasicShapeModelView.h"

class VolumeVisualizer : public ModelView3D
{
public:
	VolumeVisualizer(int nRowsIn, int nColsIn, int nSheetsIn,
		double rowScaleFactor, double colScaleFactor, double sheetScaleFactor,
		int* attrArrayIn);
	virtual ~VolumeVisualizer();

	void getMCBoundingBox(double* xyzLimits) const;
	void handleCommand(unsigned char key, double ldsX, double ldsY);
	void handleFunctionKey(int whichFunctionKey, double ldsX, double ldsY,
		int mods);
	void handleSpecialKey(Controller::SpecialKey key,
		double ldsX, double ldsY, int mods);
	void render();

private:
	BasicShapeModelView* voxelGridCubeMV;
	// Shader Storage Buffers (not placed in VAOs)
	GLuint vboVoxelGrid[1];

	int nRows, nCols, nSheets;
	float cellSizeX, cellSizeY, cellSizeZ;
	int *attrArray;
	double xyzMinMax[6];
	int rayFunction;
	int rayFunctionParameter;
	float bgr;
	float bgg;
	float bgb;
	float c1r;
	float c2r;
	float c3r;
	float c1g;
	float c2g;
	float c3g;
	float c1b;
	float c2b;
	float c3b;
	float c1a;
	float c2a;
	float c3a;
	float colorMap12;
	float colorMap23;
	float stepSize;
	float* bgcolor;
	int bgcount;
	float* color;
	int colorCount;

	// PPUs for voxel grid
	static GLint ppuLoc_nRows, ppuLoc_nCols, ppuLoc_nSheets;
	static GLint ppuLoc_cellSizeX, ppuLoc_cellSizeY, ppuLoc_cellSizeZ;
	// PPUs for rendering options
	static GLint ppuLoc_rayFunction, ppuLoc_rayFunctionParameter, ppuLoc_bgr, ppuLoc_bgg, ppuLoc_bgb,
	ppuLoc_colorMap12, ppuLoc_colorMap23,
	ppuLoc_c1a,	ppuLoc_c2a,	ppuLoc_c3a,
	ppuLoc_c1r,	ppuLoc_c2r,	ppuLoc_c3r,
	ppuLoc_c1g,	ppuLoc_c2g,	ppuLoc_c3g,
	ppuLoc_c1b,	ppuLoc_c2b,	ppuLoc_c3b, ppuLoc_stepSize;

	static void fetchGLSLVariableLocations();

	void sendBufferData();
};

#endif
