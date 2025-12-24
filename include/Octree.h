#include <vector>
#include <memory>

struct float3 {
	float3() :x(0.0f), y(0.0f), z(0.0f) {};
	float3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	float x, y, z;
};

struct Element
{
	int id; // -1 means end of linked list
	int nextId;
};

struct Cell
{
	Cell()
	{
		parentIndex = -1;
		childrenIndex = -1;
		elementId = -1;
	}
	float3 pos;
	float width;
	float height;
	float depth;
	int level;
	int parentIndex;
	int childrenIndex;
	int elementId;
};

class Octree
{
public:
	Octree(const float* vertexData, size_t numVertices,
		   float width, float height, float depth,
		   float x, float y, float z,
		   int _maxLevels, int _maxElems);
	~Octree();
	void subdivCell(int cellId, const float* vertexData);
	std::vector<Cell> cells;
	std::vector<Element> elements;
private:
	size_t numVertices;
	int maxElems; // Maximum elements per cell
	int maxLevels; // Maximum number of levels - we do maximum of 5 for now.
};