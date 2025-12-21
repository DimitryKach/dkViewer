#include "Eigen/Geometry"


class AABB
{
public:
	AABB()
	{

	};
	~AABB() = default;

	bool rayHit(Eigen::Vector2f& ray);

	float width;
	float height;
	float depth;

	Eigen::Vector3f pos;
};