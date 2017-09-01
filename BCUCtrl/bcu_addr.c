#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>

//#include <features.h>
//#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
//#include <netpacket/packet.h>
//#include <net/ethernet.h>     /* the L2 protocols */
//#else
//#include <asm/types.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>
//#endif

#define ETH_FRAME_LEN 1400
#define ETH_HEADER_LEN 14

int main( int argc, char **argv)
{
	int i,s, result;
	struct sockaddr_ll socket_address;
	void *buffer;
	unsigned char *etherhead, *data;
	struct ethhdr *eh;
	unsigned int dest[6];
	unsigned char src_mac[6] = { 0x00, 0x01, 0x02, 0xFA, 0x70, 0xAA};
	unsigned char dest_mac[6] = { 0x00, 0x04, 0x75, 0xC8, 0x28, 0xE5};


	if (argc != 3)
		{
		printf("Usage: bcu_addr <bcu MAC address> <new bcu IP addr>\n\n");
		exit(0);
		}

	sscanf( argv[1], "%x:%x:%x:%x:%x:%x", dest, dest+1, dest+2, dest+3, dest+4, dest+5);

	for (i=0; i<6; i++)
		dest_mac[i] = (unsigned char)dest[i];

	s = socket( PF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );	
	if (s ==-1)
		perror("socket()");

	// Various buffer pointers

	buffer = malloc(ETH_FRAME_LEN);
	etherhead = buffer;
	data = buffer + ETH_HEADER_LEN;
	eh = (struct ethhdr *)etherhead;

	socket_address.sll_family = PF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_IP);

	/* index of the network device */
	socket_address.sll_ifindex =2;
	socket_address.sll_hatype = ARPHRD_ETHER;
	socket_address.sll_pkttype = PACKET_OTHERHOST;
	socket_address.sll_halen = ETH_ALEN;

	memcpy( socket_address.sll_addr, dest_mac, 6);
	socket_address.sll_addr[6] = 0x00;
	socket_address.sll_addr[7] = 0x00;

	/* Set the frame header */
	memcpy( buffer, dest_mac, ETH_ALEN);	
	memcpy( buffer+ETH_ALEN, src_mac, ETH_ALEN);
	eh->h_proto = 0x00;

	/* User data */
	memset( data, 0, ETH_FRAME_LEN - ETH_HEADER_LEN);
	memcpy( data, "test", 4);

	result = sendto( s, buffer, ETH_FRAME_LEN, 0,
			(struct sockaddr *)&socket_address, sizeof(socket_address));
	if (result == -1)
		perror("Sendto()");

	return 0; 

}
