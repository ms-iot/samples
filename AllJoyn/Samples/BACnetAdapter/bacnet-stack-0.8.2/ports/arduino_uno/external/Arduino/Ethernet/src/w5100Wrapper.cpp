/*
 * w5100Wrapper.cpp
 *
 *  Created on: 26 de Mai de 2013
 *      Author: mgf
 */


#include "w5100.h"
#include "w5100Wrapper.h"

extern "C"{

CSnMR * CSnMR_new(){
	SnMR* s = new SnMR();
	return (CSnMR*)s;
}
void CSnMR_delete(const CSnMR* obj){
	SnMR* s = (SnMR*)obj;
	delete s;
}
uint8_t SnMR_CLOSE(){ return SnMR::CLOSE; }
uint8_t SnMR_UDP(){ return SnMR::UDP; }
uint8_t SnMR_TCP(){ return SnMR::TCP; }
uint8_t SnMR_IPRAW(){ return SnMR::IPRAW; }
uint8_t SnMR_MACRAW(){ return SnMR::MACRAW; }
uint8_t SnMR_PPPOE(){ return SnMR::PPPOE; }
uint8_t SnMR_ND(){ return SnMR::ND; }
uint8_t SnMR_MULTI(){ return SnMR::MULTI; }

CSnIR * CSnIR_new(){
	SnIR* s =  new SnIR();
	return (CSnIR*) s;
}
void CSnIR_delete(const CSnIR* obj){
	SnIR* s = (SnIR*)obj;
	delete s;
}
uint8_t SnIR_SEND_OK(){ return SnIR::SEND_OK; }
uint8_t SnIR_TIMEOUT(){ return SnIR::TIMEOUT; }
uint8_t SnIR_RECV(){ return SnIR::RECV; }
uint8_t SnIR_DISCON(){ return SnIR::DISCON; }
uint8_t SnIR_CON(){ return SnIR::CON; }

CSnSR * CSnSR_new(){
	SnSR* s = new SnSR();
	return (CSnSR*) s;
}
void CSnSR_delete(const CSnSR* obj){
	SnSR* s = (SnSR*)obj;
	delete s;
}
uint8_t SnSR_CLOSED(){ return SnSR::CLOSED; }
uint8_t SnSR_INIT(){ return SnSR::INIT; }
uint8_t SnSR_LISTEN(){ return SnSR::LISTEN; }
uint8_t SnSR_SYNSENT(){ return SnSR::SYNSENT; }
uint8_t SnSR_SYNRECV(){ return  SnSR::SYNRECV; }
uint8_t SnSR_ESTABLISHED(){ return SnSR::ESTABLISHED; }
uint8_t SnSR_FIN_WAIT(){ return SnSR::FIN_WAIT; }
uint8_t SnSR_CLOSING(){ return SnSR::CLOSING; }
uint8_t SnSR_TIME_WAIT(){ return SnSR::TIME_WAIT; }
uint8_t SnSR_CLOSE_WAIT(){ return SnSR::CLOSE_WAIT; }
uint8_t SnSR_LAST_ACK(){ return SnSR::LAST_ACK; }
uint8_t SnSR_UDP(){ return SnSR::UDP; }
uint8_t SnSR_IPRAW(){ return SnSR::IPRAW; }
uint8_t SnSR_MACRAW(){ return SnSR::MACRAW; }
uint8_t SnSR_PPPOE(){ return SnSR::PPPOE; }

CIPPROTO * CIPPROTO_new(){
	IPPROTO* i = new IPPROTO();
	return (CIPPROTO*) i;
}
void CIPPROTO_delete(const CIPPROTO* obj){
	IPPROTO* i = (IPPROTO*) obj;
	delete i;
}
uint8_t IPPROTO_IP(){ return IPPROTO::IP; }
uint8_t IPPROTO_ICMP(){ return IPPROTO::ICMP; }
uint8_t IPPROTO_IGMP(){ return IPPROTO::IGMP; }
uint8_t IPPROTO_GGP(){ return IPPROTO::GGP; }
uint8_t IPPROTO_TCP(){ return IPPROTO::TCP; }
uint8_t IPPROTO_PUP(){ return IPPROTO::PUP; }
uint8_t IPPROTO_UDP(){ return IPPROTO::UDP; }
uint8_t IPPROTO_IDP(){ return IPPROTO::IDP; }
uint8_t IPPROTO_ND(){ return IPPROTO::ND; }
uint8_t IPPROTO_RAW(){ return IPPROTO::RAW; }

CW5100Class * CW5100Class_new(){
	return (CW5100Class*) &W5100;
}

void init_func(const CW5100Class * obj){
	W5100Class* w = (W5100Class*) obj;
	w->init();
}

void read_data_func(const CW5100Class * obj, SOCKET s, volatile uint8_t * src, volatile uint8_t * dst,
					uint16_t len){
	W5100Class* w = (W5100Class*) obj;
	w->read_data(s, src, dst, len);
}

void send_data_processing_func(const CW5100Class * obj, SOCKET s, const uint8_t *data, uint16_t len){
	W5100Class* w = (W5100Class*) obj;
	w->send_data_processing(s, data, len);
}
void send_data_processing_offset_func(const CW5100Class * obj, SOCKET s, uint16_t data_offset,
									  const uint8_t *data, uint16_t len){
	W5100Class* w = (W5100Class*) obj;
	w->send_data_processing_offset(s, data_offset, data, len);
}
//FIXME: Removed defaul value of 0(zero) from the peek argument
void recv_data_processing_func(const CW5100Class * obj, SOCKET s, uint8_t *data, uint16_t len,
							   uint8_t peek){
	W5100Class* w = (W5100Class*) obj;
	w->recv_data_processing(s, data, len, peek);
}
void setGatewayIp_func(const CW5100Class * obj, uint8_t *_addr){
	W5100Class* w = (W5100Class*) obj;
	w->setGatewayIp(_addr);
}
void getGatewayIp_func(const CW5100Class * obj, uint8_t *_addr){
	W5100Class* w = (W5100Class*) obj;
	w->getGatewayIp(_addr);
}

void setSubnetMask_func(const CW5100Class * obj, uint8_t *_addr){
	W5100Class* w = (W5100Class*) obj;
	w->setSubnetMask(_addr);
}
void getSubnetMask_func(const CW5100Class * obj, uint8_t *_addr){
	W5100Class* w = (W5100Class*) obj;
	w->getSubnetMask(_addr);
}

void setMACAddress_func(const CW5100Class * obj, uint8_t * addr){
	W5100Class* w = (W5100Class*) obj;
	w->setMACAddress(addr);
}
void getMACAddress_func(const CW5100Class * obj, uint8_t * addr){
	W5100Class* w = (W5100Class*) obj;
	w->getMACAddress(addr);
}
void setIPAddress_func(const CW5100Class * obj, uint8_t * addr){
	W5100Class* w = (W5100Class*) obj;
	w->setIPAddress(addr);
}
void getIPAddress_func(const CW5100Class * obj, uint8_t * addr){
	W5100Class* w = (W5100Class*) obj;
	w->getIPAddress(addr);
}
void setRetransmissionTime_func(const CW5100Class * obj, uint16_t timeout){
	W5100Class* w = (W5100Class*) obj;
	w->setRetransmissionTime(timeout);
}
void setRetransmissionCount_func(const CW5100Class * obj, uint8_t _retry){
	W5100Class* w = (W5100Class*) obj;
	w->setRetransmissionCount(_retry);
}

uint16_t getTXFreeSize_func(const CW5100Class * obj,SOCKET s){
	W5100Class* w = (W5100Class*) obj;
	return w->getTXFreeSize(s);
}
uint16_t getRXReceivedSize_func(const CW5100Class * obj,SOCKET s){
	W5100Class* w = (W5100Class*) obj;
	return w->getRXReceivedSize(s);
}

uint8_t readSnSR_func(const CW5100Class* obj, SOCKET s){
	W5100Class* w = (W5100Class*) obj;
	return w->readSnSR(s);
}

}//externC

