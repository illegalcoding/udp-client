#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#define DEFAULT_BUFLEN 4096

int do_exit = 0;
void sender(int send_result, int client_socket , char* sendbuf) {
  while(!do_exit) {
    std::cin.getline(sendbuf, DEFAULT_BUFLEN);
    send_result = send(client_socket, sendbuf, strlen(sendbuf), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return;
}
void listener(int return_value, int server_socket, char* recvbuf, int recvbuflen, char* clientaddr) {
  while(!do_exit) {
    try {
      return_value = recv(server_socket, recvbuf, recvbuflen, 0);
      if(return_value == -1) {
	std::cout << std::endl << "Connection lost" << std::endl;
	do_exit = 1;
      }
      if (return_value > 0) {
	for (int i = 0; i < DEFAULT_BUFLEN; i++) {
	  if(recvbuf[i] == '\0') {
	    memset(recvbuf, 0, DEFAULT_BUFLEN);
	    break;
	  } else {
	    std::cout << recvbuf[i];
	  }
	}
      }
    } catch (const std::exception& e) {

    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return;
}
void usage() {
  fprintf(stderr, "usage:\tudp-client address port\n");
}
void sighandler(int signum) {
  fprintf(stderr, "Signal caught, please strike ENTER to exit.");
  do_exit = 1;
}
int main(int argc, char** argv) {
  if (argc < 3) {
    usage();
    return 1;
  }
  signal(SIGINT, sighandler);
  int return_value = 0;
  struct addrinfo *result = NULL, *ptr = NULL, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  return_value = getaddrinfo(argv[1], argv[2], &hints, &result);
  
  if (return_value != 0) {
    std::cout << "getaddrinfo failed: " << return_value << " STR: " << gai_strerror(return_value) << std::endl;
    return 1;
  }
  int connect_socket;
  ptr = result;
  connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  return_value = connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
  struct sockaddr_in peer_addr;
  socklen_t addrlen = sizeof(peer_addr);
  return_value = getpeername(connect_socket, (struct sockaddr*)&peer_addr, &addrlen);
  char addrbuf[15];
  memset(addrbuf,0,sizeof(addrbuf));
  std::cout << "Connected to: " << inet_ntop(peer_addr.sin_family, &peer_addr.sin_addr, addrbuf, 15) << std::endl;
  freeaddrinfo(result);
  char sendbuf[DEFAULT_BUFLEN];
  char recvbuf[DEFAULT_BUFLEN];
  memset(&recvbuf, 0, DEFAULT_BUFLEN);
  int send_result;
  memset(&sendbuf, 0, DEFAULT_BUFLEN);
  std::thread t_listener(&listener, return_value, connect_socket, recvbuf, DEFAULT_BUFLEN, addrbuf);
  std::thread t_sender(&sender, send_result, connect_socket, sendbuf);
  t_listener.join();
  t_sender.join();
  close(connect_socket);
  return 0;
}
