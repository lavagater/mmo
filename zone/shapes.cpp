#include "shapes.h"
#include "transform_component.h"
#include "utils.h"
#include "logger.h"

static std::unordered_map<std::pair<int, int>, std::function<bool(Shape*, Shape*)>, pair_hash> CollisionFunctions({
  {std::pair<int, int>(0,0),CircleToCircle}
});

bool Shape::CheckCollision(Shape *other)
{
  return CollisionFunctions[std::make_pair(GetType(), other->GetType())](this, other);
}

bool CircleToCircle(Shape *lhs, Shape *rhs)
{
  Circle *circle1 = (Circle*)lhs;
  Circle *circle2 = (Circle*)rhs;
  return (GETCOMP(circle1->object, TransformComponent)->position - GETCOMP(circle2->object, TransformComponent)->position).squaredNorm() <= (circle1->radius + circle2->radius) * (circle1->radius + circle2->radius);
}

std::vector<std::pair<int,int>> Circle::GetGridOccupation()
{
  std::vector<std::pair<int, int>> ret;
  Eigen::Vector2d lower_bound = GETCOMP(object, TransformComponent)->position - Eigen::Vector2d(radius, radius);
  Eigen::Vector2d upper_bound = lower_bound + 2*Eigen::Vector2d(radius, radius);
  std::pair<int, int> lower = std::make_pair(int(lower_bound.x()/GRID_SIZE), int(lower_bound.y()/GRID_SIZE));
  std::pair<int, int> upper = std::make_pair(int(upper_bound.x()/GRID_SIZE), int(upper_bound.y()/GRID_SIZE));
  ret.push_back(lower);
  if (lower != upper)
  {
    //add all the grids between lower and upper
    for (int i = lower.first; i <= upper.first; ++i)
    {
      for (int j = lower.second; j <= upper.second; j++)
      {
        ret.push_back(std::make_pair(i,j));
      }
    }
  }
  return ret;
}