#include "client_rtsp.h"

// Helper functions
int get_header_value(const char* buffer, const char* header_name, char* value)
{
    char key[64];
    snprintf(key, sizeof(key), "%s:", header_name);
    
    char* ptr = NULL;
    if ((ptr = strstr(buffer, key)) == NULL)
        return 0;
    
    ptr += strlen(key);
    while (*ptr == ' ')
        ptr++;

    int i = 0;
    while (*ptr != '\r' && *ptr != '\n' && *ptr != '\0')
    {
        value[i] = *ptr;
        i++;
        ptr++;
    }
    value[i] = '\0';
    return 1;
}
int validate_response(RTSP_Client* client, const char* resp_buf, const char* method_name)
{
    if (strstr(resp_buf, "200 OK") == NULL)
    {
        printf("!! Error: %s failed. Server response not 200 OK.\n", method_name);
        printf("   Response: %.50s...\n", resp_buf);
        return 0;
    }   

    char val[32];
    if (get_header_value(resp_buf, "CSeq", val))
    {
        int resp_cseq = atoi(val);
        if (resp_cseq != client->cseq)
        {
            printf("!! Warning: CSeq mismatch in %s! Sent: %d, Received: %d\n", 
                   method_name, client->cseq, resp_cseq);
        }
    }

    if (strlen(client->session_id) > 0 && strcmp(method_name, "OPTIONS") != 0)
    {
        char session_value[32];
        if (get_header_value(resp_buf, "Session", session_value))
        {
            if (strcmp(client->session_id, session_value) != 0)
            {
                printf("!! CRITICAL ERROR: Session ID mismatch in %s!\n", method_name);
                printf("   Client holds: %s\n", client->session_id);
                printf("   Server sent: %s\n", session_value);
                return 0;
            }
        }
        else
        {
            printf("!! Warning: Missing Session header in %s response.\n", method_name);
        }
    }
    
    return 1;
}

// rtsp_send_method
void rtsp_send_options(RTSP_Client* client)
{
    client->cseq++;

    char req_buf[BUFFER_SIZE];
    sprintf(req_buf, sizeof(req_buf),
        "OPTIONS %s %s\r\n",
        "CSeq: %d\r\n",
        "\r\n",
        client->url, RTSP_VER, client->cseq);
    send(client->cmd_sock, req_buf, strlen(req_buf), 0);
    printf(">> sending OPTIONS (CSeq: %d)...\n", client->cseq);

    char resp_buf[BUFFER_SIZE];
    int len = recv(client->cmd_sock, resp_buf, BUFFER_SIZE - 1, 0);
    if (len <= 0) return;
    resp_buf[len - 1] = '\0';

    if (validate_response(client, resp_buf, "OPTIONS"))
    {
        printf(">> OPTIONS successful.\n");
    }
}
void rtsp_send_setup(RTSP_Client* client)
{
    client->cseq++;

    char req_buf[BUFFER_SIZE];
    sprintf(req_buf, sizeof(req_buf),
        "SETUP %s %s\r\n",
        "CSeq: %d\r\n",
        "Transpot: RTP/AVP;unicast;client_port=%d-%d\r\n",
        "\r\n",
        client->url, RTSP_VER, client->cseq, client->client_rtp_port, client->client_rtp_port+1);
    send(client->cmd_sock, req_buf, strlen(req_buf), 0);
    printf(">> sending SETUP (CSeq: %d)\n", client->cseq);
    
    char resp_buf[BUFFER_SIZE];
    int len = recv(client->cmd_sock, resp_buf, BUFFER_SIZE, 0);
    if (len <= 0) return;
    resp_buf[len] = '\0';

    if (validate_response(client, resp_buf, "SETUP"))
    {
        if (get_header_value(resp_buf, "Session", client->session_id))
        {
            printf(">> SETUP successfull. Session ID: %s\n", client->session_id);
            client->state = RTSP_STATE_READY;
        }
        else
        {
            printf("!! Error: Session ID missing in SETUP response.\n");    
        }
    }
}
void rtsp_send_play(RTSP_Client* client)
{
    if (client->state == RTSP_STATE_INIT)
    {
        printf("!! Error: cannot PLAY (Client is in INIT State).\n");
        return;
    }

    client->cseq++;

    char req_buf[BUFFER_SIZE];
    snprintf(req_buf, sizeof(req_buf),
        "PLAY %s %s\r\n",
        "CSeq: %d\r\n",
        "Session: %s\r\n",
        "Range: npt=0.000-\r\n",
        "\r\n",
        client->url, RTSP_VER, client->cseq, client->session_id);
    send(client->cmd_sock, req_buf, strlen(req_buf), 0);
    printf(">> Sending PLAY (CSeq: %d)...\n", client->cseq);

    char resp_buf[BUFFER_SIZE];
    int len = recv(client->cmd_sock, resp_buf, BUFFER_SIZE - 1, 0);
    if (len <= 0) return;
    resp_buf[len] = '\0';

    if (validate_response(client, resp_buf, "PLAY"))
    {
        client->state = RTSP_STATE_PLAYING;
        printf(">> PLAY successful. State changed to PLAYING.\n");
    }
}
void rtsp_send_pause(RTSP_Client* client)
{
    if (client->state != RTSP_STATE_PLAYING)
    {
        printf("!! Error: Cannot PAUSE (Client is not PLAYING).\n");
        return;
    }   

    client->cseq++;

    char req_buf[BUFFER_SIZE];
    snprintf(req_buf, sizeof(req_buf),
        "PAUSE %s %s\r\n",
        "CSeq: %d\r\n",
        "Session: %s\r\n",
        "\r\n",
        client->url, RTSP_VER, client->cseq, client->session_id);
    send(client->cmd_sock, req_buf, strlen(req_buf), 0);
    printf(">> sending PAUSE (CSeq: %d)... \n", client->cseq);
    
    char resp_buf[BUFFER_SIZE];
    int len = recv(client->cmd_sock, resp_buf, BUFFER_SIZE - 1, 0);
    if (len <= 0) return;
    resp_buf[len] = '\0';

    if (validate_response(client, resp_buf, "PAUSE"))
    {
        client->state = RTSP_STATE_READY;
        printf(">> PAUSE successful. State changed to READY.\n");
    }
}
void rtsp_send_teardown(RTSP_Client* client)
{
    if (client->state == RTSP_STATE_INIT)
    {
        printf("!! Error: cannot TEARDOWN (Client is in INIT State).\n");
        return;
    }
    
    char* req_buf[BUFFER_SIZE];
    snprintf(req_buf, sizeof(req_buf),
        "TEARDOWN %s %s\r\n",
        "CSeq: %d\r\n",
        "Session: %s\r\n",
        "\r\n",
        client->url, RTSP_VER, client->cseq, client->session_id);
    send(client->cmd_sock, req_buf, strlen(req_buf), 0);
    printf(">> Sending TEARDOWN (CSeq: %d)...\n", client->cseq);

    char resp_buf[BUFFER_SIZE];
    int len = recv(client->cmd_sock, resp_buf, BUFFER_SIZE - 1, 0);
    
    if (len > 0)
    {
        resp_buf[len] = '\0';
        validate_response(client, resp_buf, "TEARDOWN");
    }

    client->state = RTSP_STATE_INIT;
    memset(client->session_id, 0, sizeof(client->session_id));
    printf(">> TEARDOWN completed. Session cleared\n");
}

