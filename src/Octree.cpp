#include "Octree.h"
#include <numeric>

Octree::Octree(std::vector<Eigen::Vector3f>& vertsPoints, int numLevels, bool precomp)
{
	verts = &vertsPoints;
	// Initializes one main top cell that will be dynamically refined per-query.
	float minX = std::numeric_limits<double>::infinity();
	float minY = std::numeric_limits<double>::infinity();
	float minZ = std::numeric_limits<double>::infinity();
	float maxX = -std::numeric_limits<double>::infinity();
	float maxY = -std::numeric_limits<double>::infinity();
	float maxZ = -std::numeric_limits<double>::infinity();
	
	// use vertex bounds to define the AABB cell
	for (auto vert : *verts)
	{
		if (vert[0] < minX) minX = vert[0];
		if (vert[0] > maxX) maxX = vert[0];
		if (vert[1] < minY) minY = vert[1];
		if (vert[1] > maxY) maxY = vert[1];
		if (vert[2] < minZ) minZ = vert[2];
		if (vert[2] > maxZ) maxZ = vert[2];
	}

	float width = maxX - minX;
	float depth = maxZ - minZ;
	float height = maxY - minY;
	// just for fun :) Find the longest side (oh boy is this ugly...)
	float dim = (width > height) ? ((width > depth) ? width : depth) : ((height > depth) ? height : depth);

	Eigen::Vector3f xp(minX, minY, minZ);
	
	auto topCell = std::make_shared<Cell>(xp);
	topCell->id = -1; // -1 is top cell ID
	topCell->level = 0;
	topCell->pos = xp;
	std::vector<int> indices(vertsPoints.size());
	std::iota(indices.begin(), indices.end(), 0);
	topCell->vIds = std::move(indices);

	cells.push_back(topCell);
	if (precomp)
		subdivCell(topCell, numLevels, true);
}

Octree::~Octree()
{
	cells.clear();
}

void Octree::addVertices(std::vector<int>& vIds, int cellId)
{

}

void Octree::subdivCell(std::shared_ptr<Cell>& cell, int maxLvl, bool recurse)
{
	for (int i = 0; i < 8; i++)
	{
		auto child = std::make_shared<Cell>(cell, i);
		cell->children.push_back(child);
	}
	// sort the cells vertices into it's leaf nodes.
	Eigen::Vector3f xp = cell->pos;
	Eigen::Vector3f xc = xp + Eigen::Vector3f(cell->width / 2.0, cell->height / 2.0, cell->depth / 2.0);
	for (auto vId : cell->vIds)
	{
		Eigen::Vector3f dX = (*verts)[vId] - xc;
		// Shift and OR
		// x stays at bit 0 | y shifts to bit 1 | z shifts to bit 2
		int index = ((dX[0]>0) | ((dX[1]>0) << 1) | ((dX[2]>0) << 2)) + 7*cell->level;
	}
}
