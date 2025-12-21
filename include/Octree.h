#include <vector>
#include "Eigen/Geometry"
#include <memory>

struct Cell
{
	Cell(Eigen::Vector3f& _pos)
	{
		pos = pos;
	}
	// Create given a parent
	Cell(const std::shared_ptr<Cell>& _par, int _localId)
	{
		parent = _par;
		width = _par->width / 2.0;
		height = _par->height / 2.0;
		depth = _par->depth / 2.0;
		float dx = (_localId == 0 || _localId == 2 || _localId == 4 || _localId == 6) ? 0.0f : width;
		float dy = (_localId == 0 || _localId == 1 || _localId == 4 || _localId == 5) ? 0.0f : height;
		float dz = (_localId == 0 || _localId == 1 || _localId == 2 || _localId == 3) ? 0.0f : depth;
		pos = _par->pos + Eigen::Vector3f(dx, dy, dz);
		id = (_par->id == -1) ? _localId : _par->id + _localId;
		level = _par->level + 1;
	}
	~Cell()
	{
		children.clear();
	}
	std::shared_ptr<Cell> parent;
	std::vector<std::shared_ptr<Cell>> children;
	Eigen::Vector3f position;
	// TODO: Is this too much memory usage?
	std::vector<int> vIds;
	Eigen::Vector3f pos;
	float width;
	float height;
	float depth;
	int id;
	int level;
};

class Octree
{
public:
	Octree(std::vector<Eigen::Vector3f>& points, int levels, bool precomp);
	~Octree();
	void addVertices(std::vector<int>& vIds, int cellId);
	void subdivCell(std::shared_ptr<Cell>& cell, int maxLvl, bool recurse);
private:
	std::vector<std::shared_ptr<Cell>> cells;
	int maxVerts;
	std::vector<Eigen::Vector3f>* verts;
};