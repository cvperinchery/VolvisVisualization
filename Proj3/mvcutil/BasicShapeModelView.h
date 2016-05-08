// BasicShapeModelView.h

#ifndef BASICSHAPEMODELVIEW_H
#define BASICSHAPEMODELVIEW_H

#include <string>

#include "ModelView3D.h"
#include "BasicShape.h"

class BasicShapeModelView : public ModelView3D
{
public:
	BasicShapeModelView(BasicShape* shapeIn);
	virtual ~BasicShapeModelView();

	void getMCBoundingBox(double* xyzLimits) const;
	void render();

	static void setUseEBOs(bool b) { useEBOs = b; }

private:
	void defineGeometry();
	void drawShape();

	BasicShape* theShape;
	GLuint vao, vertexBuffer, normalBuffer, *ebo;
	int nEBOs;
	double xyzMinMax[6];

	// Use element buffer instead of local index arrays?
	static bool useEBOs;
};

#endif
