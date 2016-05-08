// main.c++

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#include "GLFWController.h"
#include "VolumeVisualizer.h"
#include "Endian.h"

void initializeViewingInformation(Controller& c)
{
	// determine the center of the scene:
	double xyz[6];
	c.getOverallMCBoundingBox(xyz);
	ModelView::setMCRegionOfInterest(xyz);
	double xmid = 0.5 * (xyz[0] + xyz[1]);
	double ymid = 0.5 * (xyz[2] + xyz[3]);
	double zmid = 0.5 * (xyz[4] + xyz[5]);

	// a heuristic: arrange the eye and center points so that the distance
	// between them is (2*max scene dimension)
	double maxDelta = xyz[1] - xyz[0];
	double delta = xyz[3] - xyz[2];
	if (delta > maxDelta)
		maxDelta = delta;
	delta = xyz[5] - xyz[4];
	if (delta > maxDelta)
		maxDelta = delta;
	double distEyeCenter = 2.0 * maxDelta;

	cryph::AffPoint center(xmid, ymid, zmid);
	cryph::AffPoint eye(xmid, ymid, zmid + distEyeCenter);
	cryph::AffVector up = cryph::AffVector::yu;

	ModelView::setEyeCenterUp(eye, center, up);

	// Place the projection plane roughly at the front of scene
	// and set eye coordinate zmin/zmax planes relative to it.
	double zpp = -(distEyeCenter - 0.5*maxDelta);
	ModelView::setProjectionPlaneZ(zpp);
	ModelView::setECZminZmax(zpp - maxDelta, zpp+0.5*maxDelta);

	ModelView::setProjection(ORTHOGONAL);
}

bool getData(const char* configFileName, int& nRows, int& nCols, int& nSheets,
	double& rowScaleFactor, double& colScaleFactor, double& sheetScaleFactor,
	int*& voxels)
{
	std::ifstream config(configFileName);
	if (!config.good())
	{
		std::cerr << "Could not open " << configFileName << " for reading.\n";
		return false;
	}
	config >> nRows >> nCols >> nSheets;
	config >> rowScaleFactor >> colScaleFactor >> sheetScaleFactor;
	std::string voxelFileName;
	config >> voxelFileName;

	std::ifstream voxelFile(voxelFileName.c_str());
	if (!voxelFile.good())
	{
		std::cerr << "Could not open " << voxelFileName << " for reading.\n";
		return false;
	}
	int numVerticesInFile = nRows * nCols * nSheets;
	unsigned char* voxels8Bit = new unsigned char[numVerticesInFile];
	voxelFile.read(reinterpret_cast<char*>(voxels8Bit), numVerticesInFile);
	voxels = new int[numVerticesInFile];
	for (int i=0 ; i<numVerticesInFile ; i++)
		voxels[i] = voxels8Bit[i];
	delete [] voxels8Bit;
	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0]<< " configFile\n";
		return 0;
	}

	int nRows, nCols, nSheets;
	double rowScaleFactor, colScaleFactor, sheetScaleFactor;
	int* voxels;
	if (getData(argv[1], nRows, nCols, nSheets,
		rowScaleFactor, colScaleFactor, sheetScaleFactor, voxels))
	{
		GLFWController c("Voxel Grid", MVC_USE_DEPTH_BIT);
		c.reportVersions(std::cout);

		ModelView3D::setShaderSources("shaders/volumeVisualizer.vsh",
			"shaders/volumeVisualizer.fsh");
		VolumeVisualizer* vv = new VolumeVisualizer(
			nRows, nCols, nSheets,
			rowScaleFactor, colScaleFactor, sheetScaleFactor, voxels);
		c.addModel(vv);

		initializeViewingInformation(c);

		glClearColor(1.0, 1.0, 1.0, 1.0);

		c.run();
	}

	return 0;
}
