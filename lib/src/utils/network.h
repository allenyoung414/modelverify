/* System-independent wrapper for network operations. */

#ifndef __NETWORK_H

#define __NETWORK_H

#include <atl/string.h>
#include <utils/boolean.h>

#ifdef __cplusplus
extern "C" {
#endif

/**** Server ****/
/** @return sock to pass to accept_connection()
 (and then to close_socket()!) */
int create_server(unsigned short port);
/** @return socket id for data exchange. Value <0 if something failed
 || (canBlock==true && no incoming connections ready) */
int accept_connection(int sock, bool canBlock);

/**** Client ****/
/** @return socket id for data exchange. */
int connect_to_server(const char *serverHost, unsigned short serverPort);

/**** Data exchange.
 send_to_socket() and read_from_socket() attempt to write/read data
 until specified amount of data is processed or error occurs.
 ****/
int send_to_socket(int sock, const void *data, int len);
int read_from_socket(int sock, void *data, int len);

bool send_String_message(int sock, String *str /*any bytes*/);
bool send_cstr_message(int sock, const char *str /*null-terminated*/);
bool send_data_message(int sock, const void *data /*any bytes or plain struct*/, int len);
/* Receive message sent by any of the 3 above functions */
String *receive_varchar_message(int sock);

/**** Non-blocking read checks. ****/
bool socket_has_incoming_data(int sock);
/**
 @param sockets
   in: array of socket ids to poll.
  out: array is filled with ids of sockets having data to read. Their relative
       order is preserved.
 @param sockets_num
   in: variable holding length of sockets array.
  out: variable contents is replaced with number of sockets having data to read.
*/
void check_incoming_data(int *sockets, int *sockets_num);

/**** Close ****/
void close_socket(int sock);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __NETWORK_H */
