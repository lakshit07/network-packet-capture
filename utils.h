#ifndef utilsh
#define utilsh

#include <netinet/in.h>
#include <errno.h>
#include <stdio.h> //For standard things
#include <stdlib.h>    //malloc
#include <string.h>    //strlen
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netinet/if_ether.h>  //For ETH_P_ALL
#include <net/ethernet.h>  //For ether_header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>


#define MAX 5005

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

FILE *flog;
struct sockaddr_in source,dest;
int tcp,udp,http,dns,ip,total,i,j,k;
int packlen[MAX]; 
char Sour[MAX][20];
char Dest[MAX][20];
char det[MAX][20];
char tmp[MAX][20];
int ip_index[MAX];
int tcp_index[MAX];
int udp_index[MAX]; 
int ip_count;
int tcp_count;
int udp_count;
int por1[MAX];
int por2[MAX];
int typr[MAX][4];
int pot1[MAX];
int pot2[MAX];
int conp;
int cnt;
int cntdrop;
double tm[MAX];
double tmdrop[MAX];
struct ethhdr * ethern[MAX];
struct iphdr* ipthern[MAX];
struct tcphdr* tcpthern[MAX];
struct udphdr* udpthern[MAX];
int a,orig;

static void interface(int fd, const char *name) ;
static void list_all(int fd, void (*show)(int fd, const char *name)) ;
void show(int family) ;
void ethernet_packet(unsigned char* buf, int size);
void show_interfaces();
void PrintData(unsigned char* data , int size);
void ip_packet(char* buf, int size);
void udp_packet(unsigned char *buf , int size);
void tcp_packet(unsigned char* buf, int size);
void analyze(unsigned char* buf, int size);
int raw_init(const char *device);
void get_list();
void converse();
void protocol_hierarchy();
void display_traffic();
void display_packlen();
void filter();
unsigned short compute_tcp_checksum(struct iphdr *pIph, unsigned short *ipPayload);
unsigned short compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload);
void display_drop();

struct ethernet
{
	unsigned int src[6];
	unsigned int dst[6];
	unsigned int protocol;
};
typedef struct ethernet ethernet;

struct ip_protocol
{
	unsigned int version;
	unsigned int headlen;
	unsigned int service;
	unsigned int totlen;
	unsigned int id;
	unsigned int offset;
	unsigned int time;
	unsigned int protocol;
	int checksum;
	char src[50];
	char dst[50];
};
typedef struct ip_protocol ip_protocol;

struct udp_protocol
{
	int src;
	int dst;
	int len;
	int checksum;
};
typedef struct udp_protocol udp_protocol;

struct tcp_protocol
{
	unsigned int src;
	unsigned int dst;
	unsigned int seq_no;
	unsigned int ack_no;
	unsigned int headlen;
	unsigned int urgent,ack,push,reset,syn,fin;
	unsigned int window;
	unsigned int checksum;
	unsigned int urgent_ptr;
};
typedef struct tcp_protocol tcp_protocol;

ethernet* get_ether_details(struct ethhdr*);
void print_ether_details(ethernet*);
ip_protocol* get_ip_details(struct iphdr*);
void print_ip_details(ip_protocol*);
udp_protocol* get_udp_details(struct udphdr*);
void print_udp_details(udp_protocol*);
tcp_protocol* get_tcp_details(struct tcphdr*);
void print_tcp_details(tcp_protocol*);


#endif
