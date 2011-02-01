// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/GeometryBuilder.h"

namespace livescene {

Geometry::~Geometry()
{
	freeData();
} // Geometry::Geometry

void Geometry::freeData(void)
{
	delete [] _vertices; _vertices = 0;
	delete [] _indices; _indices = 0;
	delete [] _normals; _normals = 0;
	delete [] _texcoord; _texcoord = 0;
} // Geometry::freeData

void Geometry::allocData(unsigned int numVert, unsigned int numId)
{
	freeData();
	_vertices = new short[numVert];
	_indices  = new unsigned int[numId];
} // Geometry::allocData


bool Geometry::buildPointCloud(const livescene::Image &imageZ, livescene::Image *imageRGB)
{
	_width = imageZ.getWidth();
	_height = imageZ.getHeight();

	// we have to allocate for worst-case, all vertices/indices used
	// but after the null processing loop below we'll reset these to
	// indicate the number actually used.
	// the resource tracking knows how many really need to be freed, so this is ok
	_numVertices = imageZ.getSamples();
	_numIndices  = imageZ.getSamples();
	allocData(_numVertices * 3, _numIndices);

	int width(imageZ.getWidth()), height(imageZ.getHeight());
	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	unsigned int vertSub(0), indexSub(0), loopSub(0);
	int nullValue = imageZ.getNull();
	for(int line = 0; line < height; line++)
	{
		for(int column = 0; column < width; column++)
		{
			short originalDepth = depthBuffer[loopSub];
			if(originalDepth != nullValue && originalDepth != 0)
			{
				_indices[indexSub] = vertSub;
				_vertices[vertSub++] = column;
				_vertices[vertSub++] = line;
				_vertices[vertSub++] = originalDepth;
				indexSub++;
			} // if
			loopSub++;
		} // for
	_numVertices = indexSub;
	_numIndices  = indexSub;
} // for lines

return(true);
} // buildPointCloudGeometry


bool Geometry::buildFaces(const livescene::Image &imageZ, livescene::Image *imageRGB)
{
// <<<>>>
return(true);
} // buildPointCloudGeometry



// namespace livescene
}
