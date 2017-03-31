#include "util.h"
#include "ping.h"

#include <netinet/in.h>
#include <libgen.h> // for basename function

using namespace detail;

int main(int argc, char* argv[])
{

  if(argc != 2)
  {
    perror("Usage: %s destination\n", ::basename(argv[0]));
  }

  struct sockaddr_in addr;
  if(!fromIP(argv[1], &addr) && !fromHostName(argv[1], &addr))
  {
    perror("unknown host %s\n", argv[1]);
  }

  int sockfd = createRawSocketOrDie();

  // not tcp connect! use connect function so I can use recv and send function directly
  connectOrDie(sockfd, &addr);

  std::string host = toIp(&addr);

  zy::ping::getInstance(sockfd, host)->run();

  return 0;
}