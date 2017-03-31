#include "ping.h"
#include "util.h"
#include <netinet/ip_icmp.h>

#include <algorithm>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <sys/time.h>

using namespace zy;
using namespace detail;

int ping::seq_no_ = 0;
int ping::id_ = ::getpid();
std::string ping::host_ = "";
struct timeval ping::begin_;
int ping::send_num_ = 0;
int ping::recv_num_ = 0;
std::vector<double> ping::rtts_;


// even run twice with different variables, instance will initialize once
ping *ping::getInstance(int sockfd, const std::string &host) {
  static ping instance(sockfd, host);
  return &instance;
}

ping::ping(int sockfd, const std::string &host)
  : sockfd_(sockfd)
{
  host_ = host;
  begin_ = gettimeofdayOrDie();
  // register signal handler
  signal(SIGINT, sigint_handler);
}

// todo : mdev
void ping::sigint_handler(int signal)
{
  if(send_num_ != 0)
  {
    printf("\n--- %s ping statistics ---\n", host_.c_str());
    int elpased = static_cast<int>((send_num_ - recv_num_) * 100.0 / send_num_);
    struct timeval now = gettimeofdayOrDie();
    int diff = static_cast<int>(timeDifference(begin_, now));
    printf("%d packets transmitted, %d received, %d%% packet loss, time %d ms\n", send_num_,
           recv_num_, elpased, diff);

    if(!rtts_.empty())
    {
      std::sort(rtts_.begin(), rtts_.end());
      double min_rtt = rtts_.front();
      double max_rtt = rtts_.back();
      double sum = std::accumulate(rtts_.begin(), rtts_.end(), 0);
      double avg_rtt = sum / static_cast<double>(rtts_.size());

      printf("rtt min/avg/max = %.3lf/%.3lf/%.3lf ms\n", min_rtt, avg_rtt, max_rtt);
    }
  }
  exit(0);
}

void ping::sendInThread()
{
  // 8 bytes for icmp header, 8 bytes for time, 48 bytes for icmp data
  // add ip header with 20 bytes, so total send length is 84 bytes
  char send_buff[48 + 8 + 8];
  ::bzero(send_buff, sizeof(send_buff));
  printf("PING %s 56(84) bytes of data.\n", host_.c_str());
  while(true)
  {
    struct icmp * icmp_packet = (struct icmp*) send_buff;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_seq = seq_no_ ++;
    icmp_packet->icmp_id = id_;

    struct timeval * time = (struct timeval*) icmp_packet->icmp_data;
    if(::gettimeofday(time, nullptr) == -1)
    {
      perror("gettimeofday error! %s", strerror(errno));
    }

    icmp_packet->icmp_cksum = checksum(send_buff, sizeof(send_buff));

    int n = write(sockfd_, send_buff, sizeof(send_buff));
    if(n <= 0)
    {
      perror("send error! %s\n", strerror(errno));
    }
    ++ send_num_;
    sleep(1);
  }
}

//recv raw ip data
void ping::recvInThread()
{
  char recv_buff[1024];
  while(true)
  {
    int n = read(sockfd_, recv_buff, sizeof(recv_buff));
    if(n <= 0)
    {
      fprintf(stderr, "read error!\n");
      exit(-1);
    }
    struct ip * ip_packet = (struct ip*) recv_buff;

    // ip header 的长度以 4字节为基本单位
    int ip_header_length = static_cast<int>(ip_packet->ip_hl) << 2;

    if((ip_header_length + 8) > n)
    {
      fprintf(stderr, "recv icmp data is not entire!\n");
      exit(-1);
    }

    ++ recv_num_;

    struct icmp* icmp_packet = (struct icmp*)(recv_buff + ip_header_length);

    {
      // 计算收到的数据的校验码，包括校验位, 若计算结果为0, 则校验通过
      int check_sum = checksum(&recv_buff, n);
      if(check_sum != 0)
      {
        fprintf(stderr, "recv data check sum error! which is %d\n", check_sum);
      }
    }

    // check icmp type and id
    if(icmp_packet->icmp_type == ICMP_ECHOREPLY && icmp_packet->icmp_id == id_)
    {
      struct timeval * send_time = (struct timeval*)(icmp_packet->icmp_data);
      auto now = gettimeofdayOrDie();
      double rtt = timeDifference(*send_time, now);
      rtts_.push_back(rtt);
      // 20 for ip header, 8 for icmp header
      int data_len = n - 20 - 8;
      printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms\n", data_len, host_.c_str(), seq_no_, ip_packet->ip_ttl, rtt);
    }
  }
}

void ping::run()
{
  std::thread send_thread(&ping::sendInThread, this);
  std::thread recv_thread(&ping::recvInThread, this);
  send_thread.join();
  recv_thread.join();
}
