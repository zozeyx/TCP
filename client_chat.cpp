#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

char buf[BUFSIZE + 1];
typedef struct {
	SOCKET server_sock;
}TCPServer;

void *tcp_recv(void * arg) {
	TCPServer *tcpserver = (TCPServer *)arg;
	SOCKET sock = tcpserver -> server_sock;
	
	while (1) {
			// 데이터 받기
			buf[0] = '\0';
			int retval = recv(sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("server: %s\n", buf);
		}
		pthread_exit(NULL);
}

void *tcp_send(void * arg) {
	TCPServer *tcpserver = (TCPServer *)arg;
	SOCKET sock = tcpserver -> server_sock;
	
	while(1){
		// 데이터 입력
		printf("client: ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		int len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		int retval = send(sock , buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	int len;
	
	while (1) {
	TCPServer tcpserver;
	
	pthread_t recvtcp;
	pthread_t sendtcp;
	
	pthread_create(&sendtcp, NULL, tcp_send, (void *)&tcpserver);
	pthread_create(&recvtcp, NULL, tcp_recv, (void *)&tcpserver);
		
	pthread_join(sendtcp, NULL);
	pthread_join(recvtcp, NULL);
	}
	
	// 소켓 닫기
	close(sock);
	return 0;
}	
