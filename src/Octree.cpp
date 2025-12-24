#include "Octree.h"
#include <numeric>
#include <cassert>

Octree::Octree(const float* vertexData, size_t _numVertices,
			   float width, float height, float depth,
			   float x, float y, float z,
			   int _maxLevels, int _maxElems)
{
	assert(_maxLevels <= 5 && "Maximum depth support is 5");
	assert(_maxElems > 0 && "Maximum number of elements needs to be above 0");
	assert(_numVertices > 0 && "Number of vertices needs to be above 0");
	maxLevels = _maxLevels;
	maxElems = _maxElems;
	numVertices = _numVertices;

	Cell* topCell = new Cell();
	std::vector<int> _elemIndices(_numVertices);
	std::iota(_elemIndices.begin(), _elemIndices.end(), 0);
	// We need to iterate through the elements and generate Element links
	for (int i=0; i<_elemIndices.size(); i++)
	{
		elements.push_back(Element());
		Element* elem = &elements.back();
		elem->id = _elemIndices[i];
		elem->nextId = (i != (_elemIndices.size() - 1)) ? elements.size() : -1;
	}
	topCell->elementId = 0;
	topCell->width = width;
	topCell->height = height;
	topCell->depth = depth;
	topCell->level = 0;
	topCell->pos = float3(x, y, z);
	cells.push_back(*topCell);
	subdivCell(0, vertexData);
}

Octree::~Octree()
{
	cells.clear();
}

void Octree::subdivCell(int cellId, const float* vertexData)
{
	cells[cellId].childrenIndex = cells.size();
	for (int i = 0; i < 8; i++)
	{
		Cell* newCell = new Cell();
		newCell->width = cells[cellId].width / 2.0f;
		newCell->height = cells[cellId].height / 2.0f;
		newCell->depth = cells[cellId].depth / 2.0f;
		newCell->parentIndex = cellId;
		newCell->level = cells[cellId].level + 1;
		newCell->pos.x = cells[cellId].pos.x + newCell->width * (i & 1);
		newCell->pos.y = cells[cellId].pos.y + newCell->height * ((i >> 1) & 1);
		newCell->pos.z = cells[cellId].pos.z + newCell->depth * ((i >> 2) & 1);
		cells.push_back(*newCell);
	}
	// sort the topCell's elements into the leaf nodes.
	Element* currElem = &elements[cells[cellId].elementId];
	while (currElem->nextId != -1)
	{
		int nextId = currElem->nextId;
		float3 vertex;
		vertex.x = vertexData[3 * currElem->id + 0];
		vertex.y = vertexData[3 * currElem->id + 1];
		vertex.z = vertexData[3 * currElem->id + 2];
		float3 xc;
		xc.x = cells[cellId].pos.x + cells[cellId].width / 2.0f;
		xc.y = cells[cellId].pos.y + cells[cellId].height / 2.0f;
		xc.z = cells[cellId].pos.z + cells[cellId].depth / 2.0f;
		
		int index = (vertex.x > xc.x) | ((vertex.y > xc.y) << 1) | ((vertex.z > xc.z) << 2);
		// Fetch the cell
		Cell* trgCell = &cells[cells[cellId].childrenIndex + index];
		// Make a new element on the elements
		Element* newElem = new Element();
		newElem->id = currElem->id;
		newElem->nextId = -1;
		elements.push_back(*newElem);
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
			lastElem->nextId = elements.size()-1;
		}
		else
		{
			trgCell->elementId = elements.size()-1;
		}
		currElem = &elements[nextId];
	}
}
