#include <uxr/client/transport.h>

#include <rmw_microxrcedds_c/config.h>

#include "main.h"
#include "cmsis_os.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// --- LWIP ---
#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

#define UDP_PORT        8888
static struct udp_pcb *udp_client_pcb = NULL;
static uint8_t agent_ip[4] = {192, 168, 1, 10};

#define MAX_RX_PACKETS 8
typedef struct {
    uint8_t data[1024];
    size_t len;
} udp_packet_t;

static udp_packet_t rx_packets[MAX_RX_PACKETS];
static volatile int rx_head = 0;
static volatile int rx_tail = 0;

static void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (p != NULL)
    {
        int next_head = (rx_head + 1) % MAX_RX_PACKETS;
        if (next_head != rx_tail)
        {
            if (p->tot_len <= sizeof(rx_packets[rx_head].data))
            {
                pbuf_copy_partial(p, rx_packets[rx_head].data, p->tot_len, 0);
                rx_packets[rx_head].len = p->tot_len;
                rx_head = next_head;
            }
        }
        pbuf_free(p);
    }
}

bool cubemx_transport_open(struct uxrCustomTransport * transport)
{
    (void)transport;
    if (udp_client_pcb != NULL) return true; // Already open

    udp_client_pcb = udp_new();
    if (!udp_client_pcb) return false;

    ip_addr_t dest_ip;
    IP_ADDR4(&dest_ip, agent_ip[0], agent_ip[1], agent_ip[2], agent_ip[3]);

    if (udp_connect(udp_client_pcb, &dest_ip, UDP_PORT) != ERR_OK)
    {
        udp_remove(udp_client_pcb);
        udp_client_pcb = NULL;
        return false;
    }

    // Bind to any local port
    udp_bind(udp_client_pcb, IP_ANY_TYPE, 0);

    udp_recv(udp_client_pcb, udp_receive_callback, NULL);

    rx_head = 0;
    rx_tail = 0;

    return true;
}

bool cubemx_transport_close(struct uxrCustomTransport * transport)
{
    (void)transport;
    if (udp_client_pcb != NULL)
    {
        udp_remove(udp_client_pcb);
        udp_client_pcb = NULL;
    }
    return true;
}

size_t cubemx_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err)
{
    (void)transport;
    (void)err;

    if (udp_client_pcb == NULL) return 0;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == NULL) return 0;

    pbuf_take(p, buf, len);
    err_t e = udp_send(udp_client_pcb, p);
    pbuf_free(p);

    if (e == ERR_OK)
    {
        return len;
    }
    return 0;
}

extern void MX_LWIP_Process(void);

size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{
    (void)transport;
    (void)err;

    int elapsed = 0;
    while ((rx_head == rx_tail) && elapsed < timeout)
    {
        MX_LWIP_Process();
        osDelay(1);
        elapsed += 1;
    }

    if (rx_head != rx_tail)
    {
        size_t pkt_len = rx_packets[rx_tail].len;
        size_t copy_len = (pkt_len > len) ? len : pkt_len;
        memcpy(buf, rx_packets[rx_tail].data, copy_len);
        rx_tail = (rx_tail + 1) % MAX_RX_PACKETS;
        return copy_len;
    }

    return 0;
}
#endif
