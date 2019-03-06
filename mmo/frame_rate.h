/*!
  \author
    Wyatt Lavigueure
  \date
    5/6/2017
  \brief
    Keeps track time per frame. This is where time scale should be applied 
*/
#ifndef FRAM_H
#define FRAM_H
#include <chrono>

/*!
  \brief
    Does timing.
*/
class FrameRate
{
public:
	/*!
	  \brief
	    Initalizes the frame rate object and sets the timer
	*/
	FrameRate();
	/*!
	  \brief
	    Resets the timer and gives how long its been since the last GetTime call
	  \return
	    Time in seconds since the last call
	*/
	double GetTime();
	/*!
	  \brief
	    Gets the time that was returned from the last GetTime call. does not reset timer
	  \return
	    Time in seconds since the last call
	*/
	double GetPrevTime();
  /*!
    \brief
      Gets the time since the frame rate object was created.
    \return
      Time since the frame rate object was created
  */
  double GetTotalTime();
private:
	//the previous time, for GetPrevTime
	double last_time;
  //the time at the creation of the frame rate
  std::chrono::system_clock::time_point start;
  //the time at the last GetTime call
	std::chrono::system_clock::time_point prev;
};

#endif