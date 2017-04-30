#include "utils.h"

static void interface(int fd, const char *name) 
{
    struct ifreq ifreq;
    char host[128];
    memset(&ifreq, 0, sizeof ifreq);
    strncpy(ifreq.ifr_name, name, IFNAMSIZ);
    
    if(ioctl(fd, SIOCGIFADDR, &ifreq)!=0) 
      return; 
    
    int family=ifreq.ifr_addr.sa_family;
    getnameinfo(&ifreq.ifr_addr, sizeof ifreq.ifr_addr, host, sizeof host, 0, 0, NI_NUMERICHOST);
    printf("Interface : %-24s IP Address : %s\n", name, host);
}

static void list_all(int fd, void (*show)(int fd, const char *name)) 
{
    struct ifreq *ir;
    struct ifconf ic;
    char buf[100];
    size_t len;

    ic.ifc_len = sizeof buf;
    ic.ifc_buf = buf;
    
    if(ioctl(fd, SIOCGIFCONF, &ic)!=0) 
    {
        perror("ioctl(SIOCGIFCONF)");
        exit(EXIT_FAILURE);
    }

    ir = ic.ifc_req;
    int i;
    for(i = 0 ; i < ic.ifc_len; ) 
    {
        len=sizeof *ir;
        if(show) 
            show(fd, ir->ifr_name);      
        else 
            printf("%s\n", ir->ifr_name);
        
        ir = (struct ifreq*)((char*)ir+len);
        i+=len;
    }
}

void show(int family) 
{
    int f =socket(family, SOCK_DGRAM, 0);
    if(f == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    list_all(f, interface);
    close(f);
}

void show_interfaces()
{
    show(PF_INET);
    show(PF_INET6);
}

void protocol_hierarchy()
{
	printf("TOTAL PACKETS CAPTURED : %d\n" , total);
	printf("APPLICATION LAYER : HTTP - %d\n" , http);
	printf("                  : DNS  - %d\n" , dns);
	printf("TRANSPORT LAYER   : TCP  - %d\n" , tcp);
	printf("                  : UDP  - %d\n" , udp);
	printf("NETWORK LAYER     : IP   - %d\n" , ip);
}

void print_ether_details(ethernet* eptr)
{
    fprintf(flog , "\n");
    fprintf(flog , "Ethernet Header\n");
    fprintf(flog , "   Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eptr->src[0] , eptr->src[1] , eptr->src[2] , eptr->src[3] , eptr->src[4] , eptr->src[5] );
    fprintf(flog , "   Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eptr->dst[0] , eptr->dst[1] , eptr->dst[2] , eptr->dst[3] , eptr->dst[4] , eptr->dst[5] );
    fprintf(flog , "   Protocol            : %u \n",eptr->protocol);
}

ethernet* get_ether_details(struct ethhdr* eth)
{
    ethernet* eptr = (ethernet*)malloc(sizeof(ethernet));
    int i;
    for(i = 0; i < 6; i++)
    {
        eptr->src[i] = eth->h_source[i];
        eptr->dst[i] = eth->h_dest[i];
    }
    eptr->protocol = (unsigned short)eth->h_proto;
    return eptr;
}

void ethernet_packet(unsigned char* buf, int size)
{
    struct ethhdr *eth = (struct ethhdr *)buf;
    ethernet* eptr = get_ether_details(eth);
    print_ether_details(eptr);
    ethern[cnt]=malloc(sizeof(struct ethhdr));
    ethern[cnt]->h_proto=eptr->protocol;
    for(i=0;i<6;i++)
    {
        ethern[cnt]->h_dest[i]=eptr->dst[i];
        ethern[cnt]->h_source[i]=eptr->src[i];
    }
    free(eptr);
}

void PrintData(unsigned char* data , int size)
{
    int i,j;
    fprintf(flog,"\nPacket data  \n");
    for(i = 0 ; i < size ; i++)
    { 
        if(i % 80 == 0) fprintf(flog,"\n");  
        if(data[i] >= 32 && data[i] <= 128)
            fprintf(flog , "%c",(unsigned char)data[i]); 
        else fprintf(flog , ".");

    }
    fprintf(flog,"\n");
           
}

void print_ip_details(ip_protocol* ipptr)
{
    fprintf(flog , "\n");
    fprintf(flog , "IP Header\n");
    fprintf(flog , "   Version        : %d\n",ipptr->version);
    fprintf(flog , "   Header Length  : %d bytes\n",ipptr->headlen);
    fprintf(flog , "   TOS   : %d\n",ipptr->service);
    fprintf(flog , "   Total Length   : %d  bytes\n",ipptr->totlen);
    fprintf(flog , "   Identification    : %d\n",ipptr->id);
    fprintf(flog , "   Fragmentation offset    : %d\n" , ipptr->offset);
    fprintf(flog , "   TTL      : %d\n",ipptr->time);
    fprintf(flog , "   Protocol : %d\n",ipptr->protocol);
    fprintf(flog , "   Checksum : %d\n",ipptr->checksum);
    fprintf(flog , "   Source IP Address       : %s\n",ipptr->src);
    fprintf(flog , "   Destination IP Address  : %s\n",ipptr->dst);
}

ip_protocol* get_ip_details(struct iphdr* iph)
{
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr; 
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
    ip_protocol* ipptr = (ip_protocol*)malloc(sizeof(ip_protocol));
    ipptr->version = (unsigned int)iph->version;
    ipptr->headlen = ((unsigned int)(iph->ihl))*4;
    ipptr->service = (unsigned int)iph->tos;
    ipptr->totlen = ntohs(iph->tot_len);
    ipptr->id = ntohs(iph->id);
    ipptr->offset = iph->frag_off;
    ipptr->time = (unsigned int)iph->ttl;
    ipptr->protocol = (unsigned int)iph->protocol;
    ipptr->checksum = ntohs(iph->check);
    strcpy(ipptr->src,inet_ntoa(source.sin_addr));
    strcpy(ipptr->dst,inet_ntoa(dest.sin_addr));
    return ipptr;
}

void ip_packet(char* buf, int size)
{
    ethernet_packet(buf , size);
    ip++;
   
    unsigned short iphdrlen;
         
    struct iphdr *iph = (struct iphdr *)(buf  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;

    fprintf(flog , "\n");
    fprintf(flog , "IP Header\n");
    PrintData(buf , iphdrlen);
    ip_protocol* ipptr = get_ip_details(iph); 
    print_ip_details(ipptr);

    strcpy(Sour[cnt],ipptr->src);
    strcpy(Dest[cnt],ipptr->dst);
    ipthern[cnt]=   malloc( sizeof(struct iphdr));
    ipthern[cnt]->version=ipptr->version;
    ipthern[cnt]->ihl=ipptr->headlen;
    ipthern[cnt]->tos=ipptr->service;
    ipthern[cnt]->tot_len=ipptr->totlen;
    ipthern[cnt]->id=ipptr->id;
    ipthern[cnt]->frag_off=ipptr->offset;
    ipthern[cnt]->ttl=ipptr->time;
    ipthern[cnt]->protocol=ipptr->protocol;
    ipthern[cnt]->check=ipptr->checksum;
    ipthern[cnt]->saddr=iph->saddr;
    ipthern[cnt]->daddr=iph->daddr;
    free(ipptr);

}
 
void print_udp_details(udp_protocol* udpptr)
{
    fprintf(flog , "\nUDP Header\n");
    fprintf(flog , "   Source Port      : %d\n" , udpptr->src);
    fprintf(flog , "   Destination Port : %d\n" , udpptr->dst);
    fprintf(flog , "   Length       : %d\n" , udpptr->len);
    fprintf(flog , "   Checksum     : %d\n" , udpptr->checksum);
}

udp_protocol* get_udp_details(struct udphdr* udph)
{
    udp_protocol* udpptr = (udp_protocol*)malloc(sizeof(udp_protocol));
    udpptr->src = ntohs(udph->source);
    udpptr->dst = ntohs(udph->dest);
    udpptr->len = ntohs(udph->len);
    udpptr->checksum = ntohs(udph->check);
    return udpptr;
}

void udp_packet(unsigned char *buf , int size)
{   
    udp_index[udp_count]=cnt;
    udp_count++;
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)(buf +  sizeof(struct ethhdr));
    iphdrlen = iph->ihl*4;
     
    struct udphdr *udph = (struct udphdr*)(buf + iphdrlen  + sizeof(struct ethhdr));
     
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof udph;

    bool ht = false;
    bool dn = false;
     
    fprintf(flog , "\n\n                        UDP PACKET                              \n");
     
    ip_packet(buf,size);  
    
     if(ntohs(udph->source) == 80 || ntohs(udph->dest) == 80 || ntohs(udph->source) == 443 || ntohs(udph->dest) == 443)
    {
        http++;
        ht = true;
        // printf("HTTP packet found  :  TCP : %4d  UDP : %4d  HTTP : %4d  DNS : %4d\r", tcp , udp , http , dns);
    }    
    if(ntohs(udph->source) == 53 || ntohs(udph->dest) == 53 || ntohs(udph->source) == 5353 || ntohs(udph->dest) == 5353)
    {
        dns++;
        dn = true;
        // printf("DNS packet found  :  TCP : %4d  UDP : %4d  HTTP : %4d  DNS : %4d\r", tcp , udp , http , dns);    
    }               
    udp_protocol* udpptr = get_udp_details(udph); 
    print_udp_details(udpptr);

    orig = ntohs(udph->check);
    unsigned short s = compute_udp_checksum(iph,(unsigned short*)udph);
    fprintf(flog , "   Calculated Checksum       : %d\n",ntohs(s));
     a = ntohs(s);
   
    char pr[50];

    if(ht)
        strcpy(pr , "HTTP(S)");
    else if(dn)
        strcpy(pr , "DNS");
    else
        strcpy(pr , "UNKNOWN");

    fprintf(flog , "\n%s Application Payload\n", pr);    
     
    //Move the pointer ahead and reduce the size of string
    PrintData(buf + header_size , size - header_size);
     
    fprintf(flog , "\n________________________________________________________________________________\n");
    por1[cnt]=ntohs(udph->source);
    por2[cnt]=ntohs(udph->dest);
    udpthern[cnt] = malloc(sizeof(struct udphdr));
    udpthern[cnt]->source = udph->source;
    udpthern[cnt]->dest = udph->dest;
    udpthern[cnt]->len = udph->len;
    udpthern[cnt]->check = udph->check;
    free(udpptr);
} 

void print_tcp_details(tcp_protocol* tcpptr)
{
    fprintf(flog , "\n");
    fprintf(flog , "TCP Header\n");
    fprintf(flog , "   Source Port      : %u\n",tcpptr->src);
    fprintf(flog , "   Destination Port : %u\n",tcpptr->dst);
    fprintf(flog , "   Sequence Number    : %u\n",tcpptr->seq_no);
    fprintf(flog , "   Acknowledge Number : %u\n",tcpptr->ack_no);
    fprintf(flog , "   Header Length      : %d BYTES\n" , tcpptr->headlen);
    fprintf(flog , "   Flags (URG|ACK|PSH|RST|SYN|FIN)    : %d %d %d %d %d %d\n",tcpptr->urgent, tcpptr->ack,tcpptr->push, tcpptr->reset, tcpptr->syn, tcpptr->fin); 
    fprintf(flog , "   Window         : %d\n",tcpptr->window);
    fprintf(flog , "   Checksum       : %d\n",tcpptr->checksum);
    fprintf(flog , "   Urgent Pointer : %d\n",tcpptr->urgent_ptr);
}

tcp_protocol* get_tcp_details(struct tcphdr* tcph)
{
    tcp_protocol* tcpptr = (tcp_protocol*)malloc(sizeof(tcp_protocol));
    tcpptr->src = ntohs(tcph->source);
    tcpptr->dst = ntohs(tcph->dest);
    tcpptr->seq_no = ntohl(tcph->seq);
    tcpptr->ack_no = ntohl(tcph->ack_seq);
    tcpptr->headlen = (unsigned int)tcph->doff*4;
    tcpptr->urgent = (unsigned int)tcph->urg;
    tcpptr->ack = (unsigned int)tcph->ack;
    tcpptr->push = (unsigned int)tcph->psh;
    tcpptr->reset = (unsigned int)tcph->rst;
    tcpptr->syn = (unsigned int)tcph->syn;
    tcpptr->fin = (unsigned int)tcph->fin;
    tcpptr->window = ntohs(tcph->window);
    tcpptr->checksum = ntohs(tcph->check);
    tcpptr->urgent_ptr = tcph->urg_ptr;
    return tcpptr;
}

void tcp_packet(unsigned char* buf, int size)
{
    tcp_index[tcp_count] = cnt;
    tcp_count++;
    unsigned short iphdrlen;
     
    struct iphdr *iph = (struct iphdr *)( buf  + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;     
    struct tcphdr *tcph=(struct tcphdr*)(buf + iphdrlen + sizeof(struct ethhdr));             
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
     
    fprintf(flog , "\n                              TCP PACKET                              \n");  
         
    ip_packet(buf,size);
    bool ht = false;
    bool dn = false;

    if(ntohs(tcph->source) == 80 || ntohs(tcph->dest) == 80 || ntohs(tcph->source) == 443 || ntohs(tcph->dest) == 443)
    {
        http++;
        ht = true;
        // printf("HTTP packet found  :  TCP : %4d  UDP : %4d  HTTP : %4d  DNS : %4d\r", tcp , udp , http , dns);
    }    
    if(ntohs(tcph->source) == 53 || ntohs(tcph->dest) == 53 || ntohs(tcph->source) == 5353 || ntohs(tcph->dest) == 5353)
    {
        dns++;
        dn = true;
        // printf("DNS packet found  :  TCP : %4d  UDP : %4d  HTTP : %4d  DNS : %4d\r", tcp , udp , http , dns);    
    }      
    tcp_protocol* tcpptr = get_tcp_details(tcph);
    print_tcp_details(tcpptr);

    orig = ntohs(tcph->check);
    unsigned short s = compute_tcp_checksum(iph,(unsigned short*)tcph);
    fprintf(flog , "   Calculated Checksum       : %d\n",ntohs(s));
     a = ntohs(s);

    char pr[50];

    if(ht)
        strcpy(pr , "HTTP(S)");
    else if(dn)
        strcpy(pr , "DNS");
    else
        strcpy(pr , "UNKNOWN");

    fprintf(flog , "\n%s Application Payload\n", pr);    
    PrintData(buf + header_size , size - header_size );
                         
    fprintf(flog , "\n____________________________________________________________________________\n");
    por1[cnt] = ntohs(tcph->source);
    por2[cnt] = ntohs(tcph->dest);
    tcpthern[cnt]=malloc(sizeof(struct tcphdr));
    tcpthern[cnt]->source=tcph->source;
    tcpthern[cnt]->dest=tcph->dest;
    tcpthern[cnt]->seq=tcph->seq;
    tcpthern[cnt]->ack_seq=tcph->ack_seq;
    tcpthern[cnt]->doff=tcph->doff;
    tcpthern[cnt]->window=tcph->window;
    tcpthern[cnt]->check=tcph->check;
    tcpthern[cnt]->urg=tcph->urg;
    tcpthern[cnt]->ack=tcph->ack;
    tcpthern[cnt]->psh=tcph->psh;
    tcpthern[cnt]->rst=tcph->rst;
    tcpthern[cnt]->syn=tcph->syn;
    tcpthern[cnt]->fin=tcph->fin;
    tcpthern[cnt]->urg_ptr=tcph->urg_ptr;
    free(tcpptr);
}

unsigned short compute_tcp_checksum(struct iphdr *pIph, unsigned short *ipPayload) 
{
    register unsigned long sum = 0;
    unsigned short tcpLen = ntohs(pIph->tot_len) - (pIph->ihl<<2);
    struct tcphdr *tcphdrp = (struct tcphdr*)(ipPayload);
    sum += (pIph->saddr>>16)&0xFFFF;
    sum += (pIph->saddr)&0xFFFF;
    sum += (pIph->daddr>>16)&0xFFFF;
    sum += (pIph->daddr)&0xFFFF;
    sum += htons(IPPROTO_TCP);
    sum += htons(tcpLen);
    tcphdrp->check = 0;

    while (tcpLen > 1) 
    {
        sum += * ipPayload++;
        tcpLen -= 2;
    }

    if(tcpLen > 0) 
    {
        sum += ((*ipPayload)&htons(0xFF00));
    }

    while(sum>>16) 
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
      sum = ~sum;
    return (unsigned short)sum;
}

unsigned short compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload) 
{
    register unsigned long sum = 0;
    struct udphdr *udphdrp = (struct udphdr*)(ipPayload);
    unsigned short udpLen = htons(udphdrp->len);
    sum += (pIph->saddr>>16)&0xFFFF;
    sum += (pIph->saddr)&0xFFFF;
    sum += (pIph->daddr>>16)&0xFFFF;
    sum += (pIph->daddr)&0xFFFF;
    sum += htons(IPPROTO_UDP);
    sum += udphdrp->len;
    udphdrp->check = 0;

    while (udpLen > 1) 
    {
        sum += * ipPayload++;
        udpLen -= 2;
    }

    if(udpLen > 0) 
    {
        sum += ((*ipPayload)&htons(0xFF00));
    }

    while (sum>>16) 
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    sum = ~sum;
    return ((unsigned short)sum == 0x0000)?0xFFFF:(unsigned short)sum;
}

void analyze(unsigned char* buf, int size)
{
    struct iphdr *iph = (struct iphdr*)(buf + sizeof(struct ethhdr));
    ++total;
    if(iph->protocol == 6) 
    {  
        ++tcp;
        tcp_packet(buf , size);
        ++cnt;
    } 
    else if(iph->protocol == 17) 
    {     
        ++udp;
        udp_packet(buf , size);
        ++cnt;
    }
}
 
int raw_init (const char *device)
{
    struct ifreq ifr;
    int raw_socket = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));

    memset (&ifr, 0, sizeof (struct ifreq));

    if (raw_socket < 1)
    {
        printf ("ERROR: Could not open socket.\n");
        exit(1);
    }

    strcpy (ifr.ifr_name, device);

    if (ioctl (raw_socket, SIOCGIFFLAGS, &ifr) == -1)
    {
        perror ("Error: Device unreachable. Probably unsufficient permissions.\n");
        exit (1);
    }

    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl (raw_socket, SIOCSIFFLAGS, &ifr) == -1)
    {
        perror ("Error: Could not set interface to promiscous mode");
        exit (1);
    }
    printf ("%s capturing in promiscuous mode\n", device);

    if (ioctl (raw_socket, SIOCGIFINDEX, &ifr) < 0)
    {
        perror ("Error: Interface unavailable.\n");
        exit (1);
    }

    return raw_socket;
}

void get_list()
{
	int i, j;
    int cur=0;
    int temp = 0;
    for(i=0;i<cnt;i++)
    {
        //printf("%s\n",Sour[i]);
        int flg=0;
        for(j=0;j<cur;j++)
        {
            if(strcmp(Sour[i],tmp[j])==0)
            {
                flg=1;
                 break;
            }
        }
        if(flg==0)
        {
            strcpy(tmp[cur],Sour[i]);
            printf("%18s\t",tmp[cur]);
            temp++;
            if(temp%3 == 0)
                printf("\n");
            cur++;
        }
    }
    for(i=0;i<cnt;i++)
    {
        int flg=0;
        for(j=0;j<cur;j++)
        {
            if(strcmp(Dest[i],tmp[j])==0)
            {
                flg=1;
                 break;
            }
        }
        if(flg==0)
        {
            strcpy(tmp[cur],Dest[i]);
            printf("%18s\t",tmp[cur]);
            temp++;
            if(temp%3 == 0)
                printf("\n");
            cur++;
        }
    }
    for(i=0;i<cur;i++)
    {
        strcpy(tmp[i],"");
    }
}

void converse()
{
    printf(ANSI_COLOR_CYAN "This is the list of IP adresses\n"ANSI_COLOR_RESET);
        sleep(1);
        get_list();
        printf(ANSI_COLOR_CYAN "\nEnter IP address 1 : " ANSI_COLOR_RESET);
        char a[15];
        scanf("%s",a);
        char b[15];
        printf(ANSI_COLOR_CYAN "Enter IP address 2 : " ANSI_COLOR_RESET);
        scanf("%s",b);
        int port1,port2;
        int typ=0;
        int i, j;
        
        for(i=0;i<tcp_count;i++)
        {
            
            if(strcmp(a,Sour[tcp_index[i]])==0&&strcmp(b,Dest[tcp_index[i]])==0)
            {   
                typ=1;
                port1=por1[tcp_index[i]];
                port2=por2[tcp_index[i]];
                int flg=0;
                for(j=0;j<conp;j++)
                {
                    if(pot1[j]==port1&&pot2[j]==port2)
                    {
                        typr[j][typ-1]++;
                        flg=1;
                        break;       
                    }
                }
                if(flg==0)
                {
                    pot1[conp]=port1;
                    pot2[conp]=port2;
                    typr[conp][typ-1]++;
                    conp++;
                }
            }
            else if(strcmp(b,Sour[tcp_index[i]])==0&&strcmp(a,Dest[tcp_index[i]])==0)
            {
                typ=2;
                port2=por1[tcp_index[i]];
                port1=por2[tcp_index[i]];
                int flg=0;
                for(j=0;j<conp;j++)
                {
                    if(pot1[j]==port1&&pot2[j]==port2)
                    {
                        typr[j][typ-1]++;
                        flg=1;
                        break;       
                    }
                }
                if(flg==0)
                {
                    pot1[conp]=port1;
                    pot2[conp]=port2;
                    typr[conp][typ-1]++;
                    conp++;
                }
            }
        }
        for(i=0;i<udp_count;i++)
        {
            if(strcmp(a,Sour[udp_index[i]])==0&&strcmp(b,Dest[udp_index[i]])==0)
            {
                typ=3;
                port1=por1[tcp_index[i]];
                port2=por2[tcp_index[i]];
                int flg=0;
                for(j=0;j<conp;j++)
                {
                    if(pot1[j]==port1&&pot2[j]==port2)
                    {
                        typr[j][typ-1]++;
                        flg=1;
                        break;       
                    }
                }
                if(flg==0)
                {
                    pot1[conp]=port1;
                    pot2[conp]=port2;
                    typr[conp][typ-1]++;
                    conp++;
                }
            }
            else if(strcmp(b,Sour[udp_index[i]])==0&&strcmp(a,Dest[udp_index[i]])==0)
            {
                typ=4;
                port2=por1[tcp_index[i]];
                port1=por2[tcp_index[i]];
                int flg=0;
                for(j=0;j<conp;j++)
                {
                    if(pot1[j]==port1&&pot2[j]==port2)
                    {
                        typr[j][typ-1]++;
                        flg=1;
                        break;       
                    }
                }
                if(flg==0)
                {
                    pot1[conp]=port1;
                    pot2[conp]=port2;
                    typr[conp][typ-1]++;
                    conp++;
                }
            }
        }
        if(conp == 0)
            printf("No conversations found between hosts\n");
        else
        {
            printf(ANSI_COLOR_MAGENTA"\n           IP 1             IP 2    PORT 1    PORT 2  TCP[1>2]   TCP[2>1]   UDP[1>2]   UDP[2>1]\n" ANSI_COLOR_RESET);   
            for(i=0;i<conp;i++)
            {
                printf("%15s  %15s  %8d  %8d  %8d  %9d  %9d  %9d\n",a,b,pot1[i],pot2[i],typr[i][0],typr[i][1],typr[i][2],typr[i][3]);
            }
        }
}

void display_traffic()
{
    char * commandsForgnuplot[] = {"set grid","set terminal png","set output \'traffic.png\'","set xlabel \'Time(s)\'","set ylabel \'Packets per second\'","set title \'Network traffic graph\'", "set autoscale"
,"plot \'data.temp\' using 1:2 with lines title \'Network traffic graph\'","replot"};
    
    FILE * temp = fopen("data.temp", "w");
    

    int num_packets = cnt;
    double PART_T = 1;

    double max_t = tm[num_packets -1];
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i,j;
    j = 0;
    int SIZ = (int) max_t/PART_T;
    int num_freq[SIZ];
    for(i = 0 ; i < SIZ;i++)
        num_freq[i] = 0;
    for (i=0; i < num_packets; i++)
    {
        int in = (int)(tm[i]/PART_T);
        num_freq[in]++;
    }
    double t = 0;
    for(i=0 ; i < SIZ ; i++)
    {   
        fprintf(temp, "%lf %d\n", t, num_freq[i]);
        t += PART_T;
    }

    fclose(temp);
    for (i=0; i < 9; i++)
       fprintf(gnuplotPipe, "%s \n", commandsForgnuplot[i]); 
    
    fclose(gnuplotPipe);
    system("eog traffic.png >/dev/null 2>&1");
}

void display_drop()
{
    printf("\nNumber of packets dropped %d\n",cntdrop);
    printf("%% of packets dropped %lf\n",(double)cntdrop/cnt*100);
    if(cntdrop == 0) return;
    char * commandsForgnuplot[] = {"set grid","set terminal png","set output \'drop.png\'","set xlabel \'Time(s)\'","set ylabel \'Packets dropped per second\'","set title \'Packet drop graph\'", "set autoscale"
,"plot \'data2.temp\' using 1:2 with lines title \'Packet drop graph\'","replot"};
    
    FILE * temp = fopen("data2.temp", "w");
    

    int num_packets = cntdrop;
    double PART_T = 1;

    double max_t = tmdrop[num_packets -1];
    FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
    int i,j;
    j = 0;
    int SIZ = (int) max_t/PART_T;
    int num_freq[SIZ];
    for(i = 0 ; i < SIZ;i++)
        num_freq[i] = 0;
    for (i=0; i < num_packets; i++)
    {
        int in = (int)(tmdrop[i]/PART_T);
        num_freq[in]++;
    }
    double t = 0;
    for(i=0 ; i < SIZ ; i++)
    {   
        fprintf(temp, "%lf %d\n", t, num_freq[i]);
        t += PART_T;
    }

    fclose(temp);
    for (i=0; i < 9; i++)
       fprintf(gnuplotPipe, "%s \n", commandsForgnuplot[i]); 
    
    fclose(gnuplotPipe);
    system("eog drop.png >/dev/null 2>&1");
}


void display_packlen()
{
     int num_packets[16];
    for(i = 0; i < 16; i++)
        num_packets[i] = 0;

    int min_len , max_len ,sum_len;
    sum_len = 0;
    min_len = 1000000;
    max_len = 0;

    for(i = 0 ; i < cnt ; i++)
    {
        int len = packlen[i];
        if(len < min_len)
            min_len = len;
        else if(len > max_len)
            max_len = len;
        sum_len += len;
        if((len/20) >= 15)
            num_packets[15] ++; 
        else num_packets[len/20]++;
    }

    printf("Minimimum packet length = %d bytes\n",min_len);
    printf("Maximimum packet length = %d bytes\n",max_len);
    printf("Average packet length = %lf bytes\n",(double)sum_len / cnt);

    FILE * temp2 = fopen("data2.temp","w");
    for(i = 0; i < 16;i++)
    {
        char str[50];
        if(i == 15)
            strcpy(str,">=300");
        else
            sprintf(str,"%d-%d",20*i,20*(i+1)-1);
        fprintf(temp2,"%s %d\n",str,num_packets[i]);
    }
    fclose(temp2);
    char* histo[] ={"set grid","set terminal png size 1600,400","set output \'packets_plot.png\'","set boxwidth 0.9 relative","set style data histograms","set style fill solid 1.0 border -1","set autoscale","plot \'data2.temp\'  using 2:xticlabels(1)","replot"};
    FILE * gnuplotPipe2 = popen ("gnuplot -persistent", "w");
    for(i = 0 ; i < 9 ; i++)
        fprintf(gnuplotPipe2,"%s \n",histo[i]);
    fclose(gnuplotPipe2);
    system("eog packets_plot.png >/dev/null 2>&1");
    
}

void filter()
{
    printf(ANSI_COLOR_MAGENTA "\nAt application layer you can\n1.Capture only HTTP packets\n2.Capture only DNS packets\n3.Capture any packet\n" ANSI_COLOR_RESET);
    int ch1;
    printf("Choice : ");
    scanf("%d",&ch1);
    printf(ANSI_COLOR_MAGENTA "\nAt transport layer you can\n1.Capture only TCP packets\n2.Capture only UDP packets\n3.Capture any packet\n" ANSI_COLOR_RESET);
    int ch2;
    printf("Choice : ");
    scanf("%d",&ch2);
    char c;
    printf(ANSI_COLOR_MAGENTA "\nDo you want a detailed analysis(y/n)\n" ANSI_COLOR_RESET);
    printf("Choice : ");
    scanf(" %c",&c);

    if(c =='n' || c == 'N')
    {   
        if(ch1==1&&ch2==1)
        {
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
        }
        else if(ch1==1&&ch2==2)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }   
        }
        else if(ch1==1&&ch2==3)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
        }
        if(ch1==2&&ch2==1)
        {
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
        }
        else if(ch1==2&&ch2==2)
        {
            
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
            
        }
        else if(ch1==2&&ch2==3)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                    usleep(1e5);
                }
            }   
        }
        if(ch1==3&&ch2==1)
        {
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                usleep(1e5);
            }
        }
        else if(ch1==3&&ch2==2)
        {
            
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                usleep(1e5);                 
            }
        }
        else if(ch1==3&&ch2==3)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                usleep(1e5);    
            }
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                printf("%.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %.2X-%.2X-%.2X-%.2X-%.2X-%.2X     %15s     %15s     %d     %d\n", ethern[v]->h_dest[0] , ethern[v]->h_dest[1] , ethern[v]->h_dest[2] , ethern[v]->h_dest[3] , ethern[v]->h_dest[4] , ethern[v]->h_dest[5],ethern[v]->h_source[0] , ethern[v]->h_source[1] , ethern[v]->h_source[2] , ethern[v]->h_source[3] , ethern[v]->h_source[4] , ethern[v]->h_source[5],Sour[v],Dest[v],por1[v],por2[v]);
                usleep(1e5);
            }   
        }   
    }
    else
    {
        printf("Enter the file name(must have extension .txt) : ");
        char a[50];
        scanf("%s",a);
        flog = fopen(a,"w");
        ethernet *eptr;
        ip_protocol* ipptr;
        udp_protocol* udpptr;
        tcp_protocol* tcpptr;
        if(ch1==1 && ch2==1)
        {
            for(i=0;i<tcp;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    tcpptr = get_tcp_details(tcpthern[v]);
                    print_tcp_details(tcpptr);
                    fprintf(flog,"######################################");
                }
            }
        }
        else if(ch1==1&&ch2==2)
        {
            
            for(i=0;i<udp;i++)
            {
                int v=udp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    udpptr = get_udp_details(udpthern[v]);
                    print_udp_details(udpptr);
                    fprintf(flog,"######################################");   
                }
            }   
        }
        else if(ch1==1&&ch2==3)
        {
            for(i=0;i<udp;i++)
            {
                int v=udp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    udpptr = get_udp_details(udpthern[v]);
                    print_udp_details(udpptr);
                    fprintf(flog,"######################################");
                }
            }
            for(i=0;i<tcp;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==80||por1[v]==443||por2[v]==80||por2[v]==443)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    tcpptr = get_tcp_details(tcpthern[v]);
                    print_tcp_details(tcpptr);
                    fprintf(flog,"######################################");
                }
            } 
        }
        if(ch1==2&&ch2==1)
        {
            for(i=0;i<tcp;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    tcpptr = get_tcp_details(tcpthern[v]);
                    print_tcp_details(tcpptr);
                    fprintf(flog,"######################################");
                }
            }
        }
        else if(ch1==2&&ch2==2)
        {
            
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    udpptr = get_udp_details(udpthern[v]);
                    print_udp_details(udpptr);
                    fprintf(flog,"######################################");
                }
            }
        }
        else if(ch1==2&&ch2==3)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                     eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    udpptr = get_udp_details(udpthern[v]);
                    print_udp_details(udpptr);
                    fprintf(flog,"######################################");
                }
            }
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                if(por1[v]==5353||por2[v]==5353)    
                {
                    eptr = get_ether_details(ethern[v]); 
                    print_ether_details(eptr);
                    ipptr = get_ip_details(ipthern[v]);
                    print_ip_details(ipptr);
                    tcpptr = get_tcp_details(tcpthern[v]);
                    print_tcp_details(tcpptr);
                    fprintf(flog,"######################################");
                }
            }
        }
        else if(ch1==3&&ch2==1)
        {
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                eptr = get_ether_details(ethern[v]); 
                print_ether_details(eptr);
                ipptr = get_ip_details(ipthern[v]);
                print_ip_details(ipptr);
                tcpptr = get_tcp_details(tcpthern[v]);
                print_tcp_details(tcpptr);
                fprintf(flog,"######################################");
            }
        }
        else if(ch1==3&&ch2==2)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                eptr = get_ether_details(ethern[v]); 
                print_ether_details(eptr);
                ipptr = get_ip_details(ipthern[v]);
                print_ip_details(ipptr);
                udpptr = get_udp_details(udpthern[v]);
                print_udp_details(udpptr);
                fprintf(flog,"######################################");
            }
        }
        else if(ch1==3&&ch2==3)
        {
            for(i=0;i<udp_count;i++)
            {
                int v=udp_index[i];
                eptr = get_ether_details(ethern[v]); 
                print_ether_details(eptr);
                ipptr = get_ip_details(ipthern[v]);
                print_ip_details(ipptr);
                udpptr = get_udp_details(udpthern[v]);
                print_udp_details(udpptr);
                fprintf(flog,"######################################");
            }
            for(i=0;i<tcp_count;i++)
            {
                int v=tcp_index[i];
                eptr = get_ether_details(ethern[v]); 
                print_ether_details(eptr);
                ipptr = get_ip_details(ipthern[v]);
                print_ip_details(ipptr);
                tcpptr = get_tcp_details(tcpthern[v]);
                print_tcp_details(tcpptr);
                fprintf(flog,"######################################");
            }   
        }
        fclose(flog);
        free(eptr);
        free(ipptr);
        free(udpptr);
    }
}
