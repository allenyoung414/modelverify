/********************************************************************
 Wrapper for network operations. Implementations fow Windows and Unix/Linux.
 Currently uses TCP transport directly, without other intermediate libraries.

 ********************************************************************
 TODO: Read buffering.
 ********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <utils/network.h>


#if defined(WIN32)

#include <winsock.h>
/*#include <winsock2.h>*/
#include <stdio.h>

static bool wsa_need_clean = false;
static int active_sockets = 0;

static void format_error(char *buff, int len)
{
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		WSAGetLastError(),
		0,
		buff,
		len - 1,
		NULL);

	/* Remove trailing CR/LF */
	len = strlen(buff);
	if (len >= 2 && buff[len-1] == '\n' && buff[len-2] == '\r')
		buff[len-2] = 0;
}

static int initWinSock()
{
	if (!wsa_need_clean) {
		WORD wVersionRequested = MAKEWORD(2,2);
		WSADATA wsaData;

		if (WSAStartup(wVersionRequested, &wsaData))
			return -1;

		if (wsaData.wVersion != wVersionRequested) {
			fprintf(stderr, "Invalid WinSock version\n");
			WSACleanup();
			return -1;
		}

		wsa_need_clean = true;
	}

	return 0;
}

int create_server(unsigned short port)
{
SOCKET sock;
BOOL on = TRUE;
SOCKADDR_IN saServer;
char errmsg[512];

	if (initWinSock() != 0)
		return -1;

	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sock == INVALID_SOCKET)	{
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot create server socket: %s\n", errmsg);
		return -1;
    }

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) ) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "setsockopt() failed: %s\n", errmsg);
		/* Try to continue */
	}

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	saServer.sin_port = htons(port);

	if ( bind(sock, (LPSOCKADDR)&saServer, sizeof(saServer)) == SOCKET_ERROR ) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot bind to port %i: %s\n", (int)port, errmsg);
		return -1;
	}

	if ( listen(sock, SOMAXCONN) == SOCKET_ERROR ) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot listen(): %s\n", errmsg);
		return -1;
	}

	active_sockets++;
	return (int)sock;
}

int accept_connection(int sock, bool canBlock)
{
SOCKET conn;
char errmsg[512];

	if (!canBlock && !socket_has_incoming_data(sock))
		return -1;

	conn = accept(sock, NULL, NULL);
	if (conn == INVALID_SOCKET) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot accept connection: %s\n", errmsg);
		return -1;
	}

	active_sockets++;
	return (int)conn;
}

int connect_to_server(const char *serverHost, unsigned short serverPort)
{
SOCKET		sck;
IN_ADDR		iaHost;
LPHOSTENT	lpHostEntry;
SOCKADDR_IN saServer;
char errmsg[512];

	if (initWinSock() != 0)
		return -1;

	iaHost.s_addr = inet_addr(serverHost);
	if (iaHost.s_addr == INADDR_NONE) {
		/* Wasn't an IP address string, assume it is a name */
		lpHostEntry = gethostbyname(serverHost);
	} else {
		/* It was a valid IP address string */
		lpHostEntry = gethostbyaddr((const char *)&iaHost, 
						sizeof(struct in_addr), AF_INET);
	}
	if (lpHostEntry == NULL) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Host %s not found: %s\n", serverHost, errmsg);
		return -1;
	}

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(serverPort);

	sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sck == INVALID_SOCKET) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot create socket: %s\n", errmsg);
		return -1;
	}

	if (connect( sck, (LPSOCKADDR)&saServer, sizeof(saServer) ) == SOCKET_ERROR) {
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "Cannot connect to %s:%d : %s\n", serverHost, (int)serverPort, errmsg);
		return -1;
	}

	active_sockets++;
	return (int)sck;
}

int send_to_socket(int sock, const void *data, int len)
{
int l, total = 0;
	while (total < len) {
		l = send((SOCKET)sock, ((const char*)data) + total, len - total, 0);
		if (l < 0) {
			char errmsg[512];
			format_error(errmsg, sizeof(errmsg));
			fprintf(stderr, "Socket send failed: %s\n", errmsg);
		}
		if (l <= 0)	return total;
		total += l;
	}
	return total;
}

int read_from_socket(int sock, void *data, int len)
{
int l, total = 0;
	while (total < len) {
		l = recv((SOCKET)sock, ((char*)data) + total, len - total, 0);
		if (l < 0) {
			char errmsg[512];
			format_error(errmsg, sizeof(errmsg));
			fprintf(stderr, "Socket recv failed: %s\n", errmsg);
		}
		if (l <= 0)	return total;
		total += l;
	}
	return total;
}

/* According to MSDN, WSAPoll() requires too high version of Windows. Using select(). */
bool socket_has_incoming_data(int sock)
{
struct timeval timeout = { 0, 0 };
fd_set readfds;

	FD_ZERO( &readfds );
	FD_SET( (SOCKET)sock, &readfds );
	
	if ( select(0, &readfds, NULL, NULL, &timeout) < 0) {
		/* printf( "select() failed: %i\n", WSAGetLastError() ); */
		return false;
	}
	return FD_ISSET( sock, &readfds );
}

void check_incoming_data(int *sockets, int *sockets_num)
{
struct timeval timeout = { 0, 0 };
fd_set readfds;
int ret, i, count = 0;
	
	if (*sockets_num == 0)	return;

	FD_ZERO( &readfds );
	for (i = 0; i < *sockets_num; i++) {
		FD_SET( (SOCKET)sockets[i], &readfds );
	}

	ret = select(0, &readfds, NULL, NULL, &timeout);

	if(ret < 0) {
		char errmsg[512];
		format_error(errmsg, sizeof(errmsg));
		fprintf(stderr, "select() failed: %s\n", errmsg);
	}

	if (ret > 0) {
		for (i = 0; i < *sockets_num; i++) {
			if (FD_ISSET((SOCKET)sockets[i], &readfds)) {
				if (count < i)	sockets[count] = sockets[i];
				count++;
			}
		}
	}

	*sockets_num = count;
	return;
}

void close_socket(int sock)
{
	if ((SOCKET)sock == INVALID_SOCKET)	return;

	/* shutdown( (SOCKET)sock, SD_BOTH ); */
	closesocket((SOCKET)sock);
	active_sockets--;
	if (active_sockets==0 && wsa_need_clean) {
		/* printf("Calling WSACleanup()\n"); */
		WSACleanup();
		wsa_need_clean = false;
	}
}


#elif defined (unix)


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>

int create_server(unsigned short port)
{
int sock;
int on = 1;
struct sockaddr_in s_in;

	sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)	{
		perror("socket");
		return -1;
    }

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) ) {
		perror("setsockopt");
		/* Try to continue */
	}

	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(port);
	memset(&s_in.sin_addr, 0, sizeof(s_in.sin_addr));
	if ( bind(sock, (struct sockaddr *)&s_in, sizeof(s_in)) < 0)	{
		perror("bind");
		return -1;
    }

    if (listen(sock, SOMAXCONN) < 0)	{
		perror("listen");
		return -1;
    }
	return sock;
}

int accept_connection(int sock, bool canBlock)
{
	if (!canBlock && !socket_has_incoming_data(sock))
		return -1;

	int conn = accept(sock, NULL, NULL);
	if (conn < 0) {
		perror("accept");
		return -1;
	}
	return conn;
}

int connect_to_server(const char *serverHost, unsigned short serverPort)
{
struct hostent *hp;
struct sockaddr_in s_in;
int sock;

    s_in.sin_family=AF_INET;
    s_in.sin_port=htons(serverPort);

    if ( (hp=gethostbyname(serverHost)) == NULL )	{
		perror("gethostbyname");
    	return -1;
    }
    memcpy(&s_in.sin_addr, hp->h_addr, hp->h_length);

    if ( (sock = socket(AF_INET,SOCK_STREAM,0)) < 0)	{
		perror("socket");
		return -1;
    }

    if ( connect(sock, (struct sockaddr*)&s_in, sizeof(s_in)) == -1) {
		perror("connect");
		return -1;
    }

    return  sock;
}

int send_to_socket(int sock, const void *data, int len)
{
int l, total = 0;
	while (total < len) {
		l = send(sock, ((const char*)data) + total, len - total, 0);
		if (l < 0)
			perror("send_to_socket");
		if (l <= 0)	return total;
		total += l;
	}
	return total;
}

int read_from_socket(int sock, void *data, int len)
{
int l, total = 0;
	while (total < len) {
		l = recv(sock, ((char*)data) + total, len - total, 0);
		if (l < 0)
			perror("read_from_socket");
		if (l <= 0)	return total;
		total += l;
	}
	return total;
}

/* select() code used for Windows implementation will work, too
   (without casts to type SOCKET), but we may use poll() instead. */
bool socket_has_incoming_data(int sock)
{
struct pollfd read_poll = { sock, POLLIN, 0 };

	if ( poll(&read_poll, 1, 0) < 0) {
		/* perror("poll"); */
		return false;
	}
	return read_poll.revents & POLLIN;
}

void check_incoming_data(int *sockets, int *sockets_num)
{
struct pollfd *read_poll;
struct pollfd small_buffer[64];
int ret, i, count = 0;

	if (*sockets_num == 0)	return;

	if (*sockets_num <= 64)
		read_poll = small_buffer;
	else {
		read_poll = malloc( (*sockets_num) * sizeof(struct pollfd) );
		if(read_poll == NULL) {
			fprintf(stderr, "check_incoming_data(): out of memory\n");
			exit(1);
		}
	}

	for (i = 0; i < *sockets_num; i++) {
		read_poll[i].fd = sockets[i];
		read_poll[i].events = POLLIN;
		read_poll[i].revents = 0;
	}

	ret = poll( read_poll, *sockets_num, 0);
	if (ret < 0)
		perror("poll");

	if (ret > 0) {
		for (i = 0; i < *sockets_num; i++) {
			if (read_poll[i].revents & POLLIN) {
				if (count < i)	sockets[count] = sockets[i];
				count++;
			}
		}
	}

	*sockets_num = count;
	if (read_poll != small_buffer)
		free(read_poll);
	return;
}

void close_socket(int sock)
{
    close(sock);
}


#else

#error Unknown OS

#endif


bool send_String_message(int sock, String *str)
{
bool res;

	res = send_data_message( sock, toCharArray_String(r(str)), length_String(r(str)) );

	destroy(str);
	return res;
}

bool send_cstr_message(int sock, const char *str)
{
	return send_data_message( sock, str, strlen(str) );
}

bool send_data_message(int sock, const void *data, int len)
{
	if ( send_to_socket(sock, &len, sizeof(int)) != sizeof(int))
		return false;
	if ( send_to_socket(sock, data, len) != len)
		return false;
	return true;
}

String *receive_varchar_message(int sock)
{
String *res;
char *ptr;
int len, rlen;

	rlen = read_from_socket( sock, &len, sizeof(int) );
	if (rlen != sizeof(int))
		return NULL;

	res = create( &type_String, NULL, len );
	ptr = (char*)toCharArray_String(r(res));

	rlen = read_from_socket( sock, ptr, len );
	if (rlen != len) {
		destroy(res);
		return NULL;
	}

	ptr[ len ] = 0;	// Make string null-terminated

	return res;
}

