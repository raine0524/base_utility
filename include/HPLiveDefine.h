#pragma once

#define RECOVER_TIME 500

typedef enum EnDeviceType
{
    DEV_ENCODER,
    DEV_H323,
    DEV_STREAM,
    DEV_NGINX,

    DEV_UNKNOWN = 99,
};

//-------------------------------------
// 基本设备信息
typedef struct DeviceInfo
{
    DeviceInfo()
    {
        usChannelNum    = 0;
        usMcuPort       = 0;
        usLocalPort     = 0;
        usStatus		= 0;
        usDevType       = DEV_UNKNOWN;
        nNginxPort      = 0;
    }

    DeviceInfo(const DeviceInfo& other)
    {
        usChannelNum = other.usChannelNum;
        usDevType = other.usDevType ;
        usStatus = other.usStatus;
        usLocalPort = other.usLocalPort;
        usMcuPort = other.usMcuPort;
        strDevID = other.strDevID;
        strDevName = other.strDevName;
        strMcuID = other.strMcuID;
        strMcuAddr = other.strMcuAddr;
        strNodeID = other.strNodeID;
        strNatAddr = other.strNatAddr;
        strNatType = other.strNatType;
        strLocalAddr = other.strLocalAddr;
        strMacAddr = other.strMacAddr;
        strURL = other.strURL;
        strServerID = other.strServerID;
        strServerIP = other.strServerIP;
        strAppName = other.strAppName;
        nNginxPort = other.nNginxPort;
    }

    unsigned short  usChannelNum;
    unsigned short  usDevType;
	unsigned short  usStatus;
    unsigned short	usLocalPort;
    unsigned short	usMcuPort;

    // 设备的基本信息
    std::string		strDevID;      // 设备的ID
    std::string     strDevName;

    // 实时数据信息
    std::string		strMcuID;
    std::string		strMcuAddr;
    std::string		strNodeID;
    std::string		strNatAddr;
    std::string     strNatType;
    std::string		strLocalAddr;
    std::string     strMacAddr;
    std::string     strURL;

    // RTMP
    std::string     strServerID;
    std::string     strServerIP;
    std::string     strAppName;
    unsigned int    nNginxPort;
} *PDeviceInfo;

//-------------------------------------
// 监控通道信息
typedef struct ChannelInfo
{
    ChannelInfo()
    {
        usChannelNo = 0;
        ulVideoCID = 0;
        ulAudioCID = 0;
    }
    std::string     strChannelID;
    std::string     strChannelName;
    std::string     strDevID;
    std::string     strNodeID;
    unsigned short  usChannelNo;
    unsigned long	ulVideoCID;
    unsigned long	ulAudioCID;
}*PChannelInfo;

template<typename T>
inline void CMDPacket( T& t, DeviceInfo& devInfo)
{
    t.SetAttrib( "USERID" , devInfo.strDevID);
    t.SetAttrib( "NAME", devInfo.strDevName);
    t.SetAttribUS( "CHANNELNUM" , devInfo.usChannelNum);
    t.SetAttribUS( "DEVIDTYPE" , devInfo.usDevType);

    t.SetAttrib( "MCUID:", devInfo.strMcuID);
    t.SetAttrib( "MCUADDR:", devInfo.strMcuAddr);
    t.SetAttribUS( "MCUPORT:", devInfo.usMcuPort);
    t.SetAttrib( "NODEID", devInfo.strNodeID);
    t.SetAttrib( "NATADDR:", devInfo.strNatAddr);
    t.SetAttrib( "NATTYPE", devInfo.strNatType);
    t.SetAttrib( "LOCALADDR:", devInfo.strLocalAddr);
    t.SetAttribUL( "LOCALPORT:", devInfo.usLocalPort);
    t.SetAttrib( "MACADDR", devInfo.strMacAddr);
    t.SetAttribUS( "STATUS", devInfo.usStatus);
    t.SetAttrib( "URL", devInfo.strURL);
    t.SetAttrib( "SERVERID",devInfo.strServerID);
    t.SetAttrib( "SERVERIP",devInfo.strServerIP);
    t.SetAttrib( "APP",devInfo.strAppName);
    t.SetAttribUN( "NGINXPORT",devInfo.nNginxPort);
}

//------------------------------------------------------------------------
template<typename T>
inline void CMDUnPacket( T& t, DeviceInfo& devInfo)
{
    devInfo.strDevID = t.GetAttrib( "USERID" ).AsString();
    devInfo.strDevName = t.GetAttrib( "NAME" ).AsString();
    devInfo.usChannelNum = t.GetAttrib( "CHANNELNUM" ).AsUnsignedShort();
    devInfo.usDevType = t.GetAttrib( "DEVIDTYPE" ).AsUnsignedShort();

    devInfo.strMcuID = t.GetAttrib( "MCUID:" ).AsString();
    devInfo.strMcuAddr = t.GetAttrib( "MCUADDR:" ).AsString();
    devInfo.usMcuPort = t.GetAttrib( "MCUPORT:" ).AsUnsignedShort();
    devInfo.strNodeID = t.GetAttrib( "NODEID" ).AsString();
    devInfo.strNatAddr = t.GetAttrib( "NATADDR:" ).AsString();
    devInfo.strNatType = t.GetAttrib( "NATTYPE" ).AsString();
    devInfo.strLocalAddr = t.GetAttrib( "LOCALADDR:" ).AsString();
    devInfo.usLocalPort = t.GetAttrib( "LOCALPORT:" ).AsUnsignedShort();
    devInfo.strMacAddr= t.GetAttrib("MACADDR").AsString();
	devInfo.usStatus = t.GetAttrib("STATUS").AsUnsignedShort();
    devInfo.strURL = t.GetAttrib("URL").AsString();

    devInfo.strServerID = t.GetAttrib("SERVERID").AsString();
    devInfo.strServerIP = t.GetAttrib("SERVERIP").AsString();
    devInfo.strAppName  = t.GetAttrib("APP").AsString();
    devInfo.nNginxPort  = t.GetAttrib("NGINXPORT").AsUnsignedInt();

}

/************************************************************************/
/* 打包，解包通道信息                                                   */
/************************************************************************/
template<typename T>
inline void CMDPacket( T& t, PChannelInfo pChannelInfo)
{
    t.SetAttrib( "CHANNELID", pChannelInfo->strChannelID);
    t.SetAttrib( "CHANNELNAME:", pChannelInfo->strChannelName);
    t.SetAttrib( "DEVID", pChannelInfo->strDevID);
    t.SetAttrib( "NODEID", pChannelInfo->strNodeID);
    t.SetAttribUS( "CHNUM", pChannelInfo->usChannelNo);
    t.SetAttribUL( "VIDEOCID:", pChannelInfo->ulVideoCID);
    t.SetAttribUL( "AUDIOCID:", pChannelInfo->ulAudioCID);
}

//------------------------------------------------------------------------
template<typename T>
inline void CMDUnPacket( T& t, PChannelInfo pChannelInfo)
{
    pChannelInfo->strChannelID = t.GetAttrib( "CHANNELID" ).AsString();
    pChannelInfo->strDevID = t.GetAttrib( "DEVID" ).AsString();
    pChannelInfo->strChannelName = t.GetAttrib( "CHANNELNAME:" ).AsString();
    pChannelInfo->strNodeID = t.GetAttrib( "NODEID" ).AsString();
    pChannelInfo->usChannelNo = t.GetAttrib( "CHNUM" ).AsUnsignedShort();
    pChannelInfo->ulVideoCID = t.GetAttrib( "VIDEOCID:" ).AsUnsignedLong();
    pChannelInfo->ulAudioCID = t.GetAttrib( "AUDIOCID:" ).AsUnsignedLong();
    if(pChannelInfo->strChannelID.empty())
    {
        if(pChannelInfo->usChannelNo<10)
        {
            pChannelInfo->strChannelID = pChannelInfo->strDevID + "_0" + USHORT2STR(pChannelInfo->usChannelNo);
        }
        else
        {
            pChannelInfo->strChannelID = pChannelInfo->strDevID + '_' + USHORT2STR(pChannelInfo->usChannelNo);
        }
    }
}

typedef std::map<std::string,DeviceInfo> DEV_MAP;

typedef std::map<const std::string,DeviceInfo*>	MapDevInfo;

#define MAXVIEWERNUM 50
#define BALLANCEVIEWERNUM 5
#define URLITEM 5
#define IPADDR_POS 2
#define APP_POS 3    
#define STREAMID_POS 4

#define RTMPHEADER "rtmp://"

