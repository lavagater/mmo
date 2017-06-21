/*!
  \author
    Wyatt Lavigueure
  \date
    6/12/2017
  \brief
    Timer to track performance of specific functions
*/
#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <string>
#include <map>

class Timer
{
public:
  /*!
    \brief
      Starts the timer for the time called fn
    \param fn
      The name of the thng being timed, these are added together at the end
  */
  Timer(std::string fn);
  /*!
    \brief
      Stops the timer and adds the time it took to the map
  */
  ~Timer();
  /*!
    \brief
      Writes the total times for each thing being timed to the screen
  */
  static void Display();
private:
  std::chrono::high_resolution_clock::time_point t;
  std::string fn;
  static std::map<std::string, double> times;
};
#endif