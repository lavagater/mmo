#ifndef SHAPES_H
#define SHAPES_H

#include <unordered_map>
#include <vector>
#include <functional>
#include "Eigen/Dense"
#include "game_object.h"

//how big each cell in the collider grid is
#define GRID_SIZE 1

class Shape
{
public:
  virtual ~Shape(){}
  virtual int GetType() = 0;
  bool CheckCollision(Shape *);
  virtual std::vector<std::pair<int,int>> GetGridOccupation() = 0;
  //would have liked to decouple game objects and this, but I just want to get it over with
  GameObject *object;
};

class Circle : public Shape
{
public:
  double radius;
  int GetType() {return 0;}
  std::vector<std::pair<int,int>> GetGridOccupation();
};

bool CircleToCircle(Shape *, Shape *);

#endif