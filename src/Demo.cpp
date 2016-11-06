
#include <stdio.h>
#include "TDBAPI.h"
#include "iostream"
#include <string.h>
#include <algorithm>
#include <assert.h>
using namespace std;

long GetTickCount()
{
	time_t now=time(NULL);
	return (long)now;
}

#ifdef NDEBUG

#define AssertEx(expr) expr;

#else
#define AssertEx(expr) {int n = (int)(expr); assert(n);} 
#endif

#define ELEMENT_COUNT(arr) (sizeof(arr)/sizeof(*arr))

void GetKData(THANDLE hTdb, const char* szCode, const char* szMarket, int nBeginDate, int nEndDate, int nCycle, int nUserDef, int nCQFlag, int nAutoComplete);
void GetTickData(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate);//�������̵�tick
void GetTransaction(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate); //��ʳɽ�
void GetOrder(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate);//���ί��
void GetOrderQueue(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate);//ί�ж���
void UseEZFFormula(THANDLE hTdb);
void GetCodeTable(THANDLE hTdb, char* szMarket);

std::string int2str(int n)
{
	char szBuf[32];
	snprintf(szBuf, sizeof(szBuf)/sizeof(szBuf[0]), "%d", n);
	return std::string(szBuf);
}

std::string array2str(const int* arr, int len)
{
	std::string str;
	for (int i=0; i<len; i++)
	{
		if (i == len-1)
		{
			str += int2str(arr[i]) + " ";
		}
		else 
		{
			str += int2str(arr[i]) + ",";
		}
	}
	return str;
}

int main(int argc, char** argv)
{
	THANDLE hTdb = NULL;	

	//���÷�������Ϣ
	OPEN_SETTINGS settings = {0};
	strcpy(settings.szIP,  "10.100.4.172");
	strcpy(settings.szPort, "10301");
	strcpy(settings.szUser, "1");
	strcpy(settings.szPassword,  "1");
	settings.nRetryCount = 15;
	settings.nRetryGap = 1;
	settings.nTimeOutVal = 1;

	//Proxy
	TDB_PROXY_SETTING proxy_settings;
	proxy_settings.nProxyType = TDB_PROXY_HTTP11;
	strcpy(proxy_settings.szProxyHostIp,   "10.100.3.42");
	sprintf(proxy_settings.szProxyPort, "%d",12345);
	strcpy(proxy_settings.szProxyUser,   "1");
	strcpy(proxy_settings.szProxyPwd,   "1");


	TDBDefine_ResLogin LoginRes = {0};
	//TDB_OpenProxy
	//hTdb = TDB_OpenProxy(&settings,&proxy_settings,&LoginRes);

	hTdb = TDB_Open(&settings, &LoginRes);

	if (!hTdb)
	{
		printf("����ʧ�ܣ�");
	}

	//TDB_GetCodeInfo
	{
		const TDBDefine_Code* pCode = TDB_GetCodeInfo(hTdb, "000001.SZ", "SZ-2-0");
		printf("-------------�յ�������Ϣ----------------------------\n");
		printf("���������� chWindCode:%s \n", pCode->chCode);			
		printf("�г����� chWindCode:%s \n", pCode->chMarket);
		printf("֤ȯ�������� chWindCode:%s \n", pCode->chCNName);
		printf("֤ȯӢ������ chWindCode:%s \n", pCode->chENName);
		printf("֤ȯ���� chWindCode:%d \n", pCode->nType);
	}
	/*************************** ��������  ***********************************/
	//code table
	{
		TDBDefine_Code* pCode = NULL;
		int pCount = 0;
		TDB_GetCodeTable(hTdb,"SZ",&pCode,&pCount);
		if (pCount && pCode)
		{
			for (int i = 0; i < pCount; i++)
			{
				printf("-------------code table ----------------------------\n");
				printf("chWindCode:%s \n", pCode[i].chCode);			
				printf("chWindCode:%s \n", pCode[i].chMarket);
				printf("chWindCode:%s \n", pCode[i].chCNName);
				printf("chWindCode:%s \n", pCode[i].chENName);
				printf("chWindCode:%d \n", pCode[i].nType);
			}
		}
		TDB_Free(pCode);
	}
	{
	//	GetKData(hTdb, "IF1602.CF", "CF-1-0", 20150910, 20150915, CYC_MINUTE, 0, 0, 0);//KLine for one minute
		GetKData(hTdb, "600715.SH", "SH-2-0", 20151126, 20151126, CYC_MINUTE, 0, 0, 1);//autocomplete k-minute
		GetTickData(hTdb, "000001.sz", "SZ-2-0", 20150910);//tick
		GetTransaction(hTdb, "000001.sz", "SZ-2-0", 20150910);//Transaction
		GetOrder(hTdb, "000001.sz", "SZ-2-0", 20150910);//Order
		GetOrderQueue(hTdb, "000001.sz", "SZ-2-0", 20150910);//OrderQueue
		UseEZFFormula(hTdb);//test for formula
	}

	printf("�����������������");
	getchar();
	int nRet = -1;
	if (hTdb)
		nRet = TDB_Close(hTdb);

	return 0;
}

//��������
void GetCodeTable(THANDLE hTdb, char* szMarket)
{
	TDBDefine_Code* pCodetable = NULL;
	int pCount;
	bool outPutTable = true;
	int ret = TDB_GetCodeTable(hTdb, szMarket, &pCodetable, &pCount);

	if (ret == TDB_NO_DATA)
	{
		printf("�޴����");
		return;
	}
	printf("---------------------------Code Table--------------------\n");
	printf("�յ������������%d��\n\n",pCount);
	//���
	if(outPutTable)
	{
		for (int i=0;i<pCount;i++)
		{
			printf("���������� chWindCode:%s \n", pCodetable[i].chCode);			
			printf("�г����� chWindCode:%s \n", pCodetable[i].chMarket);
			printf("֤ȯ�������� chWindCode:%s \n", pCodetable[i].chCNName);
			printf("֤ȯӢ������ chWindCode:%s \n", pCodetable[i].chENName);
			printf("֤ȯ���� chWindCode:%d \n", pCodetable[i].nType);
			printf("----------------------------------------\n");
		}
	}
	//�ͷ�
	TDB_Free(pCodetable);
}

void GetKData(THANDLE hTdb, const char* szCode, const char* szMarket, int nBeginDate, int nEndDate, int nCycle, int nUserDef, int nCQFlag, int nAutoComplete)
{
	//����K��
	TDBDefine_ReqKLine* req = new TDBDefine_ReqKLine;
	strncpy(req->chCode, szCode, ELEMENT_COUNT(req->chCode));
	strncpy(req->chMarketKey, szMarket, ELEMENT_COUNT(req->chMarketKey));

	req->nCQFlag = (REFILLFLAG)nCQFlag;//��Ȩ��־�����û�����
	req->nBeginDate = nBeginDate;//��ʼ����
	req->nEndDate = nEndDate;//��������
	req->nBeginTime = 0;//��ʼʱ��
	req->nEndTime = 0;//����ʱ��

	req->nCycType = (CYCTYPE)nCycle;
	req->nCycDef = 0;
	req->nAutoComplete = nAutoComplete;
	
	//���ؽṹ��ָ��
	TDBDefine_KLine* kLine = NULL;
	//������
	int pCount;
	//API����K��
	TDB_GetKLine(hTdb,req,&kLine,&pCount);
	delete req;
	req = NULL;

	printf("---------------------------K Data--------------------\n");
	printf("����������%d,��ӡ 1/100 ��\n\n",pCount);
	for(int i=0;i<pCount;)
	{
		printf("WindCode:%s\n Code:%s\n Date:%d\n Time:%d\n Open:%d\n High:%d\n Low:%d\n Close:%d\n Volume:%lld\n Turover:%lld\n MatchItem:%d\n Interest:%d\n",
			kLine[i].chWindCode,kLine[i].chCode,kLine[i].nDate,kLine[i].nTime,kLine[i].nOpen,kLine[i].nHigh,kLine[i].nLow,kLine[i].nClose,
			kLine[i].iVolume,kLine[i].iTurover,kLine[i].nMatchItems,kLine[i].nInterest);
		i +=100;
	}
	//�ͷ�
	TDB_Free(kLine);
}

//tick
void GetTickData(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate)
{
	//������Ϣ
	TDBDefine_ReqTick req = {0};
	strncpy(req.chCode, szCode, sizeof(req.chCode)); //����д�������ȡ�Ĺ�Ʊ����
	strncpy(req.chMarketKey, szMarket, sizeof(req.chMarketKey));
	req.nDate = nDate;
	req.nBeginTime = 0;
	req.nEndTime = 0;

	TDBDefine_Tick *pTick = NULL;
	int pCount;
	int ret = TDB_GetTick(hTdb,&req,&pTick, &pCount);

	printf("---------------------------------------Tick Data------------------------------------------\n");
	printf("���յ� %d ��Tick���ݣ� ��ӡ 1/100 ����\n", pCount);

	for(int i=0; i<pCount;)
	{
		TDBDefine_Tick& pTickCopy = pTick[i];
		printf("��ô��� chWindCode:%s \n", pTickCopy.chWindCode);
		printf("���� nDate:%d \n", pTickCopy.nDate);
		printf("ʱ�� nTime:%d \n", pTickCopy.nTime);

		printf("�ɽ��� nPrice:%d \n", pTickCopy.nPrice);
		printf("�ɽ��� iVolume:%lld \n", pTickCopy.iVolume);
		printf("�ɽ���(Ԫ) iTurover:%lld \n", pTickCopy.iTurover);
		printf("�ɽ����� nMatchItems:%d \n", pTickCopy.nMatchItems);
		printf(" nInterest:%d \n", pTickCopy.nInterest);

		printf("�ɽ���־: chTradeFlag:%c \n", pTickCopy.chTradeFlag);
		printf("BS��־: chBSFlag:%c \n", pTickCopy.chBSFlag);
		printf("���ճɽ���: iAccVolume:%lld \n", pTickCopy.iAccVolume);
		printf("���ճɽ���: iAccTurover:%lld \n", pTickCopy.iAccTurover);

		printf("��� nHigh:%d \n", pTickCopy.nHigh);
		printf("��� nLow:%d \n", pTickCopy.nLow);
		printf("���� nOpen:%d \n", pTickCopy.nOpen);
		printf("ǰ���� nPreClose:%d \n", pTickCopy.nPreClose);

		//�������ֶ�
		std::string strOut= array2str(pTickCopy.nAskPrice,ELEMENT_COUNT(pTickCopy.nAskPrice));
		printf("������ nAskPrice:%s \n", strOut.c_str());
		strOut= array2str((const int*)pTickCopy.nAskVolume,ELEMENT_COUNT(pTickCopy.nAskVolume));
		printf("������ nAskVolume:%s \n", strOut.c_str());
		strOut= array2str(pTickCopy.nBidPrice,ELEMENT_COUNT(pTickCopy.nBidPrice));
		printf("����� nBidPrice:%s \n", strOut.c_str());
		strOut= array2str((const int*)pTickCopy.nBidVolume,ELEMENT_COUNT(pTickCopy.nBidVolume));
		printf("������ nBidVolume:%s \n", strOut.c_str());
		printf("��Ȩƽ�������� nAskAvPrice:%d \n", pTickCopy.nAskAvPrice);
		printf("��Ȩƽ������� nBidAvPrice:%d \n", pTickCopy.nBidAvPrice);
		printf("�������� iTotalAskVolume:%lld \n", pTickCopy.iTotalAskVolume);
		printf("�������� iTotalBidVolume:%lld \n", pTickCopy.iTotalBidVolume);
#if 0
		//�ڻ��ֶ�
		printf("����� nSettle:%d \n", pTickCopy.nSettle);
		printf("�ֲ��� nPosition:%d \n", pTickCopy.nPosition);
		printf("��ʵ�� nCurDelta:%d \n", pTickCopy.nCurDelta);
		printf("����� nPreSettle:%d \n", pTickCopy.nPreSettle);
		printf("��ֲ� nPrePosition:%d \n", pTickCopy.nPrePosition);

		//ָ��
		printf("����Ȩָ�� nIndex:%d \n", pTickCopy.nIndex);
		printf("Ʒ������ nStocks:%d \n", pTickCopy.nStocks);
		printf("����Ʒ���� nUps:%d \n", pTickCopy.nUps);
		printf("�µ�Ʒ���� nDowns:%d \n", pTickCopy.nDowns);
		printf("��ƽƷ���� nHoldLines:%d \n", pTickCopy.nHoldLines);
#endif
		printf("-------------------------------\n");
		i += 1000;
	}
	//�ͷ�
	TDB_Free(pTick);
}

//��ʳɽ�
void GetTransaction(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
	//����
	TDBDefine_ReqTransaction req = {0};
	strncpy(req.chCode, szCode, sizeof(req.chCode)); //����д�������ȡ�Ĺ�Ʊ����
	strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
	req.nDate = nDate;
	req.nBeginTime = 0;
	req.nEndTime = 0;

	TDBDefine_Transaction *pTransaction = NULL;
	int pCount;
	int ret = TDB_GetTransaction(hTdb,&req, &pTransaction, &pCount);

	printf("---------------------------------------Transaction Data------------------------------------------\n");
	printf("�յ� %d ����ʳɽ���Ϣ����ӡ 1/10000 ��\n", pCount);

	for (int i=0; i<pCount; )
	{
		const TDBDefine_Transaction& trans = pTransaction[i];
		printf("�ɽ�ʱ��(Date): %d \n", trans.nDate);
		printf("�ɽ�ʱ��: %d \n", trans.nTime);
		printf("�ɽ�����: %c \n", trans.chFunctionCode);
		printf("ί�����: %c \n", trans.chOrderKind);
		printf("BS��־: %c \n", trans.chBSFlag);
		printf("�ɽ��۸�: %d \n", trans.nTradePrice);
		printf("�ɽ�����: %d \n", trans.nTradeVolume);
		printf("�������: %d \n", trans.nAskOrder);
		printf("�������: %d \n", trans.nBidOrder);
		printf("------------------------------------------------------\n");
#if 0
		printf("�ɽ����: %d \n", trans.nBidOrder);
#endif
		i += 10000;
	}
	//�ͷ�
	TDB_Free(pTransaction);
}

//���ί��
void GetOrder(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
	//����
	TDBDefine_ReqOrder req = {0};
	strncpy(req.chCode, szCode, sizeof(req.chCode)); //����д�������ȡ�Ĺ�Ʊ����
	strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
	req.nDate = nDate;
	req.nBeginTime = 0;
	req.nEndTime = 0;

	TDBDefine_Order *pOrder = NULL;
	int pCount;
	int ret = TDB_GetOrder(hTdb,&req, &pOrder, &pCount);
	
	printf("---------------------Order Data----------------------\n");
	printf("�յ� %d �����ί����Ϣ����ӡ 1/10000 ��\n", pCount);
	for (int i=0; i<pCount; )
	{
		const TDBDefine_Order& order = pOrder[i];
		printf("����ʱ��(Date): %d \n", order.nDate);
		printf("ί��ʱ��(HHMMSSmmm): %d \n", order.nTime);
		printf("ί�б��: %d \n", order.nOrder);
		printf("ί�����: %c \n", order.chOrderKind);
		printf("ί�д���: %c \n", order.chFunctionCode);
		printf("ί�м۸�: %d \n", order.nOrderPrice);
		printf("ί������: %d \n", order.nOrderVolume);
		printf("-------------------------------\n");

		i += 10000;
	}
	//�ͷ�
	TDB_Free(pOrder);
}

//ί�ж���
void GetOrderQueue(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
	//����
	TDBDefine_ReqOrderQueue req = {0};
	strncpy(req.chCode, szCode, sizeof(req.chCode)); //����д�������ȡ�Ĺ�Ʊ����
	strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
	req.nDate = nDate;
	req.nBeginTime = 0;
	req.nEndTime = 0;

	TDBDefine_OrderQueue *pOrderQueue = NULL;
	int pCount;
	TDB_GetOrderQueue(hTdb,&req, &pOrderQueue, &pCount);

	printf("-------------------OrderQueue Data-------------\n");
	printf("�յ� %d ��ί�ж�����Ϣ����ӡ 1/1000 ��\n", pCount);

	for (int i=0; i<pCount; i++)
	{
		const TDBDefine_OrderQueue& que = pOrderQueue[i];
		printf("����ʱ��(Date): %d \n", que.nDate);
		printf("����ʱ��(HHMMSS): %d \n", que.nTime);
		printf("��������('B':Bid 'A':Ask): %c \n", que.nSide);
		printf("�ɽ��۸�: %d \n", que.nPrice);
		printf("��������: %d \n", que.nOrderItems);
		printf("��ϸ����: %d \n", que.nABItems);
		printf("������ϸ: %s \n", array2str(que.nABVolume, que.nABItems).c_str());
		printf("-------------------------------\n");
		i += 1000;
	}
	//�ͷ�
	TDB_Free(pOrderQueue);
}

//ָ�깫ʽ
void UseEZFFormula(THANDLE hTdb)
{
	//��ʽ�ı�д����ο�<<TRANSEND-TS-M0001 �ױ๫ʽ������V1(2).0-20110822.pdf>>;
	std::string strName = "KDJ";
	std::string strContent = "INPUT:N(9), M1(3,1,100,2), M2(3);"
		"RSV:=(CLOSE-LLV(LOW,N))/(HHV(HIGH,N)-LLV(LOW,N))*100;"
		"K:SMA(RSV,M1,1);"
		"D:SMA(K,M2,1);"
		"J:3*K-2*D;";

	//��ӹ�ʽ�������������룬�����������д��󷵻�
	TDBDefine_AddFormulaRes* addRes = new TDBDefine_AddFormulaRes;
	int nErr = TDB_AddFormula(hTdb, strName.c_str(), strContent.c_str(),addRes);
	printf("Add Formula Result:%s",addRes->chInfo);

	//��ѯ�������ϵĹ�ʽ���ܿ������Ǹղ��ϴ���"KDJ"
	TDBDefine_FormulaItem* pEZFItem = NULL;
	int nItems = 0;
	//����Ϊ�ձ�ʾ��ѯ�����������еĹ�ʽ
	nErr = TDB_GetFormula(hTdb, NULL, &pEZFItem, &nItems);

	for (int i=0; i<nItems; i++)
	{
		std::string strNameInner(pEZFItem[i].chFormulaName, 0, sizeof(pEZFItem[i].chFormulaName));
		std::string strParam(pEZFItem[i].chParam, 0, sizeof(pEZFItem[i].chParam));
		printf("��ʽ���ƣ�%s, ����:%s \n", strNameInner.c_str(), strParam.c_str());
	}

	struct EZFCycDefine
	{
		char chName[8];
		int  nCyc;
		int  nCyc1;
	}
	EZFCyc[5]={
		{"����", 2, 0},
		{"30��", 0, 30},
		{"5����", 0, 5},
		{"1����", 0, 1},
		{"15��", 11, 15}};

		//��ȡ��ʽ�ļ�����
		TDBDefine_ReqCalcFormula reqCalc = {0};
		strncpy(reqCalc.chFormulaName, "KDJ", sizeof(reqCalc.chFormulaName));
		strncpy(reqCalc.chParam, "N=9,M1=3,M2=3", sizeof(reqCalc.chParam));
		strncpy(reqCalc.chCode, "000001.SZ", sizeof(reqCalc.chCode));
		strncpy(reqCalc.chMarketKey, "SZ-2-0", sizeof(reqCalc.chMarketKey));
		reqCalc.nCycType = (CYCTYPE)(EZFCyc[0].nCyc); //0��ʾ����
		reqCalc.nCycDef = EZFCyc[0].nCyc1; 
		reqCalc.nCQFlag = (REFILLFLAG)0;		  //��Ȩ��־
		reqCalc.nCalcMaxItems = 4000; //��������������
		reqCalc.nResultMaxItems = 100;	//���͵Ľ�������������

		TDBDefine_CalcFormulaRes* pResult = new TDBDefine_CalcFormulaRes;
		nErr = TDB_CalcFormula(hTdb, &reqCalc, pResult);
		//�жϴ������

		printf("��������: %d ��:\n", pResult->nRecordCount);
		char szLineBuf[1024] = {0};
		//����ֶ���
		for (int j=0; j<pResult->nFieldCount;j++)
		{
			std::cout << pResult->chFieldName[j] << "  ";
		}
		std::cout << endl << endl;
		//�������
		for (int i=0; i<pResult->nRecordCount; i++)
		{
			for (int j=0; j<pResult->nFieldCount;j++)
			{
				std::cout << (pResult->dataFileds)[j][i] << "  ";
			}
			std::cout << endl;
		}

		//ɾ��֮ǰ�ϴ��Ĺ�ʽָ��
		TDBDefine_DelFormulaRes pDel = {0};
		nErr = TDB_DeleteFormula(hTdb, "KDJ", &pDel);
		printf("ɾ��ָ����Ϣ:%s", pDel.chInfo);
		//�ͷ��ڴ�
		delete pEZFItem;
		TDB_ReleaseCalcFormula(pResult);
}
