#ifndef CLIENT_RTSP_H
#define CLIENT_RTSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


#define BUFFER_SIZE 2048
#define RTSP_VER "RTSP/1.0"

typedef enum{
    RTSP_STATE_INIT = 0, 
    RTSP_STATE_READY, 
    RTSP_STATE_PLAYING
} RTSP_State;

typedef struct
{
    int cmd_sock;
    char server_ip[64];
    int server_port;
    char url[256];

    int client_rtp_port;

    int cseq;
    char session_id[32];
    RTSP_State state;
} RTSP_Client;

// Helper functions
int get_header_value(const char* buffer, const char* header_name, char* value);
int validate_response(RTSP_Client* client, const char* resp_buf, const char* method_name);

// rtsp_send_method
void rtsp_send_options(RTSP_Client* client);
void rtsp_send_setup(RTSP_Client* client);
void rtsp_send_play(RTSP_Client* client);
void rtsp_send_pause(RTSP_Client* client);
void rtsp_send_teardown(RTSP_Client* client);
#endif
