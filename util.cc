#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

// warning! strerror is not thread safe
void detail::perror(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  exit(-1);
}

int detail::createRawSocketOrDie()
{
  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if(sockfd == -1)
  {
    perror("cannot create icmp socket file descriptor! the reason is %s\n", strerror(errno));
  }
  return sockfd;
}

bool detail::fromIP(const char *ip, struct sockaddr_in *addr)
{
  ::bzero(addr, sizeof(struct sockaddr_in));
  addr->sin_family = AF_INET;

  return ::inet_pton(AF_INET, ip, &(addr->sin_addr)) == 1;
}


bool detail::fromHostName(const char *hostname, struct sockaddr_in *addr)
{
  hostent* host = ::gethostbyname(hostname);
  if(host == nullptr)
    return false;
  memcpy(&(addr->sin_addr), host->h_addr, sizeof(uint32_t));
  return true;
}

void detail::connectOrDie(int sockfd, const struct sockaddr *dest_addr)
{
  if(-1 == ::connect(sockfd, dest_addr, sizeof(struct sockaddr_in)))
  {
    perror("connect to remote server error! %s\n", strerror(errno));
  }
}

void detail::connectOrDie(int sockfd, const struct sockaddr_in *dest_addr)
{
  connectOrDie(sockfd, (const struct sockaddr*)dest_addr);
}

void detail::setsockoptOrDie(int sockfd, int level, int opt_name, int opt_val)
{
  int value = opt_val;
  socklen_t len = static_cast<socklen_t>(value);
  if(-1 == setsockopt(sockfd, level, opt_name, &value, len))
  {
    perror("setsockopt error! %s\n", strerror(errno));
  }
}

struct timeval detail::gettimeofdayOrDie() {
  struct timeval time;
  if(::gettimeofday(&time, nullptr) == -1)
  {
    perror("gettimeofday error!%s\n", strerror(errno));
  }
  return time;
}

uint16_t detail::checksum(const uint16_t *buf, int len)
{
  long sum = 0;
  while(len > 1)
  {
    sum += *buf;
    ++buf;
    len -= 2;
  }
  if(len)
  {
    sum += *(uint8_t*)buf;
  }
  while(sum >> 16)
  {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  return ~sum;
}

uint16_t detail::checksum(const void *buf, int len)
{
  return checksum((const uint16_t*)buf, len);
}

std::string detail::toIp(const struct sockaddr_in *addr)
{
  char buff[256];
  if(inet_ntop(addr->sin_family, &(addr->sin_addr), buff, static_cast<socklen_t>(sizeof(buff))) == nullptr)
  {
    fprintf(stderr, "inet_ntop error! %s\n", strerror(errno));
    return "";
  }
  return buff;
}

double detail::timeDifference(const struct timeval &begin, const struct timeval &end)
{
   return ((end.tv_sec - begin.tv_sec) * 1000.0 + (end.tv_usec - begin.tv_usec) / 1000.0);
}