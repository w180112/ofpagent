/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
  ofp_sock.c
  
  Created by Dennis Tseng on June 22,'11
\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#include       	<common.h>
#include		<sys/socket.h>
#include 		<sys/types.h> 
#include 		<netinet/in.h>
#include        "ofp_sock.h"
#include		"ofpd.h"

struct ifreq	ethreq;
int				ofpSockSize = sizeof(struct sockaddr_in);
int				ofp_io_fds[10];
fd_set			ofp_io_ready[2];

/**************************************************************************
 * OFP_SOCK_INIT :
 *
 * Some useful methods :
 * - from string to ulong
 *   inet_aton("10.5.5.217", (struct in_addr*)cSA_ip); 
 **************************************************************************/ 
int OFP_SOCK_INIT() 
{
	struct sockaddr_in sock_info[2], local_sock_info[2];

    if ((ofp_io_fds[0]=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    printf("socket");
	    return -1;
	}
	
	if ((ofp_io_fds[1]=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    printf("socket");
	    return -1;
	}

    FD_ZERO(&ofp_io_ready[0]);
    FD_SET(ofp_io_fds[0],&ofp_io_ready[0]);

	FD_ZERO(&ofp_io_ready[1]);
    FD_SET(ofp_io_fds[1],&ofp_io_ready[1]);
    
	bzero(&sock_info[0], sizeof(sock_info[0]));
    sock_info[0].sin_family = PF_INET;
    sock_info[0].sin_addr.s_addr = inet_addr("192.168.10.171");
    sock_info[0].sin_port = htons(6653);
    
	bzero(&local_sock_info[0], sizeof(local_sock_info[0]));
    local_sock_info[0].sin_family = PF_INET;
    local_sock_info[0].sin_addr.s_addr = inet_addr("192.168.10.156");
    local_sock_info[0].sin_port = htons(6654);

	bzero(&local_sock_info[1], sizeof(local_sock_info[1]));
    local_sock_info[1].sin_family = PF_INET;
    local_sock_info[1].sin_addr.s_addr = inet_addr("192.168.10.156");
    local_sock_info[1].sin_port = htons(6655);

	bind(ofp_io_fds[0],(struct sockaddr *)&local_sock_info[0],sizeof(local_sock_info[0]));
	bind(ofp_io_fds[1],(struct sockaddr *)&local_sock_info[1],sizeof(local_sock_info[1]));

    int err = connect(ofp_io_fds[0],(struct sockaddr *)&sock_info,sizeof(sock_info));
    if (err == -1) {
        printf("Connection error.");
    }

	return 0;
}

/**************************************************************************
 * ofp_sockd_cp:
 *
 * iov structure will provide the memory, so parameter pBuf doesn't need to care it.
 **************************************************************************/
void ofp_sockd_cp(void)
{
	int		n,rxlen;
	U8 	    buffer[ETH_MTU];
		
	/* 
	** to.tv_sec = 1;  ie. non-blocking; "select" will return immediately; =polling 
    ** to.tv_usec = 0; ie. blocking
    */
	for(;;) {    
		if ((n=select(ofp_io_fds[0]+1,&ofp_io_ready[0],(fd_set*)0,(fd_set*)0,NULL/*&to*/)) < 0){
   		    /* if "to" = NULL, then "select" will block indefinite */
   			printf("select error !!! Giveup this receiving.\n");
   			sleep(2);
   			continue;
   		}
		if (n == 0)  continue;
		
   		/*----------------------------------------------------------------------
       	 * rx data from "LOC_sockAddr" to "LOC_fd" in Blocking mode
     	 *---------------------------------------------------------------------*/
    	if (FD_ISSET(ofp_io_fds[0],&ofp_io_ready[0])){
    		rxlen = recv(ofp_io_fds[0],(char*)buffer,1500,0);
    		if (rxlen <= 0){
      			printf("Error! recvfrom(): len <= 0\n");
       			continue;
    		}
    		
   			/*printf("=========================================================\n");
			printf("rxlen=%d\n",rxlen);
    		PRINT_MESSAGE((char*)buffer, rxlen);*/
    		
    		ofp_send2mailbox((U8*)buffer, rxlen);
   		} /* if select */
   	} /* for */
}

/**************************************************************************
 * ofp_sockd_dp:
 *
 * iov structure will provide the memory, so parameter pBuf doesn't need to care it.
 **************************************************************************/
void ofp_sockd_dp(void)
{
	int		n,rxlen;
	U8 	    buffer[ETH_MTU];
	struct sockaddr_in client;              
	int addrlen = sizeof(client), client_fd;
		
	listen(ofp_io_fds[1],10);
	for(;;) {
		client_fd = accept(ofp_io_fds[1],(struct sockaddr *)&client, (socklen_t *)&addrlen);
    	rxlen = recv(client_fd,(char*)buffer,1500,0);
    	if (rxlen <= 0) {
    		printf("Error! recv(): len <= 0\n");
       		continue;
		}
    		
   			/*printf("=========================================================\n");
			printf("rxlen=%d\n",rxlen);
    		PRINT_MESSAGE((char*)buffer, rxlen);*/
    		
    	ofp_send2mailbox((U8*)buffer, rxlen);
	}
}

/**************************************************************
 * drv_xmit :
 *
 * - sid  : socket id
 **************************************************************/
void drv_xmit(U8 *mu, U16 mulen, int fd_id)
{
	printf("\ndrv_xmit ............\n");
	PRINT_MESSAGE((unsigned char*)mu, mulen);
	send(ofp_io_fds[fd_id], mu, mulen, 0);
}

/*********************************************************
 * ofp_send2mailbox:
 *
 * Input  : mu - incoming ethernet+igmp message unit
 *          mulen - mu length
 *          port - 0's based
 *          sid - need tag ? (Y/N)
 *          prior - if sid=1, then need set this field
 *          vlan - if sid=1, then need set this field
 *
 * return : TRUE or ERROR(-1)
 *********************************************************/
STATUS ofp_send2mailbox(U8 *mu, int mulen)
{
	tOFP_MBX mail;

    if (ofpQid == -1){
		if ((ofpQid=msgget(OFP_Q_KEY,0600|IPC_CREAT)) < 0){
			printf("send> Oops! ofpQ(key=0x%x) not found\n",OFP_Q_KEY);
   	 	}
	}
	
	if (mulen > ETH_MTU){
	 	printf("Incoming frame length(%d) is too lmaile!\n",mulen);
		return ERROR;
	}

	mail.len = mulen;
	memcpy(mail.refp,mu,mulen); /* mail content will be copied into mail queue */
	
	//printf("ofp_send2mailbox(ofp_sock.c %d): mulen=%d\n",__LINE__,mulen);
	mail.type = IPC_EV_TYPE_DRV;
	ipc_sw(ofpQid, &mail, sizeof(mail), -1);
	return TRUE;
}
