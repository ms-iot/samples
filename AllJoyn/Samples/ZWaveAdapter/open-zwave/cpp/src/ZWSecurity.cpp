//-----------------------------------------------------------------------------
//
//	Security.cpp
//
//	Common Security/Encryption Routines
//
//	Copyright (c) 2015 Justin Hammond <justin@dynam.ac>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include "Defs.h"
#include "ZWSecurity.h"
#include "Node.h"
#include "Driver.h"
#include "Manager.h"
#include "Options.h"
#include "Utils.h"
#include "platform/Log.h"
#include "command_classes/MultiInstance.h"
#include "command_classes/Security.h"
#include "aes/aescpp.h"


namespace OpenZWave {
	//using namespace OpenZWave;

	//-----------------------------------------------------------------------------
	// <GenerateAuthentication>
	// Generate authentication data from a security-encrypted message
	//-----------------------------------------------------------------------------
	bool GenerateAuthentication
	(
			uint8 const* _data,				// Starting from the command class command
			uint32 const _length,
			Driver *driver,
			uint8 const _sendingNode,
			uint8 const _receivingNode,
			uint8 *iv,
			uint8* _authentication			// 8-byte buffer that will be filled with the authentication data
	)
	{
		// Build a buffer containing a 4-byte header and the encrypted
		// message data, padded with zeros to a 16-byte boundary.
		uint8 buffer[256];
		uint8 tmpauth[16];
		memset(buffer, 0, 256);
		memset(tmpauth, 0, 16);
		buffer[0] = _data[0];							// Security command class command
		buffer[1] = _sendingNode;
		buffer[2] = _receivingNode;
		buffer[3] = _length - 19; // Subtract 19 to account for the 9 security command class bytes that come before and after the encrypted data
		memcpy( &buffer[4], &_data[9], _length-19 );	// Encrypted message

		uint8 bufsize = _length - 19 + 4; /* the size of buffer */
#ifdef DEBUG
		PrintHex("Raw Auth (minus IV)", buffer, bufsize);
		Log::Write(LogLevel_Debug, _receivingNode, "Raw Auth (Minus IV) Size: %d (%d)", bufsize, bufsize+16);
#endif

		aes_mode_reset(driver->GetAuthKey());
		/* encrypt the IV with ecb */
		if (aes_ecb_encrypt(iv, tmpauth, 16, driver->GetAuthKey()) == EXIT_FAILURE) {
			Log::Write(LogLevel_Warning, _receivingNode, "Failed Initial ECB Encrypt of Auth Packet");
			return false;
		}

		/* our temporary holding var */
		uint8 encpck[16];

		int block = 0;
		/* reset our encpck temp var */
		memset(encpck, 0, 16);
		/* now xor the buffer with our encrypted IV */
		for (int i = 0; i < bufsize; i++) {
			encpck[block] = buffer[i];
			block++;
			/* if we hit a blocksize, then encrypt */
			if (block == 16) {
				for (int j = 0; j < 16; j++) {
					/* here we do our xor */
					tmpauth[j] = encpck[j] ^ tmpauth[j];
					/* and reset encpck for good measure */
					encpck[j] = 0;
				}
				/* reset our block counter back to 0 */
				block = 0;
				aes_mode_reset(driver->GetAuthKey());
				if (aes_ecb_encrypt(tmpauth, tmpauth, 16, driver->GetAuthKey()) == EXIT_FAILURE) {
					Log::Write(LogLevel_Warning, _receivingNode, "Failed Subsequent (%d) ECB Encrypt of Auth Packet", i);
					return false;
				}
			}
		}
		/* any left over data that isn't a full block size*/
		if (block > 0) {
			for (int i= 0; i < 16; i++) {
				/* encpck from block to 16 is already gauranteed to be 0
				 * so its safe to xor it with out tmpmac */
				tmpauth[i] = encpck[i] ^ tmpauth[i];
			}
			aes_mode_reset(driver->GetAuthKey());
			if (aes_ecb_encrypt(tmpauth, tmpauth, 16, driver->GetAuthKey()) == EXIT_FAILURE) {
				Log::Write(LogLevel_Warning, _receivingNode, "Failed Final ECB Encrypt of Auth Packet");
				return false;
			}
		}
		/* we only care about the first 8 bytes of tmpauth as the mac */
#ifdef DEBUG
		PrintHex("Computed Auth", tmpauth, 8);
#endif
		/* so only copy 8 bytes to the _authentication var */
		memcpy(_authentication, tmpauth, 8);
		return true;
	}

	bool EncyrptBuffer(
			uint8 *m_buffer,
			uint8 m_length,
			Driver *driver,
			uint8 const _sendingNode,
			uint8 const _receivingNode,
			uint8 const m_nonce[8],
			uint8* e_buffer
	)
	{

#if 0
		m_nonce[0] = 0x09;
		m_nonce[1] = 0x0d;
		m_nonce[2] = 0x93;
		m_nonce[3] = 0xd3;
		m_nonce[4] = 0x61;
		m_nonce[5] = 0x61;
		m_nonce[6] = 0x1d;
		m_nonce[7] = 0xd6;
#endif
		uint8 len = 0;
		e_buffer[len++] = SOF;
		e_buffer[len++] = m_length + 18; // length of full packet
		e_buffer[len++] = REQUEST;
		e_buffer[len++] = FUNC_ID_ZW_SEND_DATA;
		e_buffer[len++] = _receivingNode;
		e_buffer[len++] = m_length + 11; 					// Length of the payload
		e_buffer[len++] = Security::StaticGetCommandClassId();
		e_buffer[len++] = SecurityCmd_MessageEncap;

		/* create our IV */
		uint8 initializationVector[16];
		/* the first 8 bytes of a outgoing IV are random
		 * and we add it also to the start of the payload
		 */
		for (int i = 0; i < 8; i++) {
			//initializationVector[i] = (rand()%0xFF)+1;
			initializationVector[i] = 0xAA;
			e_buffer[len++] = initializationVector[i];
		}
		/* the remaining 8 bytes are the NONCE we got from the device */
		for (int i = 0; i < 8; i++) {
			initializationVector[8+i] = m_nonce[i];
		}


		uint8 plaintextmsg[32];
		/* add the Sequence Flag
		 * - Since we dont currently handle multipacket encryption
		 * just set this to 0
		 */
		plaintextmsg[0] = 0;
		/* now add the actual message to be encrypted */
		for (int i = 0; i < m_length-6-3; i++)
			plaintextmsg[i+1] = m_buffer[6+i];

		/* now encrypt */
		uint8 encryptedpayload[30];
		aes_mode_reset(driver->GetEncKey());
#ifdef DEBUG
		PrintHex("Plain Text Packet:", plaintextmsg, m_length-5-3);
#endif
		if (aes_ofb_encrypt(plaintextmsg, encryptedpayload, m_length-5-3, initializationVector, driver->GetEncKey()) == EXIT_FAILURE) {
			Log::Write(LogLevel_Warning, _receivingNode, "Failed to Encrypt Packet");
			return false;
		}
#ifdef DEBUG
		PrintHex("Encrypted Packet", encryptedpayload, m_length-5-3);
#endif
		/* now add the Encrypted output to the packet */
		for (int i = 0; i < m_length-5-3; i++) {
			e_buffer[len++] = encryptedpayload[i];
		}

		// Append the nonce identifier :)
		e_buffer[len++] = m_nonce[0];


		/* regenerate the IV */
		for (int i = 0; i < 8; i++) {
			//initializationVector[i] = (rand()%0xFF)+1;
			initializationVector[i] = 0xAA;
		}
		/* the remaining 8 bytes are the NONCE we got from the device */
		for (int i = 0; i < 8; i++) {
			initializationVector[8+i] = m_nonce[i];
		}

		/* now calculate the MAC and append it */
		uint8 mac[8];
		GenerateAuthentication(&e_buffer[7], e_buffer[5], driver, _sendingNode, _receivingNode, initializationVector, mac);
		for(int i=0; i<8; ++i )
		{
			e_buffer[len++] = mac[i];
		}

		e_buffer[len++] = driver->GetTransmitOptions();
		/* this is the same as the Actual Message */
		e_buffer[len++] = m_buffer[m_length-2];
		// Calculate the checksum
		uint8 csum = 0xff;
		for( int32 i=1; i<len; ++i )
		{
			csum ^= e_buffer[i];
		}
		e_buffer[len++] = csum;
		return true;
	}

	bool createIVFromPacket_inbound(uint8 const* _data, uint8 const m_nonce[8], uint8 *iv) {

		for (int i = 0; i < 8; i++) {
			iv[i] = _data[i];
		}
		for (int i = 0; i < 8; i++) {
			iv[8+i] = m_nonce[i];
		}
		return true;
	}

	/* To Decrypt, we start the packet at the IV (right after the command)
	 *
	 * Encrypted Packet Size is Packet Lenght - Device Nonce(8) - Reciever Nonce ID (1) - Mac (8) - CommandClass - Command
	 *
	 * Reciever Nonce is at Position 14 + Encrypted Packet Size
	 * Mac is at Position 15 + Encrypted Packet Size
	 * 0 - Command Class
	 * 1 - Command
	 * 2 to 9 - Device Nonce
	 * 10 - Sequence (e)
	 * 11 - Command Class (e)
	 * 12 - Command (e)
	 * 13 to EncryptedPckSize
	 * ReciverNonceID (1 Byte)
	 * Mac (8 Bytes)
	 */



	bool DecryptBuffer
	(
			uint8 *e_buffer,
			uint8 e_length,
			Driver *driver,
			uint8 const _sendingNode,
			uint8 const _receivingNode,
			uint8 const m_nonce[8],
			uint8* m_buffer
	)
	{
		PrintHex("Raw", e_buffer, e_length);

		if (e_length < 19) {
			Log::Write(LogLevel_Warning, _sendingNode, "Recieved a Encrypted Message that is too Short. Dropping it");
			return false;
		}


		uint8 iv[17];
		createIVFromPacket_inbound(&e_buffer[2], m_nonce, iv); /* first 8 bytes of Packet are the Random Value generated by the Device
		 * 2nd 8 bytes of the IV are our nonce we sent previously
		 */
		memset(&m_buffer[0], 0, 32);
		uint32 encryptedpacketsize = e_length - 8 - 8 - 2 - 2;

		/* if the Encrypted Packet Size is less than 3, there is probably a issue, drop it. */
		if (encryptedpacketsize < 3) {
			Log::Write(LogLevel_Warning, _sendingNode, "Encrypted Packet Size is Less than 3 Bytes. Dropping");
			return false;
		}


		uint8 encyptedpacket[32];

		for (uint32 i = 0; i < 32; i++) {
			if (i >= encryptedpacketsize) {
				/* pad the remaining fields */
				encyptedpacket[i] = 0;
			} else {
				encyptedpacket[i] = e_buffer[10+i];
			}
		}


#ifdef DEBUG
		Log::Write(LogLevel_Debug, _sendingNode, "Encrypted Packet Sizes: %d (Total) %d (Payload)", e_length, encryptedpacketsize);
		PrintHex("IV", iv, 16);
		PrintHex("Encrypted", encyptedpacket, 16);
		/* Mac Starts after Encrypted Packet. */
		PrintHex("Auth", &e_buffer[11+encryptedpacketsize], 8);
#endif
		aes_mode_reset(driver->GetEncKey());
#if 0
		uint8_t iv[16] = {  0x81, 0x42, 0xd1, 0x51, 0xf1, 0x59, 0x3d, 0x70, 0xd5, 0xe3, 0x6c, 0xcb, 0x02, 0xd0, 0x3f, 0x5c,  /* */  };
		uint8_t pck[] = {  0x25, 0x68, 0x06, 0xc5, 0xb3, 0xee, 0x2c, 0x17, 0x26, 0x7e, 0xf0, 0x84, 0xd4, 0xc3, 0xba, 0xed, 0xe5, 0xb9, 0x55};
		if (aes_ofb_decrypt(pck, decryptpacket, 19, iv, this->EncryptKey) == EXIT_FAILURE) {
			Log::Write(LogLevel_Warning, GetNodeId(), "Failed to Decrypt Packet");
			return false;
		}
		PrintHex("Pck", decryptpacket, 19);
#else
		if (aes_ofb_decrypt(encyptedpacket, m_buffer, encryptedpacketsize, iv, driver->GetEncKey()) == EXIT_FAILURE) {
			Log::Write(LogLevel_Warning, _sendingNode, "Failed to Decrypt Packet");
			return false;
		}
		Log::Write(LogLevel_Detail, _sendingNode, "Decrypted Packet: %s", PktToString(m_buffer, encryptedpacketsize).c_str());
#endif
		uint8 mac[32];
		/* we have to regenerate the IV as the ofb decryption routine will alter it. */
		createIVFromPacket_inbound(&e_buffer[2], m_nonce, iv);

		GenerateAuthentication(&e_buffer[1], e_length-1, driver, _sendingNode, _receivingNode, iv, mac);
		if (memcmp(&e_buffer[11+encryptedpacketsize], mac, 8) != 0) {
			Log::Write(LogLevel_Warning, _sendingNode, "MAC Authentication of Packet Failed. Dropping");
			return false;
		}
		/* XXX TODO: Check the Sequence Header Frame to see if this is the first part of a
		 * message, or 2nd part, or a entire message.
		 *
		 * I havn't actually seen a Z-Wave Message thats too big to fit in a encrypted message
		 * yet, so we will look at this if such a message actually exists!
		 */

		return true;
	}

	SecurityStrategy ShouldSecureCommandClass(uint8 CommandClass) {
		string securestrategy;
		Options::Get()->GetOptionAsString( "SecurityStrategy", &securestrategy );

		if (ToUpper(securestrategy) == "ESSENTIAL") {
			return SecurityStrategy_Essential;
		} else if (ToUpper(securestrategy) == "SUPPORTED") {
			return SecurityStrategy_Supported;
		} else if (ToUpper(securestrategy) == "CUSTOM") {
			string customsecurecc;
			Options::Get()->GetOptionAsString( "CustomSecuredCC", &customsecurecc);

			char* pos = const_cast<char*>(customsecurecc.c_str());
			while( *pos )
			{
				if (CommandClass == (uint8)strtol( pos, &pos, 16 )) {
					return SecurityStrategy_Supported;
				}
				if( (*pos) == ',' )
				{
					++pos;
				}
			}
		}
		return SecurityStrategy_Essential;
	}

}
