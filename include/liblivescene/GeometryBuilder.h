// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_GEOMETRYBUILDER_H__
#define __LIVESCENE_GEOMETRYBUILDER_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"
#include <algorithm>
#include <cassert>


namespace livescene {


/** \defgroup Geometry Geometry Building */
/*@{*/



/** \brief Class for managing the data responsibility associated with creating geometry objects/vertices/normals/texcoords

*/

class LIVESCENE_EXPORT Geometry
{
public:
	typedef enum
	{
		GEOMETRY_UNKNOWN             = 0,
		GEOMETRY_POINTS              = 1,
		GEOMETRY_FACES               = 2,
	} GeometryEntityType;

	Geometry() : _entityType(GEOMETRY_UNKNOWN),
		_vertices(0), _indices(0), _indicesTempBuffer(0), _normals(0), _texcoord(0),
		_numVertices(0), _numIndices(0), _numNormals(0), _numTexCoords(0),
		_numVerticesAllocated(0), _numIndicesAllocated(0), _numNormalsAllocated(0), _numTexCoordsAllocated(0),
		_width(0), _height(0), _meshEpsilonPercent(.015f) {}
	~Geometry();

	short *getVertices(void) const {return(_vertices);}
	unsigned int *getIndices(void) const {return(_indices);}
	float *getNormals(void) const {return(_normals);}
	float *getTexCoord(void) const {return(_texcoord);}

	unsigned int getNumVertices(void) const {return(_numVertices);}
	unsigned int getNumIndices(void) const {return(_numIndices);}
	unsigned int getNumNormals(void) const {return(_numNormals);}
	unsigned int getNumTexCoords(void) const {return(_numTexCoords);}

	int getWidth(void) const {return(_width);}
	int getHeight(void) const {return(_height);}

	GeometryEntityType getEntityType(void) const {return(_entityType);}

	// the mesh epsilon is the amount of z difference tolerated when comparing the candidate Z values
	// of adjacent samples to see if they can be connected with a mesh. It is expressed as a percent
	// of the Z value of the samples being compared, since Z precision is presumed to degrade with distance.
	void setMeshEpsilonPercent(const float meshEpsilonPercent) {_meshEpsilonPercent = meshEpsilonPercent;}
	float getMeshEpsilonPercent(void) const {return(_meshEpsilonPercent);}

	/** Build point cloud geometry vertex, and optional texcoord arrays from Z buffer, with optional nulling.
	imageZ is required, imageRGB is optional can can be NULL. Returns NULL for failure. */
	bool buildPointCloud(const livescene::Image &imageZ, const livescene::Image * const imageRGB);

	/** Build quads geometry vertex, normal and texcoord arrays from Z buffer, with optional nulling.
	This utilizes a hidden temporary buffer so as not to reallocate on each frame. Do not change image resolution
	once you've started using buildFaces(). */
	bool buildFaces(const livescene::Image &imageZ, const livescene::Image * const imageRGB);

	/** Build quads geometry vertex, normal and texcoord arrays from Z buffer, with optional nulling.
	This utilizes a hidden temporary buffer so as not to reallocate on each frame. Do not change image resolution
	once you've started using buildFaces().
	This variant is less sophisticated about making "nice" geometry and should be faster. */
	bool buildFacesSimple(const livescene::Image &imageZ, const livescene::Image * const imageRGB);

private:
	short *_vertices;
	unsigned int *_indices;
	unsigned int *_indicesTempBuffer; // tempbuffer is used to record which index a vertex/tx has already been recorded at (for reuse)
	float *_normals;
	float *_texcoord;
	int _width, _height;
	float _meshEpsilonPercent;
	GeometryEntityType _entityType;

	unsigned int _numVertices, _numIndices, _numNormals, _numTexCoords;
	unsigned int _numVerticesAllocated, _numIndicesAllocated, _numNormalsAllocated, _numTexCoordsAllocated;

	void freeData(void);
	void allocData(const unsigned int &numVert, const unsigned int &numId, const unsigned int &numTex, const unsigned int &numNorm);
	void freeTempBuffer(void);
	void allocTempBuffer(void);
	void clearTempBuffer(const unsigned int &_clearValue);

	inline bool isTriRangeMeshable(const short &valueA, const short &valueB, const short &valueC)
	{
		short delta = (short)((float)valueA * _meshEpsilonPercent);
		short minZ, maxZ;
		//short maxZtest = std::max(std::max(valueA, valueB), valueC); // total of two tests in best optimized case
		//short minZtest = std::min(std::min(valueA, valueB), valueC); // total of two tests in best optimized case
		// perhaps the above (four comparisons) can be done more efficiently
		// http://stackoverflow.com/questions/3343530/how-to-sort-three-variables-using-at-most-two-swaps
		// Below I derived a new implementation from a truth table
		// it takes between two and three comparisons, instead of the
		// guaranteed minimum four above

		// this process should only use three comparisons
		if (valueA < valueB)
		{
			if (valueB < valueC)
			{
				minZ = valueA;
				maxZ = valueC;
				// total of two tests performed
			} // if
			else
			{
				minZ = std::min(valueA, valueC);
				maxZ = valueB;
				// total of three tests performed
			} // else
		} // if
		else
		{
			if (valueB < valueC)
			{
				minZ = valueB;
				maxZ = std::max(valueA, valueC);
				// total of three tests performed
			} // if
			else
			{
				minZ = valueC;
				maxZ = valueA;
				// total of two tests performed
			} // else
		} // else

		//assert(minZ == minZtest);
		//assert(maxZ == maxZtest);

		return(maxZ - minZ <= delta);
	}

}; // Geometry









// namespace livescene
}

// __LIVESCENE_GEOMETRYBUILDER_H__
#endif
