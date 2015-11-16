/*
 * w5100Wrapper.h
 *
 *  Created on: 26 de Mai de 2013
 *      Author: mgf
 */

#ifndef W5100WRAPPER_H_
#define W5100WRAPPER_H_

#include <avr/pgmspace.h>

typedef uint8_t SOCKET;
typedef void CSnMR;
typedef void CSnIR;
typedef void CSnSR;
typedef void CIPPROTO;
typedef void CW5100Class;

#define MAX_SOCK_NUM 4

#ifdef __cplusplus
extern "C" {
#endif

    CSnMR *CSnMR_new();
    void CSnMR_delete(const CSnMR * obj);
    uint8_t SnMR_CLOSE();
    uint8_t SnMR_UDP();
    uint8_t SnMR_TCP();
    uint8_t SnMR_IPRAW();
    uint8_t SnMR_MACRAW();
    uint8_t SnMR_PPPOE();
    uint8_t SnMR_ND();
    uint8_t SnMR_MULTI();

    CSnIR *CSnIR_new();
    void CSnIR_delete(const CSnIR * obj);
    uint8_t SnIR_SEND_OK();
    uint8_t SnIR_TIMEOUT();
    uint8_t SnIR_RECV();
    uint8_t SnIR_DISCON();
    uint8_t SnIR_CON();


    CSnSR *CSnSR_new();
    void CSnSR_delete(const CSnSR * obj);
    uint8_t SnSR_CLOSED();
    uint8_t SnSR_INIT();
    uint8_t SnSR_LISTEN();
    uint8_t SnSR_SYNSENT();
    uint8_t SnSR_SYNRECV();
    uint8_t SnSR_ESTABLISHED();
    uint8_t SnSR_FIN_WAIT();
    uint8_t SnSR_CLOSING();
    uint8_t SnSR_TIME_WAIT();
    uint8_t SnSR_CLOSE_WAIT();
    uint8_t SnSR_LAST_ACK();
    uint8_t SnSR_UDP();
    uint8_t SnSR_IPRAW();
    uint8_t SnSR_MACRAW();
    uint8_t SnSR_PPPOE();

    CIPPROTO *CIPPROTO_new();
    void CIPPROTO_delete(const CIPPROTO * obj);
    uint8_t IPPROTO_IP();
    uint8_t IPPROTO_ICMP();
    uint8_t IPPROTO_IGMP();
    uint8_t IPPROTO_GGP();
    uint8_t IPPROTO_TCP();
    uint8_t IPPROTO_PUP();
    uint8_t IPPROTO_UDP();
    uint8_t IPPROTO_IDP();
    uint8_t IPPROTO_ND();
    uint8_t IPPROTO_RAW();

    CW5100Class *CW5100Class_new();
    void init_func(const CW5100Class * obj);
    void CW5100Class_delete(const CW5100Class * obj);

    void read_data_func(const CW5100Class * obj,
        SOCKET s,
        volatile uint8_t * src,
        volatile uint8_t * dst,
        uint16_t len);

    void send_data_processing_func(const CW5100Class * obj,
        SOCKET s,
        const uint8_t * data,
        uint16_t len);
    void send_data_processing_offset_func(const CW5100Class * obj,
        SOCKET s,
        uint16_t data_offset,
        const uint8_t * data,
        uint16_t len);
//FIXME: Removed defaul value of 0(zero) from the peek argument
    void recv_data_processing_func(const CW5100Class * obj,
        SOCKET s,
        uint8_t * data,
        uint16_t len,
        uint8_t peek);
    void setGatewayIp_func(const CW5100Class * obj,
        uint8_t * _addr);
    void getGatewayIp_func(const CW5100Class * obj,
        uint8_t * _addr);
//
    void setSubnetMask_func(const CW5100Class * obj,
        uint8_t * _addr);
    void getSubnetMask_func(const CW5100Class * obj,
        uint8_t * _addr);
//
    void setMACAddress_func(const CW5100Class * obj,
        uint8_t * addr);
    void getMACAddress_func(const CW5100Class * obj,
        uint8_t * addr);
//
    void setIPAddress_func(const CW5100Class * obj,
        uint8_t * addr);
    void getIPAddress_func(const CW5100Class * obj,
        uint8_t * addr);
//
    void setRetransmissionTime_func(const CW5100Class * obj,
        uint16_t timeout);
    void setRetransmissionCount_func(const CW5100Class * obj,
        uint8_t _retry);
//

    uint16_t getTXFreeSize_func(const CW5100Class * obj,
        SOCKET s);
    uint16_t getRXReceivedSize_func(const CW5100Class * obj,
        SOCKET s);

    uint8_t readSnSR_func(const CW5100Class * obj,
        SOCKET s);

#ifdef __cplusplus
}
#endif
#endif /* W5100WRAPPER_H_ */
