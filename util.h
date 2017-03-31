#pragma once

#include <stdint.h>
#include <string>
#include <netinet/in.h>

namespace detail
{
// simple wrapper for fprintf function
void perror(const char* fmt, ...);

int createRawSocketOrDie();

// convert from ip address to sockaddr_in.sin_addr, v4 only
bool fromIP(const char* ip, struct sockaddr_in * addr);

bool fromHostName(const char* hostname, struct sockaddr_in* addr);

// connect to remote server, fail will exit the program
void connectOrDie(int sockfd, const struct sockaddr* dest_addr);

void connectOrDie(int sockfd, const struct sockaddr_in* dest_addr);

// only support value of type int
void setsockoptOrDie(int sockfd, int level, int opt_name, int opt_val);

struct timeval gettimeofdayOrDie();

// calculate check sum
uint16_t checksum(const uint16_t* buf, int len);

uint16_t checksum(const void* buf, int len);

std::string toIp(const struct sockaddr_in* addr);

// calculate time difference, unit is ms
double timeDifference(const struct timeval& begin, const struct timeval& end);

}