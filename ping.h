#pragma once

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>

namespace zy
{

// because of signal handler, we must use static variable
// so just define ping as singleton
class ping : boost::noncopyable
{
 public:

  static  ping* getInstance(int sockfd, const std::string& host);

  void run();

 private:

  static void sigint_handler(int signal);

  void sendInThread();

  void recvInThread();

  ping(int sockfd, const std::string& host);

  int sockfd_;

  static int seq_no_;
  static int id_;
  static std::string host_;
  static struct timeval begin_;
  static int send_num_;
  static int recv_num_;
  // store rtts_
  static std::vector<double> rtts_;
};

}