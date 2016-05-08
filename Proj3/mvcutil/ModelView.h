// ModelView.h - an Abstract Base Class for a combined Model and View for OpenGL

#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <string>

#ifdef __APPLE_CC__
#include "GLFW/glfw3.h"
#else
#include <GL/gl.h>
#endif

#include "AffPoint.h"
#include "AffVector.h"
#include "Controller.h"
#include "Matrix4x4.h"
#include "ProjectionType.h"

class ModelView
{
public:
	ModelView();
	virtual ~ModelView();

	virtual void getMCBoundingBox(double* xyzLimits) const = 0;
	virtual void handleCommand(unsigned char key, double ldsX, double ldsY) { }
	virtual void handleFunctionKey(int whichFunctionKey,
		double ldsX, double ldsY, int mods) { }
	virtual void handleSpecialKey(Controller::SpecialKey key,
		double ldsX, double ldsY, int mods) { }
	virtual void render() = 0;

	// The portion of the viewing API common to 2D and 3D:
	static void setMCRegionOfInterest(double xyz[]);

	// 3D Viewing API for specifying initial view:
	static void setAspectRatioPreservationEnabled(bool b) { aspectRatioPreservationEnabled = b; }
	static void setECZminZmax(double zMinIn, double zMaxIn);
	static void setEyeCenterUp(cryph::AffPoint E, cryph::AffPoint C, cryph::AffVector up);
	static void setObliqueProjectionDirection(const cryph::AffVector& dir);
	static void setProjection(ProjectionType pType);
	static void setProjectionPlaneZ(double zppIn);

	// Viewing interfaces for dynamic view manipulation
	static void addToGlobalPan(double dxInLDS, double dyInLDS, double dzInLDS);
	static void addToGlobalRotationDegrees(double rx, double ry, double rz);
	static void addToGlobalZoom(double increment);  // dynamic_zoomScale += increment
	static void resetGlobalDynamic(); // rotation and pan
	static void resetGlobalZoom();
	static void scaleGlobalZoom(double multiplier); // dynamic_zoomScale *= multiplier

protected:
	GLenum polygonMode;

	// NOTE: Subclasses will generally use EITHER compute2DScaleTrans OR getMatrices:
	// Method "compute2DScaleTrans" is intended for 2D scenes:
	static void compute2DScaleTrans(float* sclTrans);
	// Method "getMatrices" is intended for 3D scenes:
	static void getMatrices(cryph::Matrix4x4& mc_ec_fullOut,
							cryph::Matrix4x4& ec_ldsOut);

	static void linearMap(double fromMin, double fromMax, double toMin, double toMax,
		double& scale, double& trans);
	static void matchAspectRatio(double& xmin, double& xmax,
		double& ymin, double& ymax, double vAR);
	// "pp": "per-primitive"; "pv": "per-vertex"
	static GLint ppUniformLocation(GLuint glslProgram, const std::string& name);
	static GLint pvAttribLocation(GLuint glslProgram, const std::string& name);

	// Current eye coordinate view volume deltas
	static double ecDeltaX, ecDeltaY, ecDeltaZ;
	static bool aspectRatioPreservationEnabled;
private:
	static double dynamic_zoomScale; // for global zoom
	// View orientation
	struct EyeCoordSystemSpec
	{
		cryph::AffPoint eye, center;
		cryph::AffVector up;
		double distEyeCenter;

		EyeCoordSystemSpec() : eye(0,0,4), center(0,0,0), up(0,1,0),
			distEyeCenter(4)
		{}
	};
	static EyeCoordSystemSpec curEC, origEC;
	// Current MC region of interest
	static double mcRegionOfInterest[6];

	static cryph::AffVector mcPanVector2D;
	// Projections
	static ProjectionType projType;
	static cryph::AffVector obliqueProjectionDir;
	static double zpp;
	static double ecZmin, ecZmax;
	// matrix variables and utilities
	static cryph::Matrix4x4 dynamic_view; // mouse-based dynamic 3D rotations and pan
	static cryph::Matrix4x4 lookAtMatrix;
	static cryph::Matrix4x4 mc_ec_full; // = rotation * lookAtMatrix
	static cryph::Matrix4x4 ec_lds; // wv * proj

	static void set_ecDeltas();
	static void setProjectionTransformation();
	// end matrix utilities
};

#endif
