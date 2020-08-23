/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
 *   12/21/2005   xiaoweifang  Initial file creation.
 *---------------------------------------------------------------------------*/

#include "L3DataEB.h"
void EBConfig();
void CreateUserContext();


void StartL3()
{
    //简单配置消息;
    EBConfig();
    //创建用户上下文;
    CreateUserContext();
    return;
}



void EBConfig()
{
    /**************************
    *用户过期时间(秒)，可修改
    *注意别溢出 <= 65535.
    */
    UINT16 usTime = 5 * 60 * 60;

    /**************************
    *是否转发下行的广播报文给CPE?
    *可修改
    */
    bool bForwardEgressTraffic = true;


/*****************
*以下代码不可修改
*/
    CDataConfig msgEBConfig;
    msgEBConfig.CreateMessage( *CTBridge::GetInstance() );
    msgEBConfig.SetTransactionId( 0x1234 );
    msgEBConfig.SetEgressBCFltr( bForwardEgressTraffic );
    msgEBConfig.SetLearnedBridgingAgingTime( usTime );
    msgEBConfig.SetPPPoESessionKeepAliveTime( 0x200 );
    msgEBConfig.SetWorkMode( WM_LEARNED_BRIDGING );
    msgEBConfig.SetDstTid( M_TID_EB );

    msgEBConfig.Post();
}

void CreateUserContext()
{
    /**************************
    *模拟创建用户的
    *MAC地址(00-09-5b-ce-68-84)
    *可修改
    */

    UINT8 SrcMAC[ 6 ];
    SrcMAC[0] = 0x00;
    SrcMAC[1] = 0x09;
    SrcMAC[2] = 0x5B;
    SrcMAC[3] = 0xCE;
    SrcMAC[4] = 0x68;
    SrcMAC[5] = 0x84;

    /*****************
    *模拟创建用户的CPE
    *可修改
    */
    UINT32 ulEid = 0xF0E1D2C3;

/*****************
*以下代码不可修改
*/
    UINT8 DstMAC[ 6 ];
    DstMAC[0] = 0x00;
    DstMAC[1] = 0x0E;
    DstMAC[2] = 0xA6;
    DstMAC[3] = 0x79;
    DstMAC[4] = 0xA3;
    DstMAC[5] = 0x00;

    EtherHdr Ether;
    memcpy( Ether.aucDstMAC, DstMAC, 6 );
    memcpy( Ether.aucSrcMAC, SrcMAC, 6 );
    Ether.usProto = htons( M_ETHER_TYPE_IP );

    UINT16 usDataLen = sizeof( EtherHdr ) + sizeof( IpHdr );
    CComMessage *pComMsg = new ( CTBridge::GetInstance(), usDataLen )CComMessage;
    if ( NULL == pComMsg )
        {
        return;
        }
    pComMsg->SetDstTid( M_TID_EB ); 
    pComMsg->SetSrcTid( M_TID_L2MAIN ); 
    pComMsg->SetMessageId( MSGID_HIGH_PRIORITY_TRAFFIC );
    pComMsg->SetEID( ulEid );
    memcpy( pComMsg->GetDataPtr(), &Ether, sizeof( Ether ) );

    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        pComMsg->Destroy();
        }
}
