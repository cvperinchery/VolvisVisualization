// BasicShapeModelView.c++

#include "BasicShapeModelView.h"

bool BasicShapeModelView::useEBOs = true;

BasicShapeModelView::BasicShapeModelView(BasicShape* shapeIn) :
		theShape(shapeIn), vertexBuffer(0), normalBuffer(0),
		ebo(NULL), nEBOs(0)
{
	if (theShape == NULL)
		return;
	defineGeometry();
	theShape->getMCBoundingBox(xyzMinMax);
}

BasicShapeModelView::~BasicShapeModelView()
{
	if (vertexBuffer > 0)
		glDeleteBuffers(1, &vertexBuffer);
	if (normalBuffer > 0)
		glDeleteBuffers(1, &normalBuffer);
	if (ebo != NULL)
	{
		glDeleteBuffers(nEBOs, ebo);
		delete [] ebo;
	}
	glDeleteVertexArrays(1, &vao);
	if (theShape != NULL)
		delete theShape;
}

void BasicShapeModelView::defineGeometry()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// There is always a vertex buffer:
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	const float* pts = theShape->getPointCoords();
	int nPoints = theShape->getNumPoints();
	glBufferData(GL_ARRAY_BUFFER, nPoints*3*sizeof(float), pts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(pvaLoc_mcPosition);
	glVertexAttribPointer(pvaLoc_mcPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create the normal buffer, if appropriate
	const float* normals = theShape->getNormals();
	if (normals == NULL)
		glDisableVertexAttribArray(pvaLoc_mcNormal);
	else
	{
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, nPoints*3*sizeof(float), normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(pvaLoc_mcNormal);
		glVertexAttribPointer(pvaLoc_mcNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if (useEBOs)
	{
		nEBOs = theShape->getNumIndexLists();
		ebo = new GLuint[nEBOs];
		glGenBuffers(nEBOs, ebo);
		for (int i=0 ; i<nEBOs ; i++)
		{
			GLenum mode;
			GLsizei nInList;
			GLenum type;
			bool canUseTexCoordArray, canUseNormalArray;
			cryph::AffVector fixedN;
			const void* indices = theShape->getIndexList(i, mode, nInList, type,
									canUseTexCoordArray, canUseNormalArray, fixedN);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, nInList*sizeof(unsigned int), indices, GL_STATIC_DRAW);
		}
	}
}

void BasicShapeModelView::drawShape()
{
	glBindVertexArray(vao);

	// Rendering BasicShape instances may involve 1 or more glDrawArrays calls:
	int nDrawArraysCalls = theShape->getNumDrawArraysCalls();
	float normal[3]; // Used in vComponents calls
	for (int i=0 ; i<nDrawArraysCalls ; i++)
	{
		// get data for i-th glDrawArrays call:
		GLenum mode;
		int offset;
		bool canUseTexCoordArray, canUseNormalArray;
		cryph::AffVector fixedN;
		int nPoints = theShape->getDrawArraysData(i, mode, offset,
			canUseTexCoordArray, canUseNormalArray, fixedN);
		// process the i-th call
		if (nPoints > 0)
		{
			if (canUseNormalArray)
				glEnableVertexAttribArray(pvaLoc_mcNormal);
			else
			{
				glDisableVertexAttribArray(pvaLoc_mcNormal);
				glVertexAttrib3fv(pvaLoc_mcNormal, fixedN.vComponents(normal));
			}
			glDrawArrays(mode, offset, nPoints);
		}
	}

	// Rendering BasicShape instances may also involve 1 or more glDrawElements calls.
	// (For example, caps, if present, are drawn with index lists. So are 3 of the faces
	// of blocks.)
	int nIndexLists = theShape->getNumIndexLists();
	for (int i=0 ; i<nIndexLists ; i++)
	{
		GLenum mode;
		GLsizei nInList;
		GLenum type;
		bool canUseTexCoordArray, canUseNormalArray;
		cryph::AffVector fixedN;
		const void* indices = theShape->getIndexList(i, mode, nInList, type,
									canUseTexCoordArray, canUseNormalArray, fixedN);
		if (canUseNormalArray)
			glEnableVertexAttribArray(pvaLoc_mcNormal);
		else
		{
			glDisableVertexAttribArray(pvaLoc_mcNormal);
			glVertexAttrib3fv(pvaLoc_mcNormal, fixedN.vComponents(normal));
		}
		if (ebo != NULL)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
			glDrawElements(mode, nInList, type, NULL);
		}
		else
			glDrawElements(mode, nInList, type, indices);
	}
}

void BasicShapeModelView::getMCBoundingBox(double* xyzLimits) const
{
	// Just pass back what the BasicShape instance gave to us
	for (int i=0 ; i<6 ; i++)
		xyzLimits[i] = xyzMinMax[i];
}

void BasicShapeModelView::render()
{
	if (theShape == NULL)
		return;

	// save the current and set the new shader program
	int savedPgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &savedPgm);
	glUseProgram(shaderProgram);

	glEnable(GL_DEPTH_TEST);

	// establish global matrices
	cryph::Matrix4x4 mc_ec, ec_lds;
	getMatrices(mc_ec, ec_lds);
	float m[16];
	glUniformMatrix4fv(ppuLoc_mc_ec, 1, false, mc_ec.extractColMajor(m));
	glUniformMatrix4fv(ppuLoc_ec_lds, 1, false, ec_lds.extractColMajor(m));

	drawShape();

	// restore the saved program
	glUseProgram(savedPgm);
}
