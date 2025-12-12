#ifndef SERVER_RTSP_H
#define SERVER_RTSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 554
#define RTSP_VER "RTSP/1.0"
#define BUFFER_SIZE 1024

typedef enum
{
    RTSP_STATE_INIT = 0,
    RTSP_STATE_READY, 
    RTSP_STATE_PLAYING
} RTSP_State;

typedef struct
{
    int cmd_sock;
    char client_port[64];

    int data_sock;
    int client_rtp_port;
    int server_rtp_port;
    
    int cseq;
    char* session_id;
    RTSP_State state;

    FILE* fp_video;
    int frame_idx;
} RTSP_ClientState;

int get_header_value(const char* buffer, const char* header_name, char* header_value);
void generation_session_id(char* dest, int len);
int create_udp_socket(int* port_out);
void send_rtsp_error(RTSP_ClientState* client, int cseq, int status_code, const char* msg);
// void parse_request_line(const char* buffer, char* method, char* url);
// int get_cseq(const char* buffer);

void handle_options(RTSP_ClientState* client, int cseq, char* req_buf);
void handle_setup(RTSP_ClientState* client, int cseq, char* req_buf);
void handle_play(RTSP_ClientState* client, int cseq, char* req_buf);
void handle_pause(RTSP_ClientState* client, int cseq, char* req_buf);
void handle_teardown(RTSP_ClientState* client, int cseq, char* req_buf);
#endif