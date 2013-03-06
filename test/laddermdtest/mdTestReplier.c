/**********************************************************************************************************************/
/**
 * @file	mdTestReplier.c
 *
 * @brief	Demo MD ladder application for TRDP
 *
 * @details	TRDP Ladder Topology Support MD Transmission Replier
 *
 * @note	Project: TCNOpen TRDP prototype stack
 *
 * @author	Kazumasa Aiba, TOSHIBA
 *
 * @remarks	All rights reserved. Reproduction, modification, use or disclosure
 *		to third parties without express authority is forbidden,
 *		Copyright TOSHIBA, Japan, 2013.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>
#include <sys/select.h>
#include <mqueue.h>
#include <time.h>

#ifdef __linux__
	#include <uuid/uuid.h>
#endif

#include "trdp_types.h"
#include "trdp_if_light.h"
#include "trdp_private.h"
#include "trdp_utils.h"

#include "mdTestApp.h"

/**********************************************************************************************************************/
/** MDReplier Thread
 *
 *  @param[in]		pReplierThreadParameter			pointer to ReplierThread Parameter
 *
 */
VOS_THREAD_FUNC_T MDReplier (
		REPLIER_THREAD_PARAMETER *pReplierThreadParameter)
{
	mqd_t replierMqDescriptor = 0;
	int err = MD_APP_NO_ERR;
/*	TRDP_LIS_T replierLisHandle, replierLisHandle2; */
	APP_THREAD_SESSION_HANDLE appThreadSessionHandle ={{0}};		/* appThreadSessionHandle for Subnet1 */
	APP_THREAD_SESSION_HANDLE appThreadSessionHandle2 ={{0}};		/* appThreadSessionHandle for Subnet2 */
	/* AppThreadListener Area */
	appThreadSessionHandle.pMdAppThreadListener = (TRDP_LIS_T)malloc(sizeof(TRDP_LIS_T));
	appThreadSessionHandle2.pMdAppThreadListener = (TRDP_LIS_T)malloc(sizeof(TRDP_LIS_T));

	/* Check Reply Error Type */
	/* Other than MD_REPLY_ERROR_TYPE_6 */
	if (pReplierThreadParameter->pCommandValue->mdReplyErr != MD_REPLY_ERROR_TYPE_6)
	{
		/* Multicast I/F ? */
		if (vos_isMulticast(pReplierThreadParameter->pCommandValue->mdDestinationAddress))
		{
			/* Add Listener for Multicast */
			err = tlm_addListener(
					appHandle,
//					&(appThreadSessionHandle.pMdAppThreadListener),
					NULL,
					0,						/* user supplied value returned with reply */
					pReplierThreadParameter->pCommandValue->mdAddListenerComId,		/* comId to be observed */
					0,							/* topocount to use */
					pReplierThreadParameter->pCommandValue->mdDestinationAddress,	/* destination Address (Multicast Group) */
					0,							/* OPTION FLAG */
					subnetId1URI);			/* destination URI */
		}
		else
		{
			/* Add Listener for Subnet1 */
			err = tlm_addListener(
					appHandle,
//					&(appThreadSessionHandle.pMdAppThreadListener),
					NULL,
					0,						/* user supplied value returned with reply */
					pReplierThreadParameter->pCommandValue->mdAddListenerComId,		/* comId to be observed */
					0,							/* topocount to use */
					subnetId1Address,			/* destination Address */
					0,							/* OPTION FLAG */
					subnetId1URI);			/* destination URI */
		}
		/* Check tlm_addListener Return Code */
		if (err != TRDP_NO_ERR)
		{
			printf("AddListener comID = 0x%x error = %d\n", pReplierThreadParameter->pCommandValue->mdAddListenerComId, err);
			return 0;
		}
		printf("AddListener(): comID = 0x%x, replierLisHandle = %p\n",
				pReplierThreadParameter->pCommandValue->mdAddListenerComId,
				appThreadSessionHandle.pMdAppThreadListener);
		/* Set Subnet1 appThreadListener */
		appThreadSessionHandle.pMdAppThreadListener->comId = pReplierThreadParameter->pCommandValue->mdAddListenerComId;
		appThreadSessionHandle.pMdAppThreadListener->srcIpAddr = IP_ADDRESS_NOTHING;
		/* Multicast I/F ? */
		if (vos_isMulticast(pReplierThreadParameter->pCommandValue->mdDestinationAddress))
		{
			appThreadSessionHandle.pMdAppThreadListener->destIpAddr = pReplierThreadParameter->pCommandValue->mdDestinationAddress;
		}
		else
		{
			appThreadSessionHandle.pMdAppThreadListener->destIpAddr = subnetId1Address;
		}

		/* Is this Ladder Topology ? */
		if (pReplierThreadParameter->pCommandValue->mdLadderTopologyFlag == TRUE)
		{
			/* Multicast I/F ? */
			if (vos_isMulticast(pReplierThreadParameter->pCommandValue->mdDestinationAddress))
			{
				/* Add Listener for Multicast */
				err = tlm_addListener(
							appHandle2,
//							(&appThreadSessionHandle2.pMdAppThreadListener),
							NULL,
							0,						/* user supplied value returned with reply */
							pReplierThreadParameter->pCommandValue->mdAddListenerComId,		/* comId to be observed */
							0,							/* topocount to use */
							subnetId2Address,			/* destination Address */
							0,							/* OPTION FLAG */
							subnetId2URI);			/* destination URI */
			}
			else
			{
				/* Add Listener for Subnet2 */
				err = tlm_addListener(
							appHandle2,
//							(&appThreadSessionHandle2.pMdAppThreadListener),
							NULL,
							0,						/* user supplied value returned with reply */
							pReplierThreadParameter->pCommandValue->mdAddListenerComId,		/* comId to be observed */
							0,							/* topocount to use */
							subnetId2Address,			/* destination Address */
							0,							/* OPTION FLAG */
							subnetId2URI);			/* destination URI */
			}
			/* Check tlm_addListener Return Code */
			if (err != TRDP_NO_ERR)
			{
				printf("AddListener comID = 0x%x error = %d\n", pReplierThreadParameter->pCommandValue->mdAddListenerComId, err);
				return 0;
			}
			printf("AddListener: comID = 0x%x, replierLisHandle2 = %p\n",
					pReplierThreadParameter->pCommandValue->mdAddListenerComId,
					appThreadSessionHandle2.pMdAppThreadListener);
			/* Set Subnet2 appThreadListener */
			appThreadSessionHandle2.pMdAppThreadListener->comId = pReplierThreadParameter->pCommandValue->mdAddListenerComId;
			appThreadSessionHandle2.pMdAppThreadListener->srcIpAddr = IP_ADDRESS_NOTHING;
			/* Multicast I/F ? */
			if (vos_isMulticast(pReplierThreadParameter->pCommandValue->mdDestinationAddress))
			{
				appThreadSessionHandle2.pMdAppThreadListener->destIpAddr = pReplierThreadParameter->pCommandValue->mdDestinationAddress;
			}
			else
			{
				appThreadSessionHandle2.pMdAppThreadListener->destIpAddr = subnetId2Address;
			}
		}
	}
	/* MD_REPLY_ERROR_TYPE_6 */
	else
	{
		/* Not AddListner */
	}

	/* Message Queue Open */
	err = queue_initialize(pReplierThreadParameter->mqName, &replierMqDescriptor);
	if (err != MD_APP_NO_ERR)
	{
		printf("Replier Message Queue Open error\n");
		return 0;
	}
	else
	{
		/* Set Replier Message Queue Descriptor */
		err = setAppThreadSessionMessageQueueDescriptor(&appThreadSessionHandle, replierMqDescriptor);
		if (err != MD_APP_NO_ERR)
		{
			printf("Subnet1 setAppThreadSessionMessageQueueDescriptor error\n");
			return 0;
		}
		/* Is this Ladder Topology ? */
		if (pReplierThreadParameter->pCommandValue->mdLadderTopologyFlag == TRUE)
		{
			/* Set Replier Message Queue Descriptor */
			err = setAppThreadSessionMessageQueueDescriptor(&appThreadSessionHandle2, replierMqDescriptor);
			if (err != MD_APP_NO_ERR)
			{
				printf("Subnet2 setAppThreadSessionMessageQueueDescriptor error\n");
				return 0;
			}
		}
	}

	/* Replier Thread main process */
	replier_main_proc(
			replierMqDescriptor,
			pReplierThreadParameter->pCommandValue->mdAddListenerComId,
			pReplierThreadParameter);

	return 0;
}

/**********************************************************************************************************************/
/** Replier thread main loop process
 *
 *  @param[in]		mqDescriptor						Message Queue Descriptor
 *  @param[in]		replierComId						Replier ComId
 *  @param[in]		pReplierThreadParameter			pointer to Replier Thread parameter
 *
 *  @retval         0					no error
 *  @retval         1					error
 */
MD_APP_ERR_TYPE replier_main_proc (
		mqd_t mqDescriptor,
		UINT32 replierComId,
		REPLIER_THREAD_PARAMETER *pReplierThreadParameter)
{
	MD_APP_ERR_TYPE err = MD_APP_NO_ERR;
	trdp_apl_cbenv_t receiveMqMsg;

	/* LOG */
//	CHAR8 logString[CALLER_LOG_BUFFER_SIZE] ={0};		/* Replier Log String */
	CHAR8 *logString = NULL;
	size_t logStringLength = 0;
/*	CHAR8 *pWorkLogString = logString; */
	size_t workLogStringLength = 0;
	char strIp[16] = {0};
	logString= calloc(CALLER_LOG_BUFFER_SIZE, sizeof(CHAR8));

	/* Output Log of Replier Thread Parameter */
	if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG)
		|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG))
	{
		/* Set Log String */
		/* -b --md-caller-replier-type Value */
		workLogStringLength = sprintf(logString,
				"Caller Replier Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdCallerReplierType);
		logStringLength = workLogStringLength;
		/* -c --md-transport-type Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Transport Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdTransportType);
		logStringLength = logStringLength + workLogStringLength;
		/* -d --md-message-kind Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Message Kind : %u\n",
				pReplierThreadParameter->pCommandValue->mdMessageKind);
		logStringLength = logStringLength + workLogStringLength;
		/* -e --md-telegram-type Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Telegram Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdTelegramType);
		logStringLength = logStringLength + workLogStringLength;
		/* -f --md-message-size Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Message Size : %u\n",
				pReplierThreadParameter->pCommandValue->mdMessageSize);
		logStringLength = logStringLength + workLogStringLength;
		/* -g --md-destination-address Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Destination IP Address : %s\n",
				miscIpToString(pReplierThreadParameter->pCommandValue->mdDestinationAddress, strIp));
		logStringLength = logStringLength + workLogStringLength;
		/* -i --md-dump Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Dump Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdDump);
		logStringLength = logStringLength + workLogStringLength;
		/* -j --md-replier-number Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Number of Replier : %u\n",
				pReplierThreadParameter->pCommandValue->mdReplierNumber);
		logStringLength = logStringLength + workLogStringLength;
		/* -k --md-cycle-number Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Number of MD Request Send Cycle : %u\n",
				pReplierThreadParameter->pCommandValue->mdCycleNumber);
		logStringLength = logStringLength + workLogStringLength;
		/* -l --md-log Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Log Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdLog);
		logStringLength = logStringLength + workLogStringLength;
		/* -m --md-cycle-time Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"MD Request Send Cycle Time : %u\n",
				pReplierThreadParameter->pCommandValue->mdCycleTime);
		logStringLength = logStringLength + workLogStringLength;
		/* -n --md-topo Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Ladder Topology Support Flag : %u\n",
				pReplierThreadParameter->pCommandValue->mdLadderTopologyFlag);
		logStringLength = logStringLength + workLogStringLength;
		/* -o --md-reply-err Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Reply Error Type : %u\n",
				pReplierThreadParameter->pCommandValue->mdReplyErr);
		logStringLength = logStringLength + workLogStringLength;
		/* -p --md-marshall Value*/
		workLogStringLength = sprintf(logString + logStringLength,
				"Marshalling Support Flag : %u\n",
				pReplierThreadParameter->pCommandValue->mdMarshallingFlag);
		logStringLength = logStringLength + workLogStringLength;
		/* -r --md-listener-comid Value */
		workLogStringLength = sprintf(logString + logStringLength,
				"Listener ComId : 0x%x\n",
				pReplierThreadParameter->pCommandValue->mdAddListenerComId);
		logStringLength = logStringLength + workLogStringLength;
		/* Caller Send comId */
		workLogStringLength = sprintf(logString + logStringLength,
				"Caller Send ComId : 0x%x\n",
				pReplierThreadParameter->pCommandValue->mdSendComId);
		logStringLength = logStringLength + workLogStringLength;
		/* -r --md-timeout-reply Value */
		workLogStringLength = sprintf(logString + logStringLength,
					"Reply Timeout : %u\n",
					pReplierThreadParameter->pCommandValue->mdTimeoutReply);
		logStringLength = logStringLength + workLogStringLength;
		/* -t --md-send-subnet Value */
		workLogStringLength = sprintf(logString + logStringLength,
					"Sender Subnet : %u\n",
					pReplierThreadParameter->pCommandValue->mdSendSubnet);
		logStringLength = logStringLength + workLogStringLength;
		/* MD Application Version */
		workLogStringLength = sprintf(logString + logStringLength,
					"MD Application Version : %s\n",
					MD_APP_VERSION);
		logStringLength = logStringLength + workLogStringLength;

		/* Lock MD Application Thread Mutex */
//		lockMdApplicationThread();

		/* Output Log : Operation Log */
		l2fLog(logString,
				((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG),
				((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG));

		/* UnLock MD Application Thread Mutex */
//		unlockMdApplicationThread();

		/* Clear Log String */
		memset(logString, 0, sizeof(logString));
		logStringLength = 0;
	}

	while(1)
	{
		/* Receive Message Queue */
		err = queue_receiveMessage(&receiveMqMsg, mqDescriptor);
		if (err != MD_APP_NO_ERR)
		{
/*			printf( "Replier queue_receiveMessage ERROR.\n"); */
		}
		else
		{
			/* Output LOG : MD Operation Result Log ? */
			if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG)
				|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG))
			{
				/* Output MD Operation Result Log to md_indicate() TimeStamp */
				l2fLog(receiveMqMsg.timeStampString,
						((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG),
						((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG));
			}
			/* Output LOG : Receive Log ? */
			if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_RECEIVE_LOG) == MD_RECEIVE_LOG)
					|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_RECEIVE_LOG) == MD_RECEIVE_LOG))
			{
				strncpy(logString, receiveMqMsg.timeStampString, sizeof(receiveMqMsg.timeStampString));
				logStringLength = strlen(logString);
				/* Receive MD DATA String */
				sprintf((char *)(logString + logStringLength), "Receive MD DATA\n");
				/* Output Receive Log : MD DATA Title */
				l2fLog(logString,
						((pReplierThreadParameter->pCommandValue->mdLog) & MD_RECEIVE_LOG),
						((pReplierThreadParameter->pCommandValue->mdDump) & MD_RECEIVE_LOG));
				/* Output Receive Log : MD DATA */
				miscMemory2String(
						receiveMqMsg.pData,
//						(receiveMqMsg.pData) + MD_HEADER_SIZE,
						receiveMqMsg.dataSize,
//						(receiveMqMsg.dataSize) - MD_HEADER_SIZE - MD_FCS_SIZE,
						((pReplierThreadParameter->pCommandValue->mdLog) & MD_RECEIVE_LOG),
						((pReplierThreadParameter->pCommandValue->mdDump) & MD_RECEIVE_LOG),
						RECURSIVE_CALL_NOTHING);
				/* Clear Log String */
				memset(logString, 0, sizeof(logString));
				logStringLength = 0;
			}

			/* Decide MD Message */
			err = decideReceiveMdDataToReplier(&receiveMqMsg, pReplierThreadParameter);
			if (err != MD_APP_NO_ERR)
			{
				printf( "decideReceiveMdDataToReplier ERROR.\n");
			}
		}
	}
}

/**********************************************************************************************************************/
/** Decide Receive MD DATA
 *
 *  @param[in]		pReceiveMsg						pointer to Receive MD Message
 *  @param[in]		pReplierThreadParameter			pointer to Replier Thread parameter
 *
 *  @retval         0					no error
 *  @retval         1					error
 *
 */
MD_APP_ERR_TYPE decideReceiveMdDataToReplier (
		trdp_apl_cbenv_t *pReceiveMsg,
		REPLIER_THREAD_PARAMETER *pReplierThreadParameter)
{
	MD_APP_ERR_TYPE err = 0;
	TRDP_APP_SESSION_T replierAppHandle = appHandle;
	UINT32 useSubnet = 0;
	UINT32 receiveMdDataSetSize = 0;
	/* LOG */
	CHAR8 replierLogString[CALLER_LOG_BUFFER_SIZE] ={0};		/* Replier Log String */
	size_t replierLogStringLength = 0;
	/* Result Counter */
	static UINT32 mdReceiveCounter = 0;						/* MD Receive Counter */
	static UINT32 mdReceiveFailureCounter = 0;				/* MD Receive OK Counter */
	static UINT32 mdReceiveSuccessCounter = 0;				/* MD Receive NG Counter */
	static UINT32 mdRetryCounter = 0;							/* MD Retry Counter */

	/* Get MD DATASET Size (MD DATA = MD HEADER + MD DATASET + FCR) */
	receiveMdDataSetSize = (pReceiveMsg->dataSize) - MD_HEADER_SIZE - MD_FCS_SIZE;

	/* Check ComId */
	if (pReceiveMsg->Msg.comId != pReplierThreadParameter->pCommandValue->mdAddListenerComId)
	{
		/* ComId Err*/
		printf( "Receive ComId ERROR\n");
	}
	else
	{
		/* Check Result Code */
		if (decideResultCode(pReceiveMsg->Msg.resultCode) == MD_APP_NO_ERR)
		{
			/* Check msgType */
			switch(pReceiveMsg->Msg.msgType)
			{
				case TRDP_MSG_MN:
/* Lock MD Application Thread Mutex */
//lockMdApplicationThread();
					/* Decide MD Transmission Result */
					err = decideMdTransmissionResult(
							pReceiveMsg->Msg.comId,
							pReceiveMsg->pData,
//							(UINT32 *)pReceiveMsg->dataSize,
							&(pReceiveMsg->dataSize),
							replierLogString);
/* UnLock MD Application Thread Mutex */
//unlockMdApplicationThread();
					if (err == MD_APP_NO_ERR)
					{
						/* MD Receive OK Count UP*/
						mdReceiveSuccessCounter++;
					}
					else
					{
						/* MD Receive NG Count */
						mdReceiveFailureCounter++;
					}
					/* MD Receive Count */
					mdReceiveCounter++;

					/* Output LOG : Operation Log */
					if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG)
						|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG))
					{
						replierLogStringLength = strlen(replierLogString);
						sprintf((char *)(replierLogString + replierLogStringLength), "MD Receive Count = %u\nMD Receive OK Count = %u\nMD Receive NG Count = %u\nMD Retry Count = %u\n",
								mdReceiveCounter, mdReceiveSuccessCounter, mdReceiveFailureCounter, mdRetryCounter);
						l2fLog(replierLogString,
								((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG),
								((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG));
					}
					/* Clear Log String */
					memset(replierLogString, 0, sizeof(replierLogString));
				break;
				case TRDP_MSG_MR:
					/* Set Receive appHandle */
//					if (pReceiveMsg->pRefCon == MD_SEND_USE_SUBNET1)
					memcpy(&useSubnet, (void *)pReceiveMsg->pRefCon, sizeof(INT8));
					if (useSubnet == MD_SEND_USE_SUBNET1)
					{
						/* Receive to Subnet1 */
						replierAppHandle = appHandle;
						/* Set tlp_reply(srcIpAddr = subnet1) */
						pReceiveMsg->Msg.destIpAddr = subnetId1Address;
					}
					else
					{
						/* Receive to Subnet2 */
						replierAppHandle = appHandle2;
						/* Set tlp_reply(srcIpAddr = subnet2) */
						pReceiveMsg->Msg.destIpAddr = subnetId2Address;
					}

					/* Check ComId */
					/* Decide Reply Err Mode */
					/* COMID_ERROR_DATA_1 */
					if (pReceiveMsg->Msg.comId == COMID_ERROR_DATA_1)
					{
						/* Check Reply Error Type */
						/* MD_REPLY_ERROR_TYPE_1 */
						if (pReplierThreadParameter->pCommandValue->mdReplyErr == MD_REPLY_ERROR_TYPE_1)
						{
							/* Get TimeStamp when call tlm_reply() */
							sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

							/* Set Reply Status = -1 (trdp_mdCommonSend 13 argument) */
							/* Send MD Reply */
							trdp_mdCommonSend(
									TRDP_MSG_MP,							/* reply without confirm */
									replierAppHandle,						/* the handle returned by tlc_init */
									NULL,									/* user supplied value returned with reply */
									&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
									(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
									pReceiveMsg->Msg.topoCount,			/* topocount to use */
									pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
									pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
									0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
									0, 										/* userStatus */
									0,             						 /* confirm timeout */
									0,             						 /* noOfRepliers */
									0,              						/* replyTimeout */
									TRDP_REPLY_APPL_TIMEOUT,  			/* reply state (TRDP_REPLY_APPL_TIMEOUT = -1) */
									NULL, 									/* send param */
//									(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
									(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//									pReceiveMsg->dataSize,			/* size of packet data */
									receiveMdDataSetSize,			/* size of packet data */
									pReceiveMsg->Msg.destURI,			/* source URI */
									pReceiveMsg->Msg.srcURI);			/* destination URI */
							if (err != TRDP_NO_ERR)
							{
								/* Error : Send Reply */
								printf( "Send Reply ERROR\n");
							}
						}
						/* MD_REPLY_ERROR_TYPE_2 */
						else if (pReplierThreadParameter->pCommandValue->mdReplyErr == MD_REPLY_ERROR_TYPE_2)
						{
							/* Resource Error */
							/* Get TimeStamp when call tlm_reply() */
							sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

							/* Set Reply Status = no memory (trdp_mdCommonSend 13 argument) */
							/* Send MD Reply */
							trdp_mdCommonSend(
									TRDP_MSG_MP,							/* reply without confirm */
									replierAppHandle,						/* the handle returned by tlc_init */
									NULL,									/* user supplied value returned with reply */
									&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
									(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
									pReceiveMsg->Msg.topoCount,			/* topocount to use */
									pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
									pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
									0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
									0, 										/* userStatus */
									0,             						 /* confirm timeout */
									0,             						 /* noOfRepliers */
									0,              						/* replyTimeout */
									TRDP_REPLY_NO_MEM_REPL,  			/* reply state (TRDP_REPLY_NO_MEM_REPL = no memory) */
									NULL, 									/* send param */
//									(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
									(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//									pReceiveMsg->dataSize,			/* size of packet data */
									receiveMdDataSetSize,			/* size of packet data */
									pReceiveMsg->Msg.destURI,			/* source URI */
									pReceiveMsg->Msg.srcURI);			/* destination URI */
							if (err != TRDP_NO_ERR)
							{
								/* Error : Send Reply */
								printf( "Send Reply ERROR\n");
							}
						}
						/* MD_REPLY_ERROR_TYPE_3 */
						else if (pReplierThreadParameter->pCommandValue->mdReplyErr == MD_REPLY_ERROR_TYPE_3)
						{
							/* Get TimeStamp when call tlm_reply() */
							sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

							/* ComId Error */
							/* Set ComId = 0 (tlm_reply 5 argument) */
							/* Send MD Reply */
							err = tlm_reply (
								replierAppHandle,						/* the handle returned by tlc_init */
								NULL,									/* user supplied value returned with reply */
								&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
								pReceiveMsg->Msg.topoCount,			/* topocount to use */
								0,										/* comId of packet to be sent */
								pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
								pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
								0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
								0, 										/* userStatus */
								NULL, 									/* send param */
//								(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
								(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//								pReceiveMsg->dataSize,				/* size of packet data */
								receiveMdDataSetSize,				/* size of packet data */
								pReceiveMsg->Msg.destURI,			/* source URI */
								pReceiveMsg->Msg.srcURI);			/* destination URI */
							if (err != TRDP_NO_ERR)
							{
								/* Error : Send Reply */
								printf( "Send Reply ERROR\n");
							}
						}
						/* MD_REPLY_ERROR_TYPE_4 */
						else if (pReplierThreadParameter->pCommandValue->mdReplyErr == MD_REPLY_ERROR_TYPE_4)
						{
							/* Get TimeStamp when call tlm_reply() */
							sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

							/* Set DataSize Error (return TRDP_PARAM_ERR) */
							/* Set pData = NULL (tlm_reply 11 argument) */
							/* Set DataSize != 0 (tlm_reply 12 argument) */
							/* Send MD Reply */
							err = tlm_reply (
								replierAppHandle,						/* the handle returned by tlc_init */
								NULL,									/* user supplied value returned with reply */
								&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
								pReceiveMsg->Msg.topoCount,			/* topocount to use */
								(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
								pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
								pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
								0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
								0, 										/* userStatus */
								NULL, 									/* send param */
//								(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
//								(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
								NULL,									/* DATASET Nothing */
								1,										/* size of packet data */
								pReceiveMsg->Msg.destURI,			/* source URI */
								pReceiveMsg->Msg.srcURI);			/* destination URI */
							if (err != TRDP_NO_ERR)
							{
								/* Error : Send Reply */
								printf( "Send Reply ERROR\n");
							}

						}
						/* MD_REPLY_ERROR_TYPE_5 */
						else if (pReplierThreadParameter->pCommandValue->mdReplyErr == MD_REPLY_ERROR_TYPE_5)
						{
							/* not Call tlm_reply */

						}
						else
						{

						}
					}
					/* COMID_ERROR_DATA_2 */
					else if (pReceiveMsg->Msg.comId == COMID_ERROR_DATA_2)
					{
						/* Not Listener */
					}
					/* COMID_ERROR_DATA_3 */
					else if (pReceiveMsg->Msg.comId == COMID_ERROR_DATA_3)
					{
						/* Get TimeStamp when call tlm_reply() */
						sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

						/* ComId Error */
						/* Set ComId = 0 (tlm_common_send 5 argument) */
						/* Send MD Reply */
						err = tlm_reply (
							replierAppHandle,						/* the handle returned by tlc_init */
							NULL,									/* user supplied value returned with reply */
							&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
							pReceiveMsg->Msg.topoCount,			/* topocount to use */
							0,										/* comId of packet to be sent */
							pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
							pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
							0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
							0, 										/* userStatus */
							NULL, 									/* send param */
//							(UINT8 *)pReceiveMsg->pData,		/* pointer to packet data or dataset */
							(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//							pReceiveMsg->dataSize,				/* size of packet data */
							receiveMdDataSetSize,				/* size of packet data */
							pReceiveMsg->Msg.destURI,			/* source URI */
							pReceiveMsg->Msg.srcURI);			/* destination URI */
						if (err != TRDP_NO_ERR)
						{
							/* Error : Send Reply */
							printf( "Send Reply ERROR\n");
						}
					}
					/* COMID_ERROR_DATA_4 */
					else if (pReceiveMsg->Msg.comId == COMID_ERROR_DATA_4)
					{
						/* Get TimeStamp when call tlm_reply() */
						sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

						/* Retry Reply */
						/* Send MD Reply */
						err = tlm_reply (
							replierAppHandle,						/* the handle returned by tlc_init */
							NULL,									/* user supplied value returned with reply */
							&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
							pReceiveMsg->Msg.topoCount,			/* topocount to use */
							(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
							pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
							pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
							0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
							0, 										/* userStatus */
							NULL, 									/* send param */
//							(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
							(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//							pReceiveMsg->dataSize,			/* size of packet data */
							receiveMdDataSetSize,			/* size of packet data */
							pReceiveMsg->Msg.destURI,			/* source URI */
							pReceiveMsg->Msg.srcURI);			/* destination URI */
						if (err != TRDP_NO_ERR)
						{
							/* Error : Send Reply */
							printf( "Send Reply ERROR\n");
						}
						/* Get TimeStamp when call tlm_reply() */
						sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

						/* Retry Send MD Reply */
						err = tlm_reply (
							replierAppHandle,						/* the handle returned by tlc_init */
							NULL,									/* user supplied value returned with reply */
							&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
							pReceiveMsg->Msg.topoCount,			/* topocount to use */
							(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
							pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
							pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
							0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
							0, 										/* userStatus */
							NULL, 									/* send param */
//							(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
							(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//							pReceiveMsg->dataSize,				/* size of packet data */
							receiveMdDataSetSize,				/* size of packet data */
							pReceiveMsg->Msg.destURI,			/* source URI */
							pReceiveMsg->Msg.srcURI);			/* destination URI */
						if (err != TRDP_NO_ERR)
						{
							/* Error : Send Reply */
							printf( "Send Reply ERROR\n");
						}
					}
					/* Normal ComId */
					else
					{
						/* Get TimeStamp when call tlm_reply() */
						sprintf(replierLogString, "%s tlm_reply()", vos_getTimeStamp());

						/* Send MD Reply */
						err = tlm_reply (
							replierAppHandle,						/* the handle returned by tlc_init */
							NULL,									/* user supplied value returned with reply */
							&(pReceiveMsg->Msg.sessionId),			/* Session ID returned by indication */
							pReceiveMsg->Msg.topoCount,			/* topocount to use */
							(pReceiveMsg->Msg.comId) | COMID_REPLY_MASK,		/* comId of packet to be sent */
							pReceiveMsg->Msg.destIpAddr,		/* srcIP Address */
							pReceiveMsg->Msg.srcIpAddr,			/* where to send the packet to */
							0, 										/* pktFlags 0 = Default = appHandle->mdDefault.flag */
							0, 										/* userStatus */
							NULL, 									/* send param */
//							(UINT8 *)pReceiveMsg->pData,	/* pointer to packet data or dataset */
							(UINT8 *)(pReceiveMsg->pData + MD_HEADER_SIZE),	/* pointer to packet data or dataset */
//							pReceiveMsg->dataSize,			/* size of packet data */
							receiveMdDataSetSize,			/* size of packet data */
							pReceiveMsg->Msg.destURI,			/* source URI */
							pReceiveMsg->Msg.srcURI);			/* destination URI */
						if (err != TRDP_NO_ERR)
						{
							/* Error : Send Reply */
							printf( "Send Reply ERROR\n");
						}
					}

					/* Output LOG : MD Operation Result Log ? */
					if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG)
						|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG))
					{
						/* Output MD Operation Result Log */
						l2fLog(replierLogString,
								((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG),
								((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG));
					}
					/* Output LOG : Send Log ? */
					if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_SEND_LOG) == MD_SEND_LOG)
							|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_SEND_LOG) == MD_SEND_LOG))
					{
						replierLogStringLength = strlen(replierLogString);
						/* Send MD DATA String */
						sprintf((char *)(replierLogString + replierLogStringLength), "Send MD DATA\n");
						/* Output Send Log : MD DATA Title */
						l2fLog(replierLogString,
								((pReplierThreadParameter->pCommandValue->mdLog) & MD_SEND_LOG),
								((pReplierThreadParameter->pCommandValue->mdDump) & MD_SEND_LOG));
						/* Output Send Log : MD DATA */
						miscMemory2String(
								pReceiveMsg->pData,
								pReceiveMsg->dataSize,
								((pReplierThreadParameter->pCommandValue->mdLog) & MD_SEND_LOG),
								((pReplierThreadParameter->pCommandValue->mdDump) & MD_SEND_LOG),
								RECURSIVE_CALL_NOTHING);
					}
					/* Clear Log String */
					memset(replierLogString, 0, sizeof(replierLogString));
					replierLogStringLength = 0;

/* UnLock MD Application Thread Mutex */
//lockMdApplicationThread();
					/* Decide MD Transmission Result */
					err = decideMdTransmissionResult(
							pReceiveMsg->Msg.comId,
							pReceiveMsg->pData,
							&(pReceiveMsg->dataSize),
							replierLogString);
/* UnLock MD Application Thread Mutex */
//unlockMdApplicationThread();
					if (err == MD_APP_NO_ERR)
					{
						/* MD Receive OK Count UP*/
						mdReceiveSuccessCounter++;
					}
					else
					{
						/* MD Receive NG Count */
						mdReceiveFailureCounter++;
					}
					/* MD Receive Count */
					mdReceiveCounter++;
					/* MD Retry Count */

					/* Output LOG */
					if ((((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG)
						|| (((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG) == MD_OPERARTION_RESULT_LOG))
					{
						replierLogStringLength = strlen(replierLogString);
						sprintf((char *)(replierLogString + replierLogStringLength), "MD Receive Count = %u\nMD Receive OK Count = %u\nMD Receive NG Count = %u\nMD Retry Count = %u\n",
								mdReceiveCounter, mdReceiveSuccessCounter, mdReceiveFailureCounter, mdRetryCounter);
						l2fLog(replierLogString,
								((pReplierThreadParameter->pCommandValue->mdLog) & MD_OPERARTION_RESULT_LOG),
								((pReplierThreadParameter->pCommandValue->mdDump) & MD_OPERARTION_RESULT_LOG));
					}
					/* Clear Log String */
					memset(replierLogString, 0, sizeof(replierLogString));
				break;
				case TRDP_MSG_MP:
				case TRDP_MSG_MQ:
				case TRDP_MSG_MC:
				case TRDP_MSG_ME:
						/* Error : msgType Other than Mn,Mr */
						printf( "Receive Message Type ERROR. Other than Mn,Mr\n");
				break;
				default:
					/* Not TRDP msgType */
					printf( "Receive Message Type ERROR. Not TRDP Message Type\n");
				break;
			}
		}
		else
		{
			/* Result Code Err */
		}
	}
	return 0;
}
