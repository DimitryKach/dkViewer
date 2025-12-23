#include "Octree.h"
#include <numeric>
#include <cassert>

Octree::Octree(const float* vertexData, size_t _numVertices, float width, float height, float depth, int _maxLevels, int _maxElems)
{
	assert(_maxLevels <= 5 && "Maximum depth support is 5");
	assert(_maxElems > 0 && "Maximum number of elements needs to be above 0");
	assert(_numVertices > 0 && "Number of vertices needs to be above 0");
	maxLevels = _maxLevels;
	maxElems = _maxElems;
	numVertices = _numVertices;

	cells.push_back(Cell());
	Cell* topCell = &cells[0];
	std::vector<int> _elemIndices(_numVertices);
	std::iota(_elemIndices.begin(), _elemIndices.end(), 0);
	// We need to iterate through the elements and generate Element links
	for (int i=0; i<_elemIndices.size(); i++)
	{
		elements.push_back(Element());
		Element* elem = &elements[-1];
		elem->id = _elemIndices[i];
		elem->nextId = (i < (_elemIndices.size() - 1)) ? elements.size() : -1;
	}
	topCell->elementId = 0;
	topCell->width = width;
	topCell->height = height;
	topCell->depth = depth;
	topCell->level = 0;
	subdivCell(0, vertexData);
}

Octree::~Octree()
{
	cells.clear();
}

void Octree::subdivCell(int cellId, const float* vertexData)
{
	Cell* topCell = &cells[cellId];
	topCell->childrenIndex = cells.size();
	for (int i = 0; i < 8; i++)
	{
		cells.push_back(Cell());
		Cell* newCell = &cells[-1];
		newCell->width = topCell->width / 2.0f;
		newCell->height = topCell->height / 2.0f;
		newCell->depth = topCell->depth / 2.0f;
		newCell->parentIndex = cellId;
		newCell->level = topCell->level + 1;
		newCell->pos.x = topCell->pos.x + newCell->width * (i & 1);
		newCell->pos.y = topCell->pos.y + newCell->height * ((i >> 1) & 1);
		newCell->pos.z = topCell->pos.z + newCell->depth * ((i >> 2) & 1);
	}
	// sort the topCell's elements into the leaf nodes.
	Element* currElem = &elements[topCell->elementId];
	while (currElem->nextId != -1)
	{
		float3 vertex;
		vertex.x = vertexData[3 * currElem->id + 0];
		vertex.y = vertexData[3 * currElem->id + 1];
		vertex.z = vertexData[3 * currElem->id + 2];
		float3 xc;
		xc.x = topCell->pos.x + topCell->width / 2.0f;
		xc.y = topCell->pos.y + topCell->height / 2.0f;
		xc.z = topCell->pos.z + topCell->depth / 2.0f;
		
		int index = (vertex.x > xc.x) | ((vertex.y > xc.y) << 1) | ((vertex.z > xc.z) << 2);
		// Fetch the cell
		Cell* trgCell = &cells[topCell->childrenIndex + index];
		// Make a new element on the elements
		elements.push_back(Element());
		Element* newElem = &elements[-1];
		newElem->id = currElem->id;
		newElem->nextId = -1;
		// Check if the trgCell has already any elements
		if (trgCell->elementId != -1)
		{
			Element* lastElem = &elements[trgCell->elementId];
			// We need to traverse till we find the last linked element
			while (lastElem->nextId != -1)
			{
				lastElem = &elements[lastElem->nextId];
			}
			// Now that the lastElem has been found, we need to link it to the newElem that we made
			lastElem->nextId = elements.size();
		}
		currElem = &elements[currElem->nextId];
	}
}
