/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "bacnet.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "dcc.h"
#include "ptransfer.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"


#if BAC_PRIVATE
/** @file s_upt.c  Send an Unconfirmed Private Transfer request. */
extern uint8_t invoke_id;
void Send_UnconfirmedPrivateTransfer(
    BACNET_ADDRESS * dest,
    BACNET_PRIVATE_TRANSFER_DATA * private_data,
		uint8_t protocal)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;
#if BAC_DCC
    if (!dcc_communication_enabled())	   
        return;
#endif
		
    datalink_get_my_address(&my_address,protocal);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[protocal][0], dest, &my_address,
        &npdu_data);
#if USB_HOST
	if(protocal == BAC_GSM)
	{
		pdu_len = 2;	
	}
#endif
    /* encode the APDU portion of the packet */
    len =
        ptransfer_ack_encode_apdu(&Handler_Transmit_Buffer[protocal][pdu_len],invoke_id,
        private_data);
    pdu_len += len;
	  
   	datalink_send_pdu(dest, &npdu_data, &Handler_Transmit_Buffer[protocal][0],pdu_len,protocal);	
	#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr,
            "Failed to Send UnconfirmedPrivateTransfer Request (%s)!\n",
            strerror(errno));
#endif
}





int Send_ConfirmedPrivateTransfer(
	BACNET_ADDRESS * dest,
	BACNET_PRIVATE_TRANSFER_DATA * private_data,
	uint8_t protocal)
{
	int  far len = 0;
	int  far pdu_len = 0;
	int  far bytes_sent = 0;
	BACNET_NPDU_DATA far  npdu_data;
	BACNET_ADDRESS  far my_address;
	int invoke_id = 0;
#if BAC_DCC
	if (!dcc_communication_enabled())
		return -1;
#endif
	invoke_id = 0;
	len = 0;
	pdu_len = 0;
	bytes_sent = 0;
	
	
	datalink_get_my_address(&my_address,protocal);
	/* encode the NPDU portion of the packet */
	// npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);	//Fance
	npdu_encode_npdu_data(&npdu_data, true, MESSAGE_PRIORITY_NORMAL);
	
	pdu_len =
		npdu_encode_pdu(&Handler_Transmit_Buffer[protocal][0], dest, &my_address,
		&npdu_data);

	/* encode the APDU portion of the packet */

	 invoke_id = tsm_next_free_invokeID();

	 if(invoke_id == 0)//???????????ID??????ID???;????;
	 tsm_free_all_invoke_id();
		 
	len =
		ptransfer_encode_apdu(&Handler_Transmit_Buffer[protocal][pdu_len],invoke_id,private_data);	//???????? Invoke ID
	// uptransfer_encode_apdu(&Handler_Transmit_Buffer[protocal][pdu_len],		//Fance
	// private_data);

	pdu_len += len;

	
	// added by chelsea
	if(protocal == BAC_MSTP)
	{
		MSTP_Flag.TransmitPacketPending = 0;
		memcpy(&TransmitPacket,&Handler_Transmit_Buffer[protocal][0],pdu_len);
		MSTP_Transfer_Len = pdu_len;
	}

	bytes_sent =
		datalink_send_pdu(dest, &npdu_data, &Handler_Transmit_Buffer[protocal][0],
		pdu_len,protocal);
#if PRINT_ENABLED
	if (bytes_sent <= 0)
		fprintf(stderr,
		"Failed to Send UnconfirmedPrivateTransfer Request (%s)!\n",
		strerror(errno));
#endif


	if(bytes_sent > 0)
	{
		return invoke_id;
	}
	else
	{
		return -2;
	}
}

#endif
