#include "utils.h"

int main(int argc, char const *argv[])
{
	printf(ANSI_COLOR_GREEN "--------------------------------------------------------------------------------------------\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_GREEN "                 Welcome to the packet capture and analysis utility\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_GREEN "--------------------------------------------------------------------------------------------\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_CYAN "\nYou can select any interface , track conversations , filter packets based on protocol(s) and even view graphical analysis\nChoose an appropriate interface to get started and then select the functionality you want from the menu.\n\n" ANSI_COLOR_RESET);

    // DISPLAYS ALL INTERFACES FOR PACKET CAPTURE
    show_interfaces();

    // BINDING INTERFACE TO SOCKET
    char intf[50];

    printf(ANSI_COLOR_RED "Enter the interface for packet capturing : " ANSI_COLOR_RESET);
    scanf("%s" , intf);

    // GETTING RAW SOCKET IN PROMISCOUS MODE
    int sock_raw = raw_init(intf);

    // flog FILE FOR PACKET CAPTURE
    char fname[50];

    int addr_size , data_size;
    struct sockaddr saddr;         
    unsigned char *buf = (unsigned char *) malloc(10000); 


    while(true)
    {
        cntdrop=0;
        for(i=0;i<MAX;i++)
        {
            ethern[i]=NULL;
            ipthern[i]=NULL;
            tcpthern[i]=NULL;
            udpthern[i]=NULL;
        }
        tcp = udp = http = dns = ip = total = k = 0;
        ip_count = tcp_count = udp_count = 0;
        cnt = cntdrop = 0;

        printf(ANSI_COLOR_YELLOW "Enter the number of packets you wish to capture (MAX -  ");  
        printf("%d ) : ",MAX);
        printf(ANSI_COLOR_RESET);
        int nop;
        scanf("%d",&nop);
        printf(ANSI_COLOR_YELLOW"Enter the name of log file (must have extension .txt) : "ANSI_COLOR_RESET);
        scanf("%s" , fname);
        flog = fopen(fname , "w");

       struct timeval  tv1, tv2;
       gettimeofday(&tv1, NULL);
        
        while(cnt < nop)
        {
            addr_size = sizeof saddr;
            data_size = recvfrom(sock_raw , buf , 10000 , 0 , &saddr , (socklen_t*)&addr_size);
            if(data_size < 0 )
            {
                printf("No packet received\n");
                return 1;
            }
            printf("Packets Received : %5d\r" , cnt+1);
            gettimeofday(&tv2, NULL);
            double val=(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +(double) (tv2.tv_sec - tv1.tv_sec);
            tm[cnt] = val;
            packlen[k++] = data_size;
            analyze(buf , data_size);
            if(a != orig)
            {
                tmdrop[cntdrop] = val;
                cntdrop++;
            }
        }
        printf("Packets captured written in %s\n" , fname);

        int choice;

        while(true)
        {
            printf(ANSI_COLOR_GREEN "\nANALYSIS : \n0. Quit analysis\n1. Track conversations between pair of IP addresses\n2. Apply protocol filters to packets\n3. Display protocol hierarchy\n4. Network traffic graph\n5. Packet length analysis\n6. Packet drop analysis\n\nChoice : " ANSI_COLOR_RESET);
            scanf("%d",&choice);
            if(choice==0)
                break;
            else if(choice == 1)   
               converse();
            else if(choice==2)
               filter();                       
            else if(choice == 3)
               protocol_hierarchy();
            else if(choice == 4)
               display_traffic();
            else if(choice == 5)
               display_packlen();
            else if(choice == 6)
               display_drop();            
            else
                printf(ANSI_COLOR_RED"Invalid Option"ANSI_COLOR_RESET);   
        }

        char c;
        printf("Do you want to capture more packets (Y/N) : ");
        scanf(" %c" , &c);
        for(i=0;i<cnt;i++)
        {
            if(ethern[i]!=NULL)
            {
                free(ethern[i]);
                ethern[i]=NULL;
            }
            if(ipthern[i]!=NULL)
            {
                free(ipthern[i]);
                ipthern[i]=NULL;
            } 
            if(tcpthern[i]!=NULL)
            {
                free(tcpthern[i]);
                tcpthern[i]=NULL;
            }
            if(udpthern[i]!=NULL)
            {
                free(udpthern[i]);
                udpthern[i]=NULL;
            }
        }
        if(c == 'n' || c == 'N')
            break;
        
    }

    close(sock_raw);
    return 0;
}
