#include "shapes.h"
#include "transform_component.h"
#include "utils.h"
#include "logger.h"

static std::unordered_map<std::pair<int, int>, std::function<bool(Shape*, Shape*)>, pair_hash> CollisionFunctions({
  {std::pair<int, int>(0,0),CircleToCircle},
  {std::pair<int, int>(0,1),CircleToLine}
});

bool Shape::CheckCollision(Shape *other)
{
  if (CollisionFunctions.find(std::make_pair(GetType(), other->GetType())) != CollisionFunctions.end())
  { 
    return CollisionFunctions[std::make_pair(GetType(), other->GetType())](this, other);
  }
  //try switching the parameters
  if (CollisionFunctions.find(std::make_pair(other->GetType(), GetType())) != CollisionFunctions.end())
  { 
    return CollisionFunctions[std::make_pair(other->GetType(), GetType())](other, this);
  }
  //if there is no function then they arnt colliding, am i right
  return false;
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

bool CircleToLine(Shape *lhs, Shape *rhs)
{
  Circle *circle = (Circle*)lhs;
  Line *line = (Line*)rhs;
  //possible short circuit if an endpoint is colliding with the circle
  double start_distance_sq = (GETCOMP(circle->object, TransformComponent)->position - line->start).squaredNorm();
  if (start_distance_sq < circle->radius * circle->radius)
  {
    return true;
  }
  double end_distance_sq = (GETCOMP(circle->object, TransformComponent)->position - line->end).squaredNorm();
  if (end_distance_sq < circle->radius * circle->radius)
  {
    return true;
  }
  Eigen::Vector2d direction = (line->end - line->start);
  double line_length = direction.norm();
  //check if the circle is too far away to possibly collide
  if (start_distance_sq > line_length*line_length + circle->radius)
  {
    return false;
  }
  if (end_distance_sq > line_length*line_length + circle->radius)
  {
    return false;
  }
  direction /= line_length;
  Eigen::Vector2d perp(direction.y(), -direction.x());
  double distance = abs(perp.dot(GETCOMP(circle->object, TransformComponent)->position - line->start));
  if (distance < circle->radius)
  {
    return true;
  }
  return false;
}

std::vector<std::pair<int,int>> Line::GetGridOccupation()
{
  std::vector<std::pair<int, int>> ret;
  std::pair<int, int> lower;
  std::pair<int, int> upper;
  if (start.x() < end.x())
  {
    lower.first = int(start.x()/GRID_SIZE);
    upper.first = int(end.x()/GRID_SIZE);
  }
  else
  {
    lower.first = int(end.x()/GRID_SIZE);
    upper.first = int(start.x()/GRID_SIZE);
  }
  if (start.y() < end.y())
  {
    lower.second = int(start.y()/GRID_SIZE);
    upper.second = int(end.y()/GRID_SIZE);
  }
  else
  {
    lower.second = int(end.y()/GRID_SIZE);
    upper.second = int(start.y()/GRID_SIZE);
  }
  //check is the line goes through 
  Eigen::Vector2d direction = (end - start).normalized();
  for (int i = lower.first; i <= upper.first; ++i)
  {
    for (int j = lower.second; j <= upper.second; j++)
    {
      Eigen::Vector2d center(i + GRID_SIZE/2.0, j + GRID_SIZE/2.0);
      Eigen::Vector2d projection = start + direction * (direction.dot(center - start));
      if (i == int(projection.x()/GRID_SIZE) && j == int(projection.y()/GRID_SIZE))
      {
        ret.push_back(std::make_pair(i,j));
      }
    }
  }
  return ret;
}