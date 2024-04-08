#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

char buf[BUFSIZE + 1];
typedef struct {
	SOCKET client_sock;
}TCPClient;

void *tcp_recv(void * arg) {
	TCPClient *tcpclient = (TCPClient *)arg;
	tcpclient -> client_sock;
	while (1) {
			// 데이터 받기
			buf[0] = '\0';
			int retval = recv(tcpclient -> client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("client: %s\n", buf);
		}
		pthread_exit(NULL);
}

void *tcp_send(void * arg) {
	TCPClient *tcpclient = (TCPClient *)arg;
	tcpclient -> client_sock;
	
	while(1){
		// 데이터 입력
		printf("server: ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		int len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		int retval = send(tcpclient -> client_sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("\n");
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	
	pthread_t recvtcp;
	pthread_t sendtcp;
	
	while (1) {
		TCPClient tcpclient;
		// accept()
		addrlen = sizeof(clientaddr);
		tcpclient.client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (tcpclient.client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		pthread_create(&sendtcp, NULL, tcp_send, (void *)&tcpclient);
		pthread_create(&recvtcp, NULL, tcp_recv, (void *)&tcpclient);
		
		pthread_join(sendtcp, NULL);
		pthread_join(recvtcp, NULL);
		
		// 소켓 닫기
		close(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	close(listen_sock);
	return 0;
}
