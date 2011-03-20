// Copyright 2011 Skew Matrix Software and AlphaPixel

#include "liblivescene/GeometryBuilder.h"
#include <limits>
#include <cstring> // memset

namespace livescene {

Geometry::~Geometry()
{
	freeData();
	freeTempBuffer();
} // Geometry::Geometry

void Geometry::freeTempBuffer(void)
{
	delete [] _indicesTempBuffer; _indicesTempBuffer = 0;
} // Geometry::freeTempBuffer

void Geometry::freeData(void)
{
	delete [] _vertices; _vertices = 0; _numVerticesAllocated = 0;
	delete [] _indices; _indices = 0; _numIndicesAllocated = 0;
	delete [] _normals; _normals = 0; _numNormalsAllocated = 0;
	delete [] _texcoord; _texcoord = 0; _numTexCoordsAllocated = 0;
} // Geometry::freeData

void Geometry::allocData(const unsigned int &numVert, const unsigned int &numId, const unsigned int &numTex, const unsigned int &numNorm)
{
	// is our existing allocation sufficiently large that it be reused?
	if(numVert <= _numVerticesAllocated
		&& numId <= _numIndicesAllocated
		&& numTex <= _numTexCoordsAllocated
		&& numNorm <= _numNormalsAllocated)
	{
		// yes, everything is fine
		return;
	} // if

	// otherwise, throw out the existing allocation and get a bigger one
	freeData();
	_vertices = new short[numVert * 3]; // three shorts per vert
	_numVerticesAllocated = numVert; // record current allocation size for later reuse comparison
	_indices  = new unsigned int[numId];
	_numIndicesAllocated = numId; // record current allocation size for later reuse comparison
	_texcoord = new float[numTex * 2]; // two floats per tc
	_numTexCoordsAllocated = numTex; // record current allocation size for later reuse comparison
	_normals  = new float[numNorm * 3]; // three floats per normal
	_numNormalsAllocated = numNorm; // record current allocation size for later reuse comparison
} // Geometry::allocData


void Geometry::allocTempBuffer(void)
{
	if(!_indicesTempBuffer)
		_indicesTempBuffer = new unsigned int[getHeight() * getWidth()];
} // Geometry::allocTempBuffer


void Geometry::clearTempBuffer(const unsigned int &_clearValue)
{
	std::fill( _indicesTempBuffer, _indicesTempBuffer + (getHeight() * getWidth()), _clearValue );

} // Geometry::clearTempBuffer




bool Geometry::buildPointCloud(const livescene::Image &imageZ, const livescene::Image * const imageRGB)
{
	_entityType = GEOMETRY_POINTS;
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
	const float invWidth(1.0f/ width), invHeight(1.0f / height);
	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	unsigned int loopSub(0), vertSub(0), indexSub(0), texSub(0), normSub(0);
	for(int line = 0; line < height; ++line)
	{
		const float lineTC = (float)line * invHeight; // inverse multiply
		for(int column = 0; column < width; ++column)
		{
			const float columnTC = (float)column * invWidth; // inverse multiply
			short originalDepth = depthBuffer[loopSub];
			if(imageZ.isCellValueValid(originalDepth))
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


bool Geometry::buildFaces(const livescene::Image &imageZ, const livescene::Image * const imageRGB)
{
	// define our "unused" value, since 0 is a valid value
	const unsigned int _tempBufferUnusedValue = std::numeric_limits<unsigned int>::max();
	_entityType = GEOMETRY_FACES;
	_width = imageZ.getWidth();
	_height = imageZ.getHeight();

	// we have to allocate for worst-case, all vertices/indices used
	// but after the null processing loop below we'll reset these to
	// indicate the number actually used.
	// the resource tracking knows how many really need to be freed, so this is ok

	int width(imageZ.getWidth()), height(imageZ.getHeight());
	const float invWidth(1.0f/ width), invHeight(1.0f / height);

	// because each sample can be in up to 4 triangle polygons
	const int vertsPerTri(3), maxTrisPerCell(2), totalSamples(width * height);
	const int maxNumTris = (width - 1) * (height - 1) * maxTrisPerCell; // two tris per cell that spans two samples
	_numVertices  = totalSamples; // one per sample
	_numIndices   = maxNumTris * vertsPerTri; // we recycle vertices, but indices are not duplicated
	_numTexCoords = totalSamples; // one per sample
	_numNormals   = 0; // width * height; // may not need normals if we're not lighting it
	allocData(_numVertices, _numIndices, _numTexCoords, _numNormals);
	allocTempBuffer(); // will only allocate if not already allocated. Must be done after allocData
	// we need to clear the array each time through, even if it's already allocated
	clearTempBuffer(_tempBufferUnusedValue); // clear to "unused" value


	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	// the meshing algorithm is excessively complicated because it normally splits
	// four-point cells into two three-point triangle with the split running UL-LR.
	// however, if that's not possible, but it can form one or the other triangle
	// if the split runs LL-UR, it will try to do so. This reduces sawtooth edges
	// along borders between data and no-data.
	unsigned int loopSub(0), vertSub(0), vertCount(0), indexSub(0), texSub(0), normSub(0), polyCount(0);
	for(int line = 0; line < height - 1; ++line) // NOTE: height - 1
	{
		const float lineTC = (float)line * invHeight; // inverse multiply
		const float linePlusOneTC = (float)(line + 1) * invHeight; // inverse multiply
		for(int column = 0; column < width - 1; ++column) // NOTE: width - 1
		{
			const unsigned int loopSub = line * width + column;
			const unsigned int loopSubPlusOneColumn = line * width + column + 1;
			const unsigned int loopSubPlusOneRow = (line + 1)* width + column;
			const unsigned int loopSubPlusOneRowColumn = (line + 1)* width + column + 1;

			const float columnTC = (float)column * invWidth; // inverse multiply
			const float columnPlusOneTC = (float)(column + 1) * invWidth; // inverse multiply
			// preread these four since we'll need them repeatedly
			short depthUL = depthBuffer[loopSub];
			short depthUR = depthBuffer[loopSubPlusOneColumn];
			short depthLL = depthBuffer[loopSubPlusOneRow];
			short depthLR = depthBuffer[loopSubPlusOneRowColumn];
			// is top-left sample of mesh cell (at loop subscripts) non-rejected?
			if(imageZ.isCellValueValid(depthUL))
			{
				// is LR vertex valid, allowing for potentially both polygons split UL<->LR?
				if(imageZ.isCellValueValid(depthLR))
				{ // try to form two triangles, UL, LL, LR and UL, LR, UR
					// is LL valid, allowing UL, LL, LR?
					if(imageZ.isCellValueValid(depthLL)
						&& isTriRangeMeshable(depthUL, depthLL, depthLR))
					{
						// form UL, LL, LR triangle
						// UL
						if(_indicesTempBuffer[loopSub] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSub] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSub];

						// LL
						if(_indicesTempBuffer[loopSubPlusOneRow] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRow] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRow];

						// LR
						if(_indicesTempBuffer[loopSubPlusOneRowColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRowColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRowColumn];

						polyCount++;
					} // if
					
					// is UR valid, allowing UL, LR, UR?
					if(imageZ.isCellValueValid(depthUR)
						&& isTriRangeMeshable(depthUL, depthLR, depthUR))
					{
						// form UL, LR, UR triangle
						// UL
						if(_indicesTempBuffer[loopSub] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSub] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSub];

						// LR
						if(_indicesTempBuffer[loopSubPlusOneRowColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRowColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRowColumn];

						// UR
						if(_indicesTempBuffer[loopSubPlusOneColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneColumn];

						polyCount++;
					} // if
				} // if
				// are at least UR and LL left valid, allowing one UL, LL, UR?
				else if(imageZ.isCellValueValid(depthLL) && imageZ.isCellValueValid(depthUR)
					&& isTriRangeMeshable(depthUL, depthLL, depthUR))
				{
					// form UL, LL, UR triangle
					// UL
					if(_indicesTempBuffer[loopSub] == _tempBufferUnusedValue)
					{
						// record where we put this sample's vertex/texcoord for later reference
						_indicesTempBuffer[loopSub] = vertCount++;
						// and then add the vertex where we said we would
						_vertices[vertSub++] = column;
						_vertices[vertSub++] = line;
						_vertices[vertSub++] = depthUL;
						_texcoord[texSub++] = columnTC; // X
						_texcoord[texSub++] = lineTC; // Y
					} // if
					// add a reference to where the vertex is already stored
					_indices[indexSub++] = _indicesTempBuffer[loopSub];

					// LL
					if(_indicesTempBuffer[loopSubPlusOneRow] == _tempBufferUnusedValue)
					{
						// record where we put this sample's vertex/texcoord for later reference
						_indicesTempBuffer[loopSubPlusOneRow] = vertCount++;
						// and then add the vertex where we said we would
						_vertices[vertSub++] = column;
						_vertices[vertSub++] = line + 1;
						_vertices[vertSub++] = depthLL;
						_texcoord[texSub++] = columnTC; // X
						_texcoord[texSub++] = linePlusOneTC; // Y
					} // if
					// add a reference to where the vertex is already stored
					_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRow];

					// UR
					if(_indicesTempBuffer[loopSubPlusOneColumn] == _tempBufferUnusedValue)
					{
						// record where we put this sample's vertex/texcoord for later reference
						_indicesTempBuffer[loopSubPlusOneColumn] = vertCount++;
						// and then add the vertex where we said we would
						_vertices[vertSub++] = column + 1;
						_vertices[vertSub++] = line;
						_vertices[vertSub++] = depthUR;
						_texcoord[texSub++] = columnPlusOneTC; // X
						_texcoord[texSub++] = lineTC; // Y
					} // if
					// add a reference to where the vertex is already stored
					_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneColumn];

					polyCount++;
				} // else if
			} // if
			// we still might make a triangle out of UR, LR, LL if all are valid even if UL is NULL
			else if(imageZ.isCellValueValid(depthLL) && imageZ.isCellValueValid(depthUR) && imageZ.isCellValueValid(depthLR)
				&& isTriRangeMeshable(depthLL, depthUR, depthLR))
			{
				// form LL, UR, LR triangle
				// LL
				if(_indicesTempBuffer[loopSubPlusOneRow] == _tempBufferUnusedValue)
				{
					// record where we put this sample's vertex/texcoord for later reference
					_indicesTempBuffer[loopSubPlusOneRow] = vertCount++;
					// and then add the vertex where we said we would
					_vertices[vertSub++] = column;
					_vertices[vertSub++] = line + 1;
					_vertices[vertSub++] = depthLL;
					_texcoord[texSub++] = columnTC; // X
					_texcoord[texSub++] = linePlusOneTC; // Y
				} // if
				// add a reference to where the vertex is already stored
				_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRow];

				// UR
				if(_indicesTempBuffer[loopSubPlusOneColumn] == _tempBufferUnusedValue)
				{
					// record where we put this sample's vertex/texcoord for later reference
					_indicesTempBuffer[loopSubPlusOneColumn] = vertCount++;
					// and then add the vertex where we said we would
					_vertices[vertSub++] = column + 1;
					_vertices[vertSub++] = line;
					_vertices[vertSub++] = depthUR;
					_texcoord[texSub++] = columnPlusOneTC; // X
					_texcoord[texSub++] = lineTC; // Y
				} // if
				// add a reference to where the vertex is already stored
				_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneColumn];

				// LR
				if(_indicesTempBuffer[loopSubPlusOneRowColumn] == _tempBufferUnusedValue)
				{
					// record where we put this sample's vertex/texcoord for later reference
					_indicesTempBuffer[loopSubPlusOneRowColumn] = vertCount++;
					// and then add the vertex where we said we would
					_vertices[vertSub++] = column + 1;
					_vertices[vertSub++] = line + 1;
					_vertices[vertSub++] = depthLR;
					_texcoord[texSub++] = columnPlusOneTC; // X
					_texcoord[texSub++] = linePlusOneTC; // Y
				} // if
				// add a reference to where the vertex is already stored
				_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRowColumn];

				polyCount++;
			} // else
		} // for
	} // for lines

	_numVertices  = vertCount; // this is number of three-element vertices, not number of elements in the vertices array
	_numIndices   = indexSub;
	_numTexCoords = vertCount; // this is number of two-element texcoords, not number of elements in the texcoord array

	return(true);
} // buildFaces


bool Geometry::buildFacesSimple(const livescene::Image &imageZ, const livescene::Image * const imageRGB)
{
	// define our "unused" value, since 0 is a valid value
	const unsigned int _tempBufferUnusedValue = std::numeric_limits<unsigned int>::max();
	_entityType = GEOMETRY_FACES;
	_width = imageZ.getWidth();
	_height = imageZ.getHeight();

	// we have to allocate for worst-case, all vertices/indices used
	// but after the null processing loop below we'll reset these to
	// indicate the number actually used.
	// the resource tracking knows how many really need to be freed, so this is ok

	int width(imageZ.getWidth()), height(imageZ.getHeight());
	const float invWidth(1.0f/ width), invHeight(1.0f / height);

	// because each sample can be in up to 4 triangle polygons
	const int vertsPerTri(3), maxTrisPerCell(2), totalSamples(width * height);
	const int maxNumTris = (width - 1) * (height - 1) * maxTrisPerCell; // two tris per cell that spans two samples
	_numVertices  = totalSamples; // one per sample
	_numIndices   = maxNumTris * vertsPerTri; // we recycle vertices, but indices are not duplicated
	_numTexCoords = totalSamples; // one per sample
	_numNormals   = 0; // width * height; // may not need normals if we're not lighting it
	allocData(_numVertices, _numIndices, _numTexCoords, _numNormals);
	allocTempBuffer(); // will only allocate if not already allocated. Must be done after allocData
	// we need to clear the array each time through, even if it's already allocated
	clearTempBuffer(_tempBufferUnusedValue); // clear to "unused" value


	short *depthBuffer = (short *)imageZ.getData();

	// loop logic taken from libfreenect glpclview, DrawGLScene()
	// This meshing algorithm is simpler because it only splits
	// four-point cells into two three-point triangle with the split running UL-LR.
	unsigned int loopSub(0), vertSub(0), vertCount(0), indexSub(0), texSub(0), normSub(0), polyCount(0);
	for(int line = 0; line < height - 1; ++line) // NOTE: height - 1
	{
		const float lineTC = (float)line * invHeight; // inverse multiply
		const float linePlusOneTC = (float)(line + 1) * invHeight; // inverse multiply
		for(int column = 0; column < width - 1; ++column) // NOTE: width - 1
		{
			const unsigned int loopSub = line * width + column;
			const unsigned int loopSubPlusOneColumn = line * width + column + 1;
			const unsigned int loopSubPlusOneRow = (line + 1)* width + column;
			const unsigned int loopSubPlusOneRowColumn = (line + 1)* width + column + 1;

			const float columnTC = (float)column * invWidth; // inverse multiply
			const float columnPlusOneTC = (float)(column + 1) * invWidth; // inverse multiply
			// pre-read these four since we'll need them repeatedly
			short depthUL = depthBuffer[loopSub];
			short depthUR = depthBuffer[loopSubPlusOneColumn];
			short depthLL = depthBuffer[loopSubPlusOneRow];
			short depthLR = depthBuffer[loopSubPlusOneRowColumn];
			// is top-left sample of mesh cell (at loop subscripts) non-rejected?
			if(imageZ.isCellValueValid(depthUL))
			{
				// is LR vertex valid, allowing for potentially both polygons split UL<->LR?
				if(imageZ.isCellValueValid(depthLR))
				{ // try to form two triangles, UL, LL, LR and UL, LR, UR
					// is LL valid, allowing UL, LL, LR?
					if(imageZ.isCellValueValid(depthLL)
						&& isTriRangeMeshable(depthUL, depthLL, depthLR))
					{
						// form UL, LL, LR triangle
						// UL
						if(_indicesTempBuffer[loopSub] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSub] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSub];

						// LL
						if(_indicesTempBuffer[loopSubPlusOneRow] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRow] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRow];

						// LR
						if(_indicesTempBuffer[loopSubPlusOneRowColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRowColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRowColumn];

						polyCount++;
					} // if

					// is UR valid, allowing UL, LR, UR?
					if(imageZ.isCellValueValid(depthUR)
						&& isTriRangeMeshable(depthUL, depthLR, depthUR))
					{
						// form UL, LR, UR triangle
						// UL
						if(_indicesTempBuffer[loopSub] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSub] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUL;
							_texcoord[texSub++] = columnTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSub];

						// LR
						if(_indicesTempBuffer[loopSubPlusOneRowColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneRowColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line + 1;
							_vertices[vertSub++] = depthLR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = linePlusOneTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneRowColumn];

						// UR
						if(_indicesTempBuffer[loopSubPlusOneColumn] == _tempBufferUnusedValue)
						{
							// record where we put this sample's vertex/texcoord for later reference
							_indicesTempBuffer[loopSubPlusOneColumn] = vertCount++;
							// and then add the vertex where we said we would
							_vertices[vertSub++] = column + 1;
							_vertices[vertSub++] = line;
							_vertices[vertSub++] = depthUR;
							_texcoord[texSub++] = columnPlusOneTC; // X
							_texcoord[texSub++] = lineTC; // Y
						} // if
						// add a reference to where the vertex is already stored
						_indices[indexSub++] = _indicesTempBuffer[loopSubPlusOneColumn];

						polyCount++;
					} // if
				} // if
			} // if
		} // for
	} // for lines

	_numVertices  = vertCount; // this is number of three-element vertices, not number of elements in the vertices array
	_numIndices   = indexSub;
	_numTexCoords = vertCount; // this is number of two-element texcoords, not number of elements in the texcoord array

	return(true);
} // buildFacesSimple


// namespace livescene
}
