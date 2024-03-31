#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512


// 각 월의 일수
int daysInMonth(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //윤년 여부
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        days[1] = 29;
    }
    return days[month - 1];
}

// 달력 출력
void ShowCalendar(char buf[]) {
	char result[BUFSIZE + 1];
    int i, j, day;
    int startingDay;
    char fyear[5], fmonth[3], fday[5];

    strncpy(fyear, buf, 4);
    fyear[4] = '\0';
    strncpy(fmonth, buf+5, 2);
    fmonth[2] = '\0';

	buf[0] = '\0';

    int year = atoi(fyear);
    int month = atoi(fmonth);

    // 시작하는 요일 구하기
    startingDay = 1; //1900년 1월 1일 시작
    for (i = 1900; i < year; i++) {  //년도
        for (j = 1; j <= 12; j++) {
            startingDay = (startingDay + daysInMonth(i, j)) % 7;
        }
    }
    for (j = 1; j < month; j++) { //월
        startingDay = (startingDay + daysInMonth(year, j)) % 7;
    }

    // 달력 출력
    strcpy(result, "SUN MON THU WED THU FRI SAT\n");
    for (i = 0; i < startingDay; i++) {
        strcat(result, "    ");
    }
    for (day = 1; day <= daysInMonth(year, month); day++) {
        sprintf(fday, "%3d", day); fday[3] = ' '; fday[4] = '\0';
        strcat(result, fday);
        
        startingDay++;
        if (startingDay % 7 == 0)
            strcat(result, "\n");
    }
	strcpy(buf, result);
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
	char buf[BUFSIZE + 1];

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			//캘린더
			ShowCalendar(buf);
			buf[(int)strlen(buf)] = '\0';
			printf("%s\n", buf);

			// 데이터 보내기
			retval = send(client_sock, buf, 200, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		close(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	close(listen_sock);
	return 0;
}
