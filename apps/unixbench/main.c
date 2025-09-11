// #include <stdio.h> 
// #include <stdlib.h> 
// #include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
// #include <pthread.h> 
// #include <uk/print.h>
// #include <flexos/isolation.h>
// #include <uk/plat/mm.h>



// /** 
//  * This is an example of a "ping" sender (with raw API and socket API).
//  * It can be used as a start point to maintain opened a network connection, or
//  * like a network "watchdog" for your device.
//  *
//  */

// #include "lwip/opt.h"


// #ifndef PING_USE_SOCKETS
// #define PING_USE_SOCKETS    LWIP_SOCKET
// #endif

// #include "lwip/mem.h"
// #include "lwip/tcp.h"
// #include "lwip/icmp.h"
// #include "lwip/netif.h"
// #include "lwip/sys.h"
// #include "lwip/inet_chksum.h"
// #include "lwip/prot/ip.h"
// #include "lwip/prot/ip4.h"

// #if PING_USE_SOCKETS
// #include "lwip/sockets.h"
// #include "lwip/inet.h"
// #endif /* PING_USE_SOCKETS */


// /**
//  * PING_DEBUG: Enable debugging for PING.
//  */
// #ifndef PING_DEBUG
// #define PING_DEBUG     LWIP_DBG_ON
// #endif

// /** ping target - should be a "ip_addr_t" */
// #ifndef PING_TARGET
// //#define PING_TARGET   (netif_default?netif_default->gw:ip_addr_any)
// #define PING_TARGET IPADDR4_INIT((u32_t)0x3901A8C0UL)
// #endif

// /** ping receive timeout - in milliseconds */
// #ifndef PING_RCV_TIMEO
// #define PING_RCV_TIMEO 1000
// #endif

// /** ping delay - in milliseconds */
// #ifndef PING_DELAY
// #define PING_DELAY     1000
// #endif

// /** ping identifier - must fit on a u16_t */
// #ifndef PING_ID
// #define PING_ID        0xAFAF
// #endif

// /** ping additional data size to include in the packet */
// #ifndef PING_DATA_SIZE
// #define PING_DATA_SIZE 32
// #endif

// /** ping result action - no default action */
// #ifndef PING_RESULT
// #define PING_RESULT(ping_ok)
// #endif

// /* ping variables */
// static u16_t ping_seq_num;
// static u32_t ping_time;
// //#if !PING_USE_SOCKETS
// struct raw_pcb *ping_pcb;
// //#endif /* PING_USE_SOCKETS */

// /** Prepare a echo ICMP request */
// static void
// ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
// {
//   size_t i;
//   size_t data_len = len - sizeof(struct icmp_echo_hdr);

//   ICMPH_TYPE_SET(iecho, ICMP_ECHO);
//   ICMPH_CODE_SET(iecho, 0);
//   iecho->chksum = 0;
//   iecho->id     = PING_ID;
//   iecho->seqno  = htons(++ping_seq_num);

//   /* fill the additional data buffer with some data */
//   for(i = 0; i < data_len; i++) {
//     ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
//   }

//   iecho->chksum = inet_chksum(iecho, len);
// }

// // #if PING_USE_SOCKETS

// // /* Ping using the socket ip */
// // static err_t
// // ping_send(int s, ip_addr_t *addr)
// // {
// //   int err;
// //   struct icmp_echo_hdr *iecho;
// //   struct sockaddr_in to;
// //   size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
// //   LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

// //   iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
// //   if (!iecho) {
// //     return ERR_MEM;
// //   }

// //   ping_prepare_echo(iecho, (u16_t)ping_size);

// //   to.sin_len = sizeof(to);
// //   to.sin_family = AF_INET;
// //   //inet_addr_from_ip4addr(&to.sin_addr, addr);
// //   to.sin_addr.s_addr = addr->addr;

// //   err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));
// //   mem_free(iecho);

// //   return (err ? ERR_OK : ERR_VAL);
// // }

// // static void
// // ping_recv(int s)
// // {
// //   char buf[64];
// //   int fromlen, len;
// //   struct sockaddr_in from;
// //   struct ip_hdr *iphdr;
// //   struct icmp_echo_hdr *iecho;

// //   while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
// //     if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) {
// //       ip_addr_t fromaddr;
// //       inet_addr_to_ip4addr(&fromaddr, &from.sin_addr);
// //       LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
// //       ip_addr_debug_print(PING_DEBUG, &fromaddr);
// //       LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now() - ping_time)));

// //       iphdr = (struct ip_hdr *)buf;
// //       iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
// //       if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
// //         /* do some ping result processing */
// //         PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
// //         return;
// //       } else {
// //         LWIP_DEBUGF( PING_DEBUG, ("ping: drop\n"));
// //       }
// //     }
// //   }

// //   if (len == 0) {
// //     LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %"U32_F" ms - timeout\n", (sys_now()-ping_time)));
// //   }

// //   /* do some ping result processing */
// //   PING_RESULT(0);
// // }

// // static void
// // ping_thread(void *arg)
// // {
// //   int s;
// //   int timeout = PING_RCV_TIMEO;
// //   ip_addr_t ping_target;

// //   LWIP_UNUSED_ARG(arg);

// //    uk_pr_crit("going to setup lwip socket\n");
// //   if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
// //     return;
// //   }

// //   uk_pr_crit("setup lwip socket\n");

// //   lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

// //   while (1) {
// //     ping_target.addr = PING_TARGET;

// //     if (ping_send(s, &ping_target) == ERR_OK) {
// //         uk_pr_crit("ping sent\n");
// //       LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
// //       ip_addr_debug_print(PING_DEBUG, &ping_target);
// //       LWIP_DEBUGF( PING_DEBUG, ("\n"));

// //       ping_time = sys_now();
// //       ping_recv(s);
// //     } else {
// //       LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
// //       ip_addr_debug_print(PING_DEBUG, &ping_target);
// //       LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
// //     }
// //     sys_msleep(PING_DELAY);
// //   }
// // }

// // #else /* PING_USE_SOCKETS */

// /* Ping using the raw ip */
// static u8_t
// ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, ip_addr_t *addr)
// {
//   struct icmp_echo_hdr *iecho;
//   LWIP_UNUSED_ARG(arg);
//   LWIP_UNUSED_ARG(pcb);
//   LWIP_UNUSED_ARG(addr);
//   LWIP_ASSERT("p != NULL", p != NULL);

//   if (pbuf_header( p, -PBUF_IP_HLEN)==0) {
//     iecho = (struct icmp_echo_hdr *)p->payload;

//     if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
//       LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
//       ip_addr_debug_print(PING_DEBUG, addr);
//       LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now()-ping_time)));

//       /* do some ping result processing */
//       PING_RESULT(1);
//       pbuf_free(p);
//       return 1; /* eat the packet */
//     }
//   }

//   return 0; /* don't eat the packet */
// }

// static void
// ping_send(struct raw_pcb *raw, ip_addr_t *addr)
// {
//   struct pbuf *p;
//   struct icmp_echo_hdr *iecho;
//   size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

//   LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
//   ip_addr_debug_print(PING_DEBUG, addr);
//   LWIP_DEBUGF( PING_DEBUG, ("\n"));
//   LWIP_ASSERT("ping_size <= 0xffff", ping_size <= 0xffff);

//   p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
//   if (!p) {
//     uk_pr_crit("not p");
//     return;
//   }
//   if ((p->len == p->tot_len) && (p->next == NULL)) {
//     iecho = (struct icmp_echo_hdr *)p->payload;

//     ping_prepare_echo(iecho, (u16_t)ping_size);

//     err_t res = raw_sendto(raw, p, addr);
//     ping_time = sys_now();
//     if (res == ERR_OK)
//         uk_pr_crit("ping sent\n");
//     else 
//          uk_pr_crit("ping error %d\n", res);
//   }
//   pbuf_free(p);
// }

// static void
// ping_timeout(void *arg)
// {
//   struct raw_pcb *pcb = (struct raw_pcb*)arg;
//   ip_addr_t ping_target = PING_TARGET;
  
//   LWIP_ASSERT("ping_timeout: no pcb given!", pcb != NULL);

//   ping_send(pcb, &ping_target);

//   sys_timeout(PING_DELAY, ping_timeout, pcb);
// }

// static void
// ping_raw_init(void)
// {
//   struct raw_pcb* ping_pcb1 = raw_new(IP_PROTO_ICMP);
//   //LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);
//   UK_ASSERT(__builtin_cheri_tag_get(ping_pcb1) == 1);
//   ping_pcb = ping_pcb1;
//   UK_ASSERT(__builtin_cheri_tag_get(ping_pcb) == 1);
//   raw_recv(ping_pcb, ping_recv, NULL);
//   raw_bind(ping_pcb, IP_ADDR_ANY);
//   sys_timeout(PING_DELAY, ping_timeout, ping_pcb);
// }

// void
// ping_send_now()
// {
//   ip_addr_t ping_target = PING_TARGET;
//   LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);
//   ping_send(ping_pcb, &ping_target);
// }

// //#endif /* PING_USE_SOCKETS */

// void
// ping_init(void)
// {
// //   //sys_thread_new("ping_thread", ping_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
// //   ping_thread(NULL);
// //    #if PING_USE_SOCKETS
// //    uk_pr_crit("usign sockets\n");
// //   //sys_thread_new("ping_thread", ping_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
// // #else /* PING_USE_SOCKETS */
//   ping_raw_init();
// //#endif /* PING_USE_SOCKETS */
// }

// void echo_accept() {
//   uk_pr_crit("connected\n");
// }

// void echo_sent() {
//   uk_pr_crit("sent\n");
// }





// // void echo_tx_tcp()
// // {
// //   err_t wr_err = ERR_OK;
// //   struct tcp_pcb *l_tcp_pcb;
// //   l_tcp_pcb = tcp_new();
// //   //l_tcp_pcb->remote_port = 0x45;
// //   ip_addr_t dest_ip =
// //   { ((u32_t)0x3901A8C0UL) };
// //   wr_err = tcp_bind(l_tcp_pcb, &dest_ip, 69);
// //   tcp_bind_netif(l_tcp_pcb, netif_default);
// //   while(tcp_connect(l_tcp_pcb, &dest_ip, 69, echo_accept) != ERR_OK);
// //   tcp_sent(l_tcp_pcb, echo_sent);

// //   struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 1024, PBUF_RAM);
// //   unsigned char buffer_send[1024] = "Hello";
// //   p->payload = buffer_send;
// //   p->len = 1024;
// //   p->tot_len = 1024;

// //   while(tcp_write(l_tcp_pcb, p->payload, p->len, 1) != ERR_OK);
// //   while(tcp_output(l_tcp_pcb) != ERR_OK);

// //   if(wr_err == ERR_OK)
// //   {
// //     p->len++;
// //   }
// //   return;
// // }

// unsigned long global_things;
  
// // A normal C function that is executed as a thread  
// // when its name is specified in pthread_create() 
// void *myThreadFun(void *vargp) 
// { 
//    // sleep(1); 
//     uk_pr_crit("Printing from thread 2 \n"); 
//     pthread_exit(NULL);
//     return NULL; 
// } 
   
// int main() 
// { 
//     uint64_t smcc_ver;
//     void* stackptr1 = malloc(sizeof(int));
//     void* stackptr = uk_malloc(flexos_shared_alloc, sizeof(int*));
//     // 	asm("str %1, [csp, #-16]!\n"
//     //         "mov w0, %w0\n"
// 	// 	"smc #0\n"
//     //     "ldr %1, [csp], #16\n"
//     //     "str x0, [%1]\n"
//     // //    "mov c0, csp\n"
//     // //    "str x0, [%2]\n"
// 	// 	:
// 	// 	: "r"(0x84000000), "r"(&smcc_ver)
// 	// 	:"c0");
//     uintptr_t* smcc_ver_ptr = &smcc_ver;
//     unsigned long pte = get_pte(stackptr1);
//     int rperms = pte & (3 << 6);
//     uk_pr_crit("Perms %d\n", rperms);
//     uk_pr_crit("ptr, where is it? %p, sp: %p, global %p\n", smcc_ver_ptr, stackptr, &global_things);
//     uk_pr_crit("smccc version %d\n", smcc_ver);
//     pthread_t thread_id; 
//     pthread_t thread_id1; 
//     uk_pr_crit("Before Thread 1, strlen %d\n", strlen("very long")); 
//     pthread_create(&thread_id, NULL, myThreadFun, NULL); 
//     pthread_create(&thread_id1, NULL, myThreadFun, NULL); 
//    pthread_join(thread_id, NULL);
//    pthread_join(thread_id1, NULL);
//     uk_pr_crit("After Thread 1\n"); 

//    ping_init();
//     uk_pr_crit("After ping\n"); 
//    // asm("wfi\n");
//    while(1);
//     return 0; 
// }



#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <flexos/isolation.h>
#include <sfork.h>

#define LISTEN_PORT 8123
static const char reply[] = "HTTP/1.1 200 OK\r\n" \
			    "Content-type: text/html\r\n" \
			    "Connection: close\r\n" \
			    "\r\n" \
			    "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">" \
			    "<html>" \
			    "<head><title>Unikraft On Purecap CHERI</title></head>" \
			    "<body><h1>Unikraft On Purecap CHERI!</h1><p>Hello Internet</p></body>" \
			    "</html>\n";

#define BUFLEN 2048
static char recvbuf[BUFLEN];

int main(int argc __attribute__((unused)),
	 char *argv[] __attribute__((unused)))
{

 
    // make two process which run same
    // program after this instruction
    pid_t p = DO_SFORK();
	printf("Hello world!, process_id(pid) = %d, %p \n", p, &p);
	if (p == 0) {
		uk_pr_crit("Printing from child pid %d\n", p);
		uk_sched_thread_sleep(100000);
		uk_pr_crit("Exiting pid: %d\n", p);
		return 0;
	}

	uk_pr_crit("Printing from parent pid %d\n", p);
	//sleep(1000);
	uk_sched_thread_sleep(100000);
	uk_pr_crit("Printing from parent pid %d\n", p);
	uk_sched_thread_sleep(100000);
		//goto out;
	//}

out:
	return 0;
}