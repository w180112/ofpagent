/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
   OFP_SOCK.H
 
 *\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#ifndef _OFP_SOCK_H_
#define _OFP_SOCK_H_

#ifdef __cplusplus  
extern "C" {
#endif   

extern void           drv_xmit(U8 *mu, U16 mulen);
extern int 						OFP_SOCK_INIT();
extern void 					ofp_sockd();
extern STATUS 			  ofp_send2mailbox(U8 *mu, int mulen);

#ifdef __cplusplus
}
#endif

#endif 
