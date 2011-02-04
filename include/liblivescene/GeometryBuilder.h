// Copyright 2011 Skew Matrix Software and AlphaPixel

#ifndef __LIVESCENE_GEOMETRYBUILDER_H__
#define __LIVESCENE_GEOMETRYBUILDER_H__ 1

#include "liblivescene/Export.h"
#include "liblivescene/Image.h"


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

	Geometry() : _vertices(0), _indices(0), _normals(0), _texcoord(0), _entityType(GEOMETRY_UNKNOWN),
		_numVertices(0), _numIndices(0), _numNormals(0), _numTexCoords(0), _width(0), _height(0) {}
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

	/** Build point cloud geometry vertex, and optional texcoord arrays from Z buffer, with optional nulling.
	imageZ is required, imageRGB is optional can can be NULL. Returns NULL for failure. */
	bool buildPointCloud(const livescene::Image &imageZ, livescene::Image *imageRGB);

	/** Build quads geometry vertex, normal and texcoord arrays from Z buffer, with optional nulling. */
	bool buildFaces(const livescene::Image &imageZ, livescene::Image *imageRGB);

private:
	short *_vertices;
	unsigned int *_indices;
	float *_normals;
	float *_texcoord;
	int _width, _height;
	int _zNull;
	GeometryEntityType _entityType;

	unsigned int _numVertices, _numIndices, _numNormals, _numTexCoords;

	void freeData(void);
	void allocData(unsigned int numVert, unsigned int numId, unsigned int numTex, unsigned int numNorm);

	inline bool isCellValueValid(short value) {return(value != _zNull && value != 0);}

}; // Geometry









// namespace livescene
}

// __LIVESCENE_GEOMETRYBUILDER_H__
#endif
