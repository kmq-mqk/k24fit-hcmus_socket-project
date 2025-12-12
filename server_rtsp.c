#include "server_rtsp.h"

// Helper functions
void send_rtsp_error(RTSP_ClientState* client, int cseq, int status_code, const char* msg)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
        "%s %d %s\r\n",
        "CSeq: %d\r\n",
        "\r\n",
        RTSP_VER, msg, cseq);
    send(client->cmd_sock, buffer, strlen(buffer), 0);
    printf(">> Error Sent: %d %s (CSeq: %d)\n", status_code, msg, cseq);
}

int get_header_value(const char* req_buf, const char* head_name, char* head_value)
{
    char* ptr = strstr(req_buf, head_name);
    if (ptr == NULL) return -1;
    
    char key[64]; 
    snprintf(key, sizeof(key), "%s:", head_name);
    ptr += strlen(key);

    while (*ptr == ' ')
        ptr++;

    int i = 0;
    while (*ptr != '\r' && *ptr != '\n' && *ptr != '\0')
    {
        head_value[i] = *ptr;
        ptr++;
        i++;
    }
    head_value[i] = '\0';
    return 1;
}

void generate_session_id(char *dest, int len)
{
    static int seeded = 0;
    if (!seeded) 
    { 
        srand(time(NULL)); 
        seeded = 1; 
    }
    int id = rand() % 900000 + 100000;
    snprintf(dest, len, "%d", id);
} 

int create_udp_socket(int* port_out)
{
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = 0;

    if (bind(sock_fd, (struct sockaddr*) &address, sizeof(address)) < 0)
    {
        close(sock_fd);
        return -1;
    }

    socklen_t len = sizeof(address);
    if (getsockname(sock_fd, (struct sockaddr*)&address, &len) == -1)
    {
        close(sock_fd);
        return -1;
    }

    *port_out = ntohs(address.sin_port);
    return sock_fd;
}


// handle_method
void handle_options(RTSP_ClientState* client, int cseq, char* req_buf)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
            "%s 200 OK\r\n",
            "CSeq: %d\r\n",
            "Public: OPTIONS, SETUP, PLAY, PAUSE, TEARDOWN\r\n",
            "\r\n",
            RTSP_VER, cseq);
    send(client->cmd_sock, buffer, strlen(buffer), 0);
    printf(">> OK: OPTIONS (CSeq: %d)\n", cseq);
}

void handle_setup(RTSP_ClientState* client, int cseq, char* req_buf)
{
    if (client->state != RTSP_STATE_INIT)
    {
        send_rtsp_error(client, cseq, 455, "Method Not Valid in This State");
        return;
    }

    char transport_val[32];
    if (!get_header_value(req_buf, "Transport", transport_val))
    {
        send_rtsp_error(client, cseq, 400, "Bad Request (Missing Transport)");
        return;
    }

    // Check-Take the client_port
    int c_port = 0;
    char* ptr = strstr(transport_val, "client_port=");
    sscanf(ptr, "client_port=%d", &c_port);

    if (c_port == 0)
    {
        send_rtsp_error(client, cseq, 400, "Bad Request (Invalid Client Port)");
        return;
    }
    client->client_rtp_port = c_port;

    // Create UDP socket
    if (client->data_sock == 0)
    {
        client->data_sock = create_udp_socket(&client->server_rtp_port);
        if (client->data_sock < 0) 
        {
            send_rtsp_error(client, cseq, 500, "Internal Server Error (UDP Bind Failed)");
            return;
        }
    }

    // One file
    if (client->fp_video == NULL)
    {
        client->fp_video = fopen("movie.jpeg", "rb");

        if (client->fp_video == NULL)
        {
            perror("Open file failed");
            send_rtsp_error(client, cseq, 404, "File Not Found");
            return;
        }
    }

    generate_session_id(client->session_id, sizeof(client->session_id));
    client->state = RTSP_STATE_READY;
    client->frame_idx = 0;

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
            "%s 200 OK\r\n",
            "CSeq: %d\r\n",
            "Session: %s\r\n",
            "\r\n"
            RTSP_VER, cseq, client->session_id);
    printf(">> OK: SETUP Done. Session: %s. Client Port: %d <-> Server Port: %d\n", 
           client->session_id, client->client_rtp_port, client->server_rtp_port);
}

void handle_play(RTSP_ClientState* client, int cseq, char* req_buf)
{
    if (client->state == RTSP_STATE_INIT)
    {
        send_rtsp_error(client, cseq, 455, "Method Not Valid in This State");
        return;
    }

    char req_session[32];
    if (!get_header_value(req_buf, "Session", req_session)
         || strcmp(client->session_id, req_session) != 0)
    {
        send_rtsp_error(client, cseq, 454, "Session Not Found");
        return;         
    }
    
    client->state = RTSP_STATE_PLAYING;

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
            "%s 200 OK\r\n",
            "CSeq: %d\r\n",
            "Session: %s\r\n",
            "\r\n",
            RTSP_VER, cseq, req_session);
    send(client->cmd_sock, buffer, strlen(buffer), 0);
    printf(">> OK: PLAYING Session %s\n", client->session_id);
}

void handle_pause(RTSP_ClientState* client, int cseq, char* req_buf)
{
    if (client->state != RTSP_STATE_PLAYING)
    {
        send_rtsp_error(client, cseq, 455, "Method Not Valid in This State");
        return;
    }

    char req_session[32];
    if (!get_header_value(req_buf, "Session", req_session)
         || strcmp(client->session_id, req_session) != 0)
    {
        send_rtsp_error(client, cseq, 454, "Session Not Found");
        return;         
    }
    
    client->state = RTSP_STATE_READY;

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
            "%s 200 OK\r\n",
            "CSeq: %d\r\n",
            "Session: %s\r\n",
            "\r\n",
            RTSP_VER, cseq, req_session);
    send(client->cmd_sock, buffer, strlen(buffer), 0);
    printf(">> OK: PAUSED Session %s\n", client->session_id);
}

void handle_teardown(RTSP_ClientState* client, int cseq, char* req_buf)
{
    if (client->state == RTSP_STATE_INIT)
    {
        send_rtsp_error(client, cseq, 455, "Method Not Valid in This State");
        return;
    }

    char req_session[64];
    if (!get_header_value(req_buf, "Session", req_session)
         || strcmp(client->session_id, req_session) != 0)
    {
        send_rtsp_error(client, cseq, 454, "Session Not Found");
        return;         
    }

    client->state = RTSP_STATE_READY;

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
        "%s 200 OK\r\n",
        "CSeq: %d\r\n",
        "Session: %s\r\n",
        "\r\n",
        RTSP_VER, cseq, req_session);
    send(client->cmd_sock, buffer, strlen(buffer), 0);

    if (client->fp_video)
    {
        close(client->fp_video);
        client->fp_video = NULL;
    }
    if (client->data_sock > 0)
    {
        close(client->data_sock);
        client->data_sock = 0;
    }

    memset(client->session_id, 0, sizeof(client->session_id));
    printf(">> OK: TEARDOWN. Session Destroyed.\n");
}
