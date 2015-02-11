
#ifndef __HPRECORD_FACTORY_H__
#define __HPRECORD_FACTORY_H__

#ifdef HPRF_EXPORT
#define HPRF_API _declspec(dllexport)
#elif HPRF_DLL
#define HPRF_API _declspec(dllimport)
#else
#define HPRF_API
#endif

typedef enum
{
	HPRF_FILE_MP4=0,
	HPRF_FILE_MKV=1,
	HPRF_FILE_FLV=2,
	HPRF_FILE_AVI=3,
	HPRF_FILE_AVMS=4,
	HPRF_FILE_MP3=5,
}HPRF_FILE_TYPE;


typedef void (*HPRF_CALLBACK_NEW_FILE)(void*pUserData,void*pHandle,char*cszNewFilePathName,char*cszLastFilePathName,unsigned long ulLastFileTime,unsigned long ulLastFileSize);

typedef void (*HPRF_CALLBACK_AAC_DATA)(void*pUserData,void*pHandle,void *pData,int nLen,unsigned long ulTimestamp);

/*���ò������ļ��ص�����������ᴥ�����˻ص�
 *1.���ļ���С�򳤶ȴﵽ���õ����ֵʱ���Զ��ָ���µ��ļ�
 *2.����Ƶ�����޷�¼��Ϊ���õ��ļ�����ʱ�����Զ������µ�*.avms�����ļ� 
 *3.����Ƶ�ֱ��ʸı�ʱ���������¼�Ƶ�avms�ļ������Զ��ָ���µ��ļ�*/
HPRF_API void  HPRF_SetNewFileCallback(void*pUserData,void*pHandle,HPRF_CALLBACK_NEW_FILE cb_new_file);

/*���û�ȡAAC��Ƶ�������ݰ��ص�������¼��MP4��MKVʱ��Ч,AAC�����ʽ����Ϣ{0x12,0x08}*/
HPRF_API void  HPRF_SetAACDataCallback(void*pUserData,void*pHandle,HPRF_CALLBACK_AAC_DATA cb_aac_data);

/*MaxTime����Ϊ��λ��ϵͳ�ɽ��ܵ����ֵ7200��Ĭ��ֵ3600��
 *ulMaxSize��ϵͳ�ɽ��ܵ����ֵ2*1024*1024*1024��Ĭ��ֵ1024*1024*1024��
 *MaxTime����MaxSize ����һ���ﵽ���ֵ�����Զ��и��ļ�*/
HPRF_API void  HPRF_SetMaxTimeSize(unsigned long ulMaxTime,unsigned long ulMaxSize);

/*��ȡ��ǰ�ļ���¼�ƴ�С�����ȣ��ڵ���HPRF_Closeǰ��Ч������Ѿ����Զ��и�򷵻ص����и�󣬵�ǰ����¼�Ƶ����ļ����ȼ���С*/
HPRF_API void  HPRF_GetCurFileTimeSize(void*pHandle,unsigned long& ulFileTime,unsigned long& ulFileSize);

/*��ʼ��¼�񹤳�*/
HPRF_API void  HPRF_Init(void);

/*�ͷ�¼�񹤳�*/
HPRF_API void  HPRF_UnInit(void);

/*��ʼ¼��*/
HPRF_API void* HPRF_Open(char*cszPathName,HPRF_FILE_TYPE type=HPRF_FILE_MP4);

/*����¼��*/
HPRF_API void  HPRF_Close(void*pHandle);

/*д��Videcԭʼ����,����ͷ��Ϣ������0Ϊд��ɹ�����0Ϊд��ʧ��*/
HPRF_API int   HPRF_WriteVideo(void*pHandle,void *pData,int nLen);

/*д��Audecԭʼ����,����ͷ��Ϣ������0Ϊд��ɹ�����0Ϊд��ʧ��*/
HPRF_API int   HPRF_WriteAudio(void*pHandle,void *pData,int nLen);

/*д����ƵPCM���ݣ�������¼�Ʒ�avms��Ƶʱ��Ч������0Ϊд��ɹ�����0Ϊд��ʧ��*/
HPRF_API int   HPRF_WriteAudioPCMData(void*pHandle,short*pSamples,int nSamples,int nSampleRate,unsigned long ulTimestamp);


#endif
