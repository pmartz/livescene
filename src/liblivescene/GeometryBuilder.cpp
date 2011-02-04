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

void Geometry::allocData(unsigned int numVert, unsigned int numId, unsigned int numTex, unsigned int numNorm)
{
	freeData();
	_vertices = new short[numVert * 3]; // three shorts per vert
	_indices  = new unsigned int[numId];
	_texcoord = new float[numTex * 2]; // two floats per tc
	_normals  = new float[numNorm * 3]; // three floats per normal
} // Geometry::allocData


bool Geometry::buildPointCloud(const livescene::Image &imageZ, livescene::Image *imageRGB)
{
	_width = imageZ.getWidth();
	_height = imageZ.getHeight();

	// we have to allocate for worst-case, all vertices/indices used
	// but after the null processing loop below we'll reset these to
	// indicate the number actually used.
	// the resource tracking knows how many really need to be freed, so this is ok
	_numVertices  = imageZ.getSamples();
	_numIndices   = 0; // imageZ.getSamples(); // don't need indices for points
	_numTexCoords = imageZ.getSamples();
	_numNormals   = 0; // points don't need normals
	allocData(_numVertices, _numIndices, _numTexCoords, _numNormals);

	int width(imageZ.getWidth()), height(imageZ.getHeight());
	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	unsigned int loopSub(0), vertSub(0), indexSub(0), texSub(0), normSub(0);
	_zNull = imageZ.getNull();
	for(int line = 0; line < height; line++)
	{
		const float lineTC = (float)line / (float)height;
		for(int column = 0; column < width; column++)
		{
			const float columnTC = (float)column / (float)width;
			short originalDepth = depthBuffer[loopSub];
			if(isCellValueValid(originalDepth))
			{
				//_indices[indexSub] = vertSub;
				_vertices[vertSub++] = column;
				_vertices[vertSub++] = line;
				_vertices[vertSub++] = originalDepth;
				_texcoord[texSub++] = columnTC; // X
				_texcoord[texSub++] = lineTC; // Y
				indexSub++;
			} // if
			loopSub++;
		} // for
	} // for lines

	// record how many were actually _used_ so we don't try to draw uninitialized data
	_numVertices  = indexSub; // this is number of three-element vertices, not number of elements in the vertices array
	_numIndices   = 0; // didn't use them
	_numTexCoords = indexSub; // this is number of two-element texcoords, not number of elements in the texcoord array

	return(true);
} // buildPointCloud


bool Geometry::buildFaces(const livescene::Image &imageZ, livescene::Image *imageRGB)
{
	_width = imageZ.getWidth();
	_height = imageZ.getHeight();

	// we have to allocate for worst-case, all vertices/indices used
	// but after the null processing loop below we'll reset these to
	// indicate the number actually used.
	// the resource tracking knows how many really need to be freed, so this is ok

	int width(imageZ.getWidth()), height(imageZ.getHeight());

	// multiply all resource sizes by maxTrisPerSample(4) because each sample can be in up to 4 triangle polygons
	const int vertsPerTri(3), maxTrisPerCell(2);
	const int maxNumTris = (width - 1) * (height - 1) * maxTrisPerCell; // two tris per cell that spans two samples
	_numVertices  = maxNumTris * vertsPerTri;
	_numIndices   = 0; // maxNumTris * vertsPerTri; // we don't seem to be using indices, we're dumping vertices
	_numTexCoords = maxNumTris * vertsPerTri;
	_numNormals   = 0; // maxNumTris * vertsPerTri; // may not need normals if we're not lighting it
	allocData(_numVertices, _numIndices, _numTexCoords, _numNormals);

	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	// the meshing algorithm is excessively complicated because it normally splits
	// four-point cells into two three-point triangle with the split running UL-LR.
	// however, if that's not possible, but it can form one or the other triangle
	// if the split runs LL-UR, it will try to do so. This reduces sawtooth edges
	// along borders between data and no-data.
	unsigned int loopSub(0), vertSub(0), indexSub(0), texSub(0), normSub(0), polyCount(0);
	_zNull = imageZ.getNull();
	for(int line = 0; line < height - 1; line++) // NOTE: height - 1
	{
		const float lineTC = (float)line / (float)height;
		const float linePlusOneTC = (float)(line + 1) / (float)height;
		for(int column = 0; column < width - 1; column++) // NOTE: width - 1
		{
			const unsigned int loopSub = line * width + column;
			const float columnTC = (float)column / (float)width;
			const float columnPlusOneTC = (float)(column + 1) / (float)width;
			short originalDepth = depthBuffer[loopSub];
			// is top-left sample of mesh cell (at loop subscripts) non-rejected?
			if(isCellValueValid(originalDepth))
			{
				// is LR vertex valid, allowing for potentially both polygons split UL<->LR?
				if(isCellValueValid(depthBuffer[(line + 1)* width + column + 1]))
				{ // try to form two triangles, UL, LL, LR and UL, LR, UR
					// is LL valid, allowing UL, LL, LR?
					if(isCellValueValid(depthBuffer[(line + 1)* width + column]))
					{
						// form UL, LL, LR triangle
						// UL
						_vertices[vertSub++] = column;
						_vertices[vertSub++] = line;
						_vertices[vertSub++] = originalDepth;
						_texcoord[texSub++] = columnTC; // X
						_texcoord[texSub++] = lineTC; // Y
						indexSub++;
						// LL
						_vertices[vertSub++] = column;
						_vertices[vertSub++] = line + 1;
						_vertices[vertSub++] = depthBuffer[(line + 1)* width + column];
						_texcoord[texSub++] = columnTC; // X
						_texcoord[texSub++] = linePlusOneTC; // Y
						indexSub++;
						// LR
						_vertices[vertSub++] = column + 1;
						_vertices[vertSub++] = line + 1;
						_vertices[vertSub++] = depthBuffer[(line + 1)* width + column + 1];
						_texcoord[texSub++] = columnPlusOneTC; // X
						_texcoord[texSub++] = linePlusOneTC; // Y
						indexSub++;

						polyCount++;
					} // if
					
					// is UR valid, allowing UL, LR, UR?
					if(isCellValueValid(depthBuffer[line * width + column + 1]))
					{
						// form UL, LR, UR triangle
						// UL
						_vertices[vertSub++] = column;
						_vertices[vertSub++] = line;
						_vertices[vertSub++] = originalDepth;
						_texcoord[texSub++] = columnTC; // X
						_texcoord[texSub++] = lineTC; // Y
						indexSub++;
						// LR
						_vertices[vertSub++] = column + 1;
						_vertices[vertSub++] = line + 1;
						_vertices[vertSub++] = depthBuffer[(line + 1)* width + column + 1];
						_texcoord[texSub++] = columnPlusOneTC; // X
						_texcoord[texSub++] = linePlusOneTC; // Y
						indexSub++;
						// UR
						_vertices[vertSub++] = column + 1;
						_vertices[vertSub++] = line;
						_vertices[vertSub++] = depthBuffer[line * width + column + 1];
						_texcoord[texSub++] = columnPlusOneTC; // X
						_texcoord[texSub++] = lineTC; // Y
						indexSub++;

						polyCount++;
					} // if
				} // if
				// are at least UR and LL left valid, allowing one UL, LL, UR?
				else if(isCellValueValid(depthBuffer[(line + 1)* width + column])
					&& isCellValueValid(depthBuffer[line * width + column + 1]))
				{
					// form UL, LL, UR triangle
					// UL
					_vertices[vertSub++] = column;
					_vertices[vertSub++] = line;
					_vertices[vertSub++] = originalDepth;
					_texcoord[texSub++] = columnTC; // X
					_texcoord[texSub++] = lineTC; // Y
					indexSub++;
					// LL
					_vertices[vertSub++] = column;
					_vertices[vertSub++] = line + 1;
					_vertices[vertSub++] = depthBuffer[(line + 1)* width + column];
					_texcoord[texSub++] = columnTC; // X
					_texcoord[texSub++] = linePlusOneTC; // Y
					indexSub++;
					// UR
					_vertices[vertSub++] = column + 1;
					_vertices[vertSub++] = line;
					_vertices[vertSub++] = depthBuffer[line * width + column + 1];
					_texcoord[texSub++] = columnPlusOneTC; // X
					_texcoord[texSub++] = lineTC; // Y
					indexSub++;

					polyCount++;
				} // else if
			} // if
			// we still might make a triangle out of UR, LR, LL if all are valid even if UL is NULL
			else if(isCellValueValid(depthBuffer[(line + 1)* width + column])
			 && isCellValueValid(depthBuffer[line * width + column + 1])
			 && isCellValueValid(depthBuffer[(line + 1)* width + column + 1]) )
			{
				// form LL, UR, LR triangle
				// LL
				_vertices[vertSub++] = column;
				_vertices[vertSub++] = line + 1;
				_vertices[vertSub++] = depthBuffer[(line + 1)* width + column];
				_texcoord[texSub++] = columnTC; // X
				_texcoord[texSub++] = linePlusOneTC; // Y
				indexSub++;
				// UR
				_vertices[vertSub++] = column + 1;
				_vertices[vertSub++] = line;
				_vertices[vertSub++] = depthBuffer[line * width + column + 1];
				_texcoord[texSub++] = columnPlusOneTC; // X
				_texcoord[texSub++] = lineTC; // Y
				indexSub++;
				// LR
				_vertices[vertSub++] = column + 1;
				_vertices[vertSub++] = line + 1;
				_vertices[vertSub++] = depthBuffer[(line + 1)* width + column + 1];
				_texcoord[texSub++] = columnPlusOneTC; // X
				_texcoord[texSub++] = linePlusOneTC; // Y
				indexSub++;

				polyCount++;
			} // else
		} // for
	} // for lines

	_numVertices  = indexSub; // this is number of three-element vertices, not number of elements in the vertices array
	_numIndices   = 0; // didn't use them
	_numTexCoords = indexSub; // this is number of two-element texcoords, not number of elements in the texcoord array

	return(true);
} // buildPointCloudGeometry



// namespace livescene
}
