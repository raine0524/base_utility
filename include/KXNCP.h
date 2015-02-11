#pragma once

#define AVCON_SERVER_VERSION		"6.3.5152"

//////////////////////////////////////////////////////////////////////////////////////
//									SYS												//
//////////////////////////////////////////////////////////////////////////////////////

#define IO_CONNECT_TYPE_CLIENT_SVC			100
#define IO_CONNECT_TYPE_CLIENT_NFS			103		//�����ϰ汾
#define IO_CONNECT_TYPE_CLIENT_MGW			104
#define IO_CONNECT_TYPE_CLIENT_VRS			105		//���¼���ϴ�
#define IO_CONNECT_TYPE_CLIENT_PLT			106		//���ƽ̨
#define IO_CONNECT_TYPE_CLIENT_NDU          107
#define IO_CONNECT_TYPE_CLIENT_NDD          108
#define IO_CONNECT_TYPE_CLIENT_NDI          109

#define IO_CONNECT_TYPE_MCU_ROUTER			200
#define IO_CONNECT_TYPE_NFS_ROUTER			201

#define SERVER_TYPE_MCU						300
#define SERVER_TYPE_MTS						301
#define SERVER_TYPE_MCU_MTS					302
#define SERVER_TYPE_GW						303
#define SERVER_TYPE_3GC						304
#define SERVER_TYPE_3GM						305
#define SERVER_TYPE_MP						306

#define DEV_AGENT_ID                        402
#define WEB_AGENT_ID                        403
#define REC_AGENT_ID                        404


//---------------------------------------------------------
// �ļ����䳣��
#define MAX_FILE_PACKET			65536 + 64
#define MAX_FILE_DATA_PACKET	65536
#define MAX_CMD_PACKET			32768 * 3

//=======================================================================================
// �������
//=======================================================================================
#define	ERR_NO							00000		//��ȷ

//---------------------------------------------------------------------------------------
// ϵͳ���������������
//---------------------------------------------------------------------------------------
#define	ERR_SYS_PROTOCOL				10000		//ͨѶЭ��汾��ƥ��
#define	ERR_SYS_UNKNOWN					10001		//δ֪����
#define	ERR_SYS_COMMAND					10002		//Э���������
#define	ERR_SYS_DATABASE				10003		//���ݿ����
#define	ERR_SYS_NETWORK					10004		//����Ͽ�(����ͨ·�Ͽ�)
#define	ERR_SYS_SERVERTYPE				10005		//���������ʹ���,��¼���������͵ķ�����
#define	ERR_SYS_LICEXPIRED              10006       //��������Ȩ����
//---------------------------------------------------------------------------------------
// ��ʱͨѶ��������
//---------------------------------------------------------------------------------------
#define	ERR_IMS_PROTOCOL				20000		//ͨѶЭ��汾��ƥ��
#define	ERR_IMS_INVALIDACCOUNT			20001		//��Ч���˻�(ϵͳû�и��˻�ID)
#define	ERR_IMS_INVALIDPASSWORD			20002		//��Ч������
#define	ERR_IMS_DUPLICATELOGIN			20003		//���˻��Ѿ���¼,�����ظ���¼


//----------------------------------------------------------------------------------------
// ¼��������������
//----------------------------------------------------------------------------------------
#define ERR_REC_NOSERVER				70000		//û�п��õ�¼�������
#define ERR_REC_SVRRES_BUSY				70001		//¼��������æ
#define ERR_REC_SVR_EXCEPTION			70002		//¼���������쳣
#define ERR_REC_SERVER_INVALIDTYPE		70003		//��Ч���˻�����
#define ERR_REC_SERVER_CANNOT_STOP		70004		//�޷�ֹͣ¼��
#define ERR_REC_STOP					70005		//�쳣ֹͣ¼��
#define ERR_REC_CUT						70006		//�и���Ƶ
#define ERR_REC_NO_CONF					70007		//����û��¼��
#define ERR_REC_CONF_CUT				70008		//����¼��ָ�
#define ERR_REC_SEVER_UNKNOWN			70099		//δ֪����(����)


//H323�������
#define  ERR_INVALID_DEVTYPE          80000     //�豸���ʹ���
#define  ERR_INVALID_DEVID            80001     //û��ָ����Dev�˺�
#define  ERR_DUPLICATE_REG            80002     //�˺��ظ���¼
#define  ERR_DUPLICATE_DEVID          80003     //������豸�˺�

