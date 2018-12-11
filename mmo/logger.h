/*!
  \author
    Wyatt Lavigueure
  \date
    6/19/2018
  \brief
    Logs things to file, there are two different types of messages, notifications and
    warnings. Notifications go to generated/logger_notifications.log and warnings go to
    generated/logger_warnings.log. use the LOG and LOGW macros, they will add the file,
    function and line number, also its easy to turn it into an empty macro for production
    so that it will take no performance. The LOGW will log to the warning log and the notification log
    in the notification log it will be marked Warning.

    example usages:

    LOG("one thing");
    LOG(aVariable);
    LOG("more " << "than " << "one " << "thing " << "and " << a_variable << "chained " << "together by << operator");
    LOGW("Something should not have happened so use a warning");
*/
#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

/*!
  \brief
    Turns a buffer into hex
*/
std::string ToHexString(char *buffer, int length);

/*!
  \brief
    Macro to add usefull information to the log
*/
#define LOG(x) Logger::Notification() << "File " << __FILE__ << " Function " << __FUNCTION__ << " Line " << __LINE__ << "\n" << x


/*!
  \brief
    Same as LOG but for warnings
*/
#define LOGW(x) LOG("*** Warning ***\n" << x); Logger::Warning() << "File " << __FILE__ << " Function " << __FUNCTION__ << " Line " << __LINE__ << "\n" << x


/*!
  \brief
    Read the file description.
*/
class Logger
{
public:
  /*!
    \brief
      opens the log file and returns it by value. Also adds the current time to the log.
    \return
      the file stream
  */
  static std::fstream Notification();
  /*!
    \brief
      opens the log file and returns it by value. Also adds the current time to the log.
    \return
      the file stream
  */
  static std::fstream Warning();
};

#endif