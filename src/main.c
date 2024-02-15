#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "3490"
#define BACKLOG 5

static void client() {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *servinfo, *p;
  int sock_fd, new_fd;

  // initialize hints to 0
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // use eitehr ipv4 or 6
  hints.ai_socktype = SOCK_STREAM; // TCP connection
  hints.ai_flags = AI_PASSIVE;     // fill in IP for me
  (void)getaddrinfo(NULL, PORT, &hints, &servinfo);

  // create a socket
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("Socket error: ");
      continue;
    }

    if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock_fd);
      perror("Client connect: ");
      continue;
    }

    break;
  }

  const char *msg = "the client says hello!";
  send(sock_fd, msg, strlen(msg) + 1, 0);

  close(sock_fd);
}
static void server() {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sock_fd, new_fd;

  // initialize hints to 0
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;       // use eitehr ipv4 or 6
  hints.ai_socktype = SOCK_STREAM; // TCP connection
  hints.ai_flags = AI_PASSIVE;     // fill in IP for me
  (void)getaddrinfo(NULL, PORT, &hints, &res);

  // create a socket
  sock_fd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
  // first argument is the domain, it means Protocol Family IPv4, basically
  // creating a socket which can use Ipv4 Second argument is the type of socket,
  // in our case we want a streaming socket third argument is the protocol, 0 is
  // just the default protocol

  // bind socket to open port
  if (bind(sock_fd, res->ai_addr, res->ai_addrlen)) {
    perror("Bind error: ");
    return;
  }

  struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
  printf("Server is listening on port: %d\n", (int)ntohs(addr->sin_port));
  if (listen(sock_fd, BACKLOG)) {
    perror("Listen error: ");
    return;
  }

  // create new file descriptor
  new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &addr_size);

  // read from client
  // create a buffer to store recieved data in
  char buf[1024];
  recv(new_fd, buf, sizeof(buf), 0);

  // print unsanitised data from client
  printf("Client says:\n %s\n", buf);

  // close sockets
  close(sock_fd);
  close(new_fd);
  freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
  printf("Hello, Game!\n");

  if (!strcmp(argv[1], "client")) {
    client();
  } else {
    server();
  }
  return 0;
}
