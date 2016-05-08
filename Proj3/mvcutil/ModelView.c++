// ModelView.c++ - an Abstract Base Class for a combined Model and View for OpenGL

#include <iostream>

#include "ModelView.h"
#include "Controller.h"

// init EYE(0,0,4), CENTER(0,0,0), UP(0,1,0)
ModelView::EyeCoordSystemSpec ModelView::curEC, ModelView::origEC;

double ModelView::mcRegionOfInterest[6] = { -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 };
ProjectionType ModelView::projType = PERSPECTIVE;
cryph::AffVector ModelView::obliqueProjectionDir(0.25, 0.5, 1.0);
double ModelView::ecZmin = -2.0;
double ModelView::ecZmax = -0.01; // for perspective, must be strictly < 0
double ModelView::zpp = -1.0; // for perspective, must be strictly < 0

bool ModelView::aspectRatioPreservationEnabled = true;
// We record eye coordinate deltas for use in defining projection
// matrices and converting LDS offsets to eye (and hence model) deltas.
double ModelView::ecDeltaX = 2.0, ModelView::ecDeltaY = 2.0, ModelView::ecDeltaZ = 2.0;

// dynamic zoom is built into the window-viewport mapping portion of the
// ec_lds matrix by expanding/contracting the window
double ModelView::dynamic_zoomScale = 1.0;

// Dynamic view panning: A common interface from the Controller is used to
// drive either a 2D pan in compute2DScaleTrans, or a 3D one in getMatrices.
// For the former, "mcPanVector2D" is computed in model coordinates in addToGlobalPan
// below. 3D view panning can be done in one of (at least) two ways:
// (1) A 3D pan vector is computed in model coordinates and then added to
//     the eye and center.
// (2) Accumulate pan translations into the dynamic_view matrix.
cryph::AffVector ModelView::mcPanVector2D(0,0,0);
// Matrices
cryph::Matrix4x4 ModelView::dynamic_view(1,0,0,0 , 0,1,0,0 , 0,0,1,0, 0,0,0,1);
cryph::Matrix4x4 ModelView::lookAtMatrix;
cryph::Matrix4x4 ModelView::mc_ec_full;
cryph::Matrix4x4 ModelView::ec_lds;

ModelView::ModelView() : polygonMode(GL_FILL)
{
}
ModelView::~ModelView()
{
}

void ModelView::addToGlobalPan(double dxInLDS, double dyInLDS, double dzInLDS)
{
	// In case this is a 2D application/ModelView instance:
    float sclTrans[4];
	compute2DScaleTrans(sclTrans);
	cryph::AffVector inc(dxInLDS/sclTrans[0], dyInLDS/sclTrans[2], 0.0);
	mcPanVector2D += inc;

	// Now in case this is a 3D application:

	// map the deltas from their (-1,+1) range to distances in eye coordinates:
	double dxInEC = 0.5 * dxInLDS * ecDeltaX;
	double dyInEC = 0.5 * dyInLDS * ecDeltaY;
	double dzInEC = 0.5 * dzInLDS * ecDeltaZ;

	// For 3D, we build and concatenate a translation matrix:
	cryph::Matrix4x4 trans = cryph::Matrix4x4::translation(
		cryph::AffVector(dxInEC, dyInEC, dzInEC));
	dynamic_view = trans * dynamic_view;
}

void ModelView::addToGlobalRotationDegrees(double rx, double ry, double rz)
{
	cryph::Matrix4x4 rxM = cryph::Matrix4x4::xRotationDegrees(rx);
	cryph::Matrix4x4 ryM = cryph::Matrix4x4::yRotationDegrees(ry);
	cryph::Matrix4x4 rzM = cryph::Matrix4x4::zRotationDegrees(rz);
	dynamic_view = rxM * ryM * rzM * dynamic_view;
}

void ModelView::addToGlobalZoom(double increment)
{
	if ((dynamic_zoomScale+increment) > 0.0)
		dynamic_zoomScale += increment;
}

void ModelView::compute2DScaleTrans(float* scaleTransF) // USED FOR 2D SCENES
{
    double xmin = mcRegionOfInterest[0]-mcPanVector2D[0];
	double xmax = mcRegionOfInterest[1]-mcPanVector2D[0];
    double ymin = mcRegionOfInterest[2]-mcPanVector2D[1];
	double ymax = mcRegionOfInterest[3]-mcPanVector2D[1];

	if (aspectRatioPreservationEnabled)
	{
    	// preserve aspect ratio. Make "region of interest" wider or taller to
    	// match the Controller's viewport aspect ratio.
    	double vAR = Controller::getCurrentController()->getViewportAspectRatio();
		matchAspectRatio(xmin, xmax, ymin, ymax, vAR);
	}
	
    double scaleTrans[4];
	double ldsD = ModelView::dynamic_zoomScale;
    linearMap(xmin, xmax, -ldsD, ldsD, scaleTrans[0], scaleTrans[1]);
    linearMap(ymin, ymax, -ldsD, ldsD, scaleTrans[2], scaleTrans[3]);
    for (int i=0 ; i<4 ; i++)
        scaleTransF[i] = static_cast<float>(scaleTrans[i]);
}

void ModelView::getMatrices(cryph::Matrix4x4& mc_ec_fullOut,
							cryph::Matrix4x4& ec_ldsOut)
{
	double preTransDist = curEC.distEyeCenter;
	cryph::Matrix4x4 preTrans = cryph::Matrix4x4::translation(
		cryph::AffVector(0.0, 0.0, preTransDist));
	cryph::Matrix4x4 postTrans = cryph::Matrix4x4::translation(
		cryph::AffVector(0.0, 0.0, -preTransDist));
	cryph::Matrix4x4 post_dynamic_pre = postTrans * dynamic_view * preTrans;

	lookAtMatrix = cryph::Matrix4x4::lookAt(curEC.eye, curEC.center, curEC.up);
	mc_ec_full = post_dynamic_pre * lookAtMatrix;

	setProjectionTransformation();

	// Return the two final matrices
	mc_ec_fullOut = mc_ec_full;
	ec_ldsOut = ec_lds;
}

void ModelView::linearMap(double fromMin, double fromMax, double toMin, double toMax,
	double& scale, double& trans)
{
	scale = (toMax - toMin) / (fromMax - fromMin);
	trans = toMin - scale*fromMin;
}

void ModelView::matchAspectRatio(double& xmin, double& xmax,
        double& ymin, double& ymax, double vAR)
{
	double wHeight = ymax - ymin;
	double wWidth = xmax - xmin;
	double wAR = wHeight / wWidth;
	if (wAR > vAR)
	{
		// make window wider
		wWidth = wHeight / vAR;
		double xmid = 0.5 * (xmin + xmax);
		xmin = xmid - 0.5*wWidth;
		xmax = xmid + 0.5*wWidth;
	}
	else
	{
		// make window taller
		wHeight = wWidth * vAR;
		double ymid = 0.5 * (ymin + ymax);
		ymin = ymid - 0.5*wHeight;
		ymax = ymid + 0.5*wHeight;
	}
}

GLint ModelView::ppUniformLocation(GLuint glslProgram, const std::string& name)
{
	GLint loc = glGetUniformLocation(glslProgram, name.c_str());
	if (loc < 0)
		std::cerr << "Could not locate per-primitive uniform: '" << name << "'\n";
	return loc;
}

GLint ModelView::pvAttribLocation(GLuint glslProgram, const std::string& name)
{
	GLint loc = glGetAttribLocation(glslProgram, name.c_str());
	if (loc < 0)
		std::cerr << "Could not locate per-vertex attribute: '" << name << "'\n";
	return loc;
}

void ModelView::resetGlobalDynamic()
{
	mcPanVector2D = cryph::AffVector(0,0,0);
	dynamic_view = cryph::Matrix4x4::IdentityMatrix;
	curEC = origEC;
}

void ModelView::resetGlobalZoom()
{
	dynamic_zoomScale = 1.0;
}	

void ModelView::scaleGlobalZoom(double multiplier)
{
	if (multiplier > 0.0)
		dynamic_zoomScale *= multiplier;
}

void ModelView::set_ecDeltas()
{
	// Get MC deltas from region of interest:
	double dx = mcRegionOfInterest[1] - mcRegionOfInterest[0];
	double dy = mcRegionOfInterest[3] - mcRegionOfInterest[2];
	double dz = mcRegionOfInterest[5] - mcRegionOfInterest[4];

	// Convert MC deltas to EC deltas (5 steps):
	// 1. Form representative MC vectors spanning the region of interest
	cryph::AffVector vecs[7] = {
		cryph::AffVector(dx, 0, 0), cryph::AffVector(0, dy, 0),
		cryph::AffVector(0, 0, dz), cryph::AffVector(dx, dy, 0),
		cryph::AffVector(dx, 0, dz), cryph::AffVector(0, dy, dz),
		cryph::AffVector(dx, dy, dz)
	};

	// 2. Get the three unit EC system vectors as measured in MC
	double m[16];
	lookAtMatrix.extractRowMajor(m);
	cryph::AffVector uvw[3] = {
		cryph::AffVector(m[0], m[1], m[2]),
		cryph::AffVector(m[4], m[5], m[6]),
		cryph::AffVector(m[8], m[9], m[10])
	};

	// 3. In each EC direction, the EC delta will be the maximum absolute
	//    value of the length of the MC vector in the EC direction. (The
	//    x and y EC directions are really the main ones of interest, We
	//    compute all 3, even though the z delta will later be adjusted
	//    as noted below.)
	double ecd[3];
	for (int i=0 ; i<3 ; i++)
	{
		ecd[i] = 0.0;
		for (int j=0 ; j<7 ; j++)
		{
			double d = fabs(uvw[i].dot(vecs[j]));
			if (d > ecd[i])
				ecd[i] = d;
		}
	}

	// 4. Use current interactive global zoom while setting basic EC deltas:
	ecDeltaX = ecd[0] / dynamic_zoomScale;
	ecDeltaY = ecd[1] / dynamic_zoomScale;
	// ecDeltaZ will be reset in setProjectionTransformation based on the
	// explicit ecZmin and ecZmax as established via ModelView::setECZminZmax.
	// Nevertheless, for now we set:
	ecDeltaZ = ecd[2];

	// 5. Adjust the x and y ecDeltas, if desired, for aspect ratio preservation:
	if (aspectRatioPreservationEnabled)
	{
		double wAR = ecDeltaY / ecDeltaX; // height/width
		double vAR = Controller::getCurrentController()->getViewportAspectRatio();
		if (vAR > wAR)
			ecDeltaY = vAR * ecDeltaX;
		else
			ecDeltaX = ecDeltaY / vAR;
	}
}

void ModelView::setECZminZmax(double zMinIn, double zMaxIn)
{
	ecZmin = zMinIn;
	ecZmax = zMaxIn;
}

void ModelView::setEyeCenterUp(cryph::AffPoint E, cryph::AffPoint C, cryph::AffVector up)
{
	cryph::AffVector v, w;
	if (cryph::Matrix4x4::getECvw(E, C, up, v, w))
	{
		origEC.eye = E;
		origEC.center = C;
		origEC.up = up;
		origEC.distEyeCenter = origEC.eye.distanceTo(origEC.center);
		resetGlobalDynamic();
		resetGlobalZoom();
	}
}

void ModelView::setMCRegionOfInterest(double xyz[])
{
	for (int i=0 ; i<6 ; i++)
		mcRegionOfInterest[i] = xyz[i];
}

void ModelView::setObliqueProjectionDirection(const cryph::AffVector& dir)
{
	ModelView::obliqueProjectionDir = dir;
}

void ModelView::setProjection(ProjectionType pType)
{
	projType = pType;
}

void ModelView::setProjectionPlaneZ(double zppIn)
{
	zpp = zppIn;
}

// Following is very roughly the 3D counterpart of the 2D "compute2DScaleTrans"
// IT IMPLICITLY ASSUMES THAT THE LINE OF SIGHT PASSES THROUGH THE CENTER
// OF THE SCENE.
void ModelView::setProjectionTransformation()
{
	// adjust window deltas to match the aspect ratio of the viewport
	set_ecDeltas();

	// Create the parameters for the desired type of projection

	double halfDx = 0.5 * ecDeltaX;
	double halfDy = 0.5 * ecDeltaY;
	if (projType == PERSPECTIVE)
	{
		ec_lds = cryph::Matrix4x4::perspective(zpp, -halfDx, halfDx, -halfDy, halfDy, ecZmin, ecZmax);
	}
	else if (projType == ORTHOGONAL)
	{
		ec_lds = cryph::Matrix4x4::orthogonal(-halfDx, halfDx, -halfDy, halfDy, ecZmin, ecZmax);
	}
	else // (projType == OBLIQUE)
	{
		ec_lds = cryph::Matrix4x4::oblique(zpp, -halfDx, halfDx, -halfDy, halfDy, ecZmin, ecZmax,
			ModelView::obliqueProjectionDir);
	}
	ecDeltaZ = ecZmax - ecZmin;
}
