#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <list>
#include <iomanip>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "include/TDBAPI.h"
#include "include/TDBAPIStruct.h"

#define TYPE_Tick 0
#define TYPE_Transaction 1
#define TYPE_Order 2
#define TYPE_OrderQueue 3
#define TYPE_KLine_1min 4
#define TYPE_KLine_1day 5


using namespace std;


#define ELEMENT_COUNT(arr) (sizeof(arr)/sizeof(*arr))

/*
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

*/

void GetCodeTable(THANDLE hTdb, string output_file);

void GetData(THANDLE hTdb, string date, int type);

void GetKData(THANDLE hTdb, const char *szCode, const char *szMarket, int nBeginDate,
              int nEndDate, int nCycle, int nUserDef, int nCQFlag, int nAutoComplete,
              string output_file);

void GetStockTickData(THANDLE hTdb, const char *szCode, const char *szMarket, int nDate,
                      string output_file);//带买卖盘的tick
/*
void GetTransaction(THANDLE hTdb, const char* szCode, const char* szMarket, int
nDate); //逐笔成交
void GetOrder(THANDLE hTdb, const char* szCode, const char* szMarket, int nDate)
;//逐笔委托
void GetOrderQueue(THANDLE hTdb, const char* szCode, const char* szMarket, int n
                   Date);//委托队列
void UseEZFFormula(THANDLE hTdb);
void GetCodeTable(THANDLE hTdb, char* szMarket);
*/


int main(int argc, char *argv[]) {

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("tdb.conf", pt);


    string ip = pt.get<std::string>("TDBServer.ServerIp");
    string port = pt.get<std::string>("TDBServer.Port");
    string user = pt.get<std::string>("TDBServer.UserName");
    string password = pt.get<std::string>("TDBServer.Password");

    THANDLE hTdb = NULL;

    //设置服务器信息
    OPEN_SETTINGS settings = {0};
    strcpy(settings.szIP, ip.c_str());
    strcpy(settings.szPort, port.c_str());
    strcpy(settings.szUser, user.c_str());
    strcpy(settings.szPassword, password.c_str());

    settings.nRetryCount = 15;
    settings.nRetryGap = 1;
    settings.nTimeOutVal = 1;

    /*
    //Proxy
    TDB_PROXY_SETTING proxy_settings;
    proxy_settings.nProxyType = TDB_PROXY_HTTP11;
    strcpy(proxy_settings.szProxyHostIp, "10.100.3.42");
    sprintf(proxy_settings.szProxyPort, "%d", 12345);
    strcpy(proxy_settings.szProxyUser, "1");
    strcpy(proxy_settings.szProxyPwd, "1");
    */

    TDBDefine_ResLogin LoginRes = {0};
    //TDB_OpenProxy
    //hTdb = TDB_OpenProxy(&settings,&proxy_settings,&LoginRes);

    hTdb = TDB_Open(&settings, &LoginRes);

    if (!hTdb) {
        printf("connect error!");
        return -1;
    }

    //code table

    string code_table_file = "data/codetable.csv";
    GetCodeTable(hTdb, code_table_file);

    string tick = pt.get<std::string>("Date.Tick");
    string transaction = pt.get<std::string>("Date.Transaction");
    string order = pt.get<std::string>("Date.Order");
    string order_queue = pt.get<std::string>("Date.OrderQueue");
    string kline_1min = pt.get<std::string>("Date.KLine_1min");
    string kline_1day = pt.get<std::string>("Date.KLine_1day");


    vector<string> vec_tick;
    vector<string> vec_transaction;
    vector<string> vec_order;
    vector<string> vec_order_queue;
    vector<string> vec_kline_1min;
    vector<string> vec_kline_1day;

    boost::split(vec_tick, tick, boost::is_any_of(","));
    boost::split(vec_order, order, boost::is_any_of(","));
    boost::split(vec_transaction, transaction, boost::is_any_of(","));
    boost::split(vec_order_queue, order_queue, boost::is_any_of(","));
    boost::split(vec_kline_1min, kline_1min, boost::is_any_of(","));
    boost::split(vec_kline_1day, kline_1day, boost::is_any_of(","));

    vector<string>::iterator pos;
    for (pos = vec_kline_1min.begin(); pos != vec_kline_1min.end(); ++pos) {
        if ((*pos).length() != 8)  {
            cerr << "date length not match! " << endl;
            continue;
        }
        GetData(hTdb, *pos, TYPE_KLine_1min);
    }

    for (pos = vec_tick.begin(); pos != vec_tick.end(); ++pos) {
        if ((*pos).length() != 8) {
            cerr << "date length not match! " << endl;
            continue;
        }
        GetData(hTdb, *pos, TYPE_Tick);
    }

    /*
    //	GetKData(hTdb, "IF1602.CF", "CF-1-0", 20150910, 20150915, CYC_MINUTE, 0, 0, 0);//KLine for one minute
    // GetKData(hTdb, "600715.SH", "SH-2-0", 20151126, 20151126, CYC_MINUTE, 0, 0, 1);//autocomplete k-minute
    // GetStockTickData(hTdb, "000001.sz", "SZ-2-0", 20161104, "test.csv");//tick
    // GetTransaction(hTdb, "000001.sz", "SZ-2-0", 20150910);//Transaction
    // GetOrder(hTdb, "000001.sz", "SZ-2-0", 20150910);//Order
    // GetOrderQueue(hTdb, "000001.sz", "SZ-2-0", 20150910);//OrderQueue
    // UseEZFFormula(hTdb);//test for formula
    */

    if (hTdb) TDB_Close(hTdb);

    return 0;
}

void GetData(THANDLE hTdb, string date, int type) {
    string dir = "";
    switch (type) {
        case TYPE_Tick:
            dir = "data/intraday/tick/" + date + "/";
            break;
        case TYPE_Transaction:
            dir = "data/intraday/transaction/" + date + "/";
            break;
        case TYPE_Order:
            dir = "data/intraday/order/" + date + "/";
            break;
        case TYPE_OrderQueue:
            dir = "data/intraday/orderqueue/" + date + "/";
            break;
        case TYPE_KLine_1min:
            dir = "data/intraday/kline_1min/" + date + "/";
            break;
        case TYPE_KLine_1day:
            dir = "data/interday/kline_1day/" + date + "/";
        default:
            dir = "data/";
    }

    long int n_date = atol(date.c_str());
    list<string> markets;

    markets.push_back("SZ");
    markets.push_back("SH");

    list<string>::iterator it;
    for (it = markets.begin(); it != markets.end(); ++it) {
        TDBDefine_Code *pCode = NULL;
        int pCount = 0;

        TDB_GetCodeTable(hTdb, (*it).c_str(), &pCode, &pCount);
        if (pCount && pCode) {
            for (int i = 0; i < pCount; i++) {
                if ((pCode[i].nType == 0x10) || (pCode[i].nType == 0x11) || (pCode[i].nType == 0x12)) {
                    printf("code: %s, type: %d\n", pCode[i].chCode, pCode[i].nType);

                    int n_start_date = 0;
                    int n_end_date = 0;

                    switch (type) {
                        case TYPE_Tick:
                            GetStockTickData(hTdb, pCode[i].chWindCode, pCode[i].chMarket, n_date,
                                             dir + string(pCode[i].chCode) + ".csv");
                            break;
                        case TYPE_Transaction:

                            break;
                        case TYPE_Order:

                            break;
                        case TYPE_OrderQueue:

                            break;
                        case TYPE_KLine_1min:

                            GetKData(hTdb, pCode[i].chWindCode, pCode[i].chMarket, n_date, n_date, CYC_MINUTE,
                                     0, 0, 0, dir + string(pCode[i].chCode) + ".csv");
                            break;
                        case TYPE_KLine_1day:
                            n_start_date = atol((date + "01").c_str());
                            n_end_date = atol((date + "31").c_str());
                            GetKData(hTdb, pCode[i].chWindCode, pCode[i].chMarket, n_start_date, n_end_date, CYC_DAY,
                                     0, 1, 0, dir + string(pCode[i].chCode) + ".csv");
                            break;
                        default:
                            cerr << "unkown type!" << endl;
                    }
                }
            }
        }
        TDB_Free(pCode);
    }
}

void GetCodeTable(THANDLE hTdb, string output_file) {
    ofstream of_code_table;
    of_code_table.open(output_file, ios_base::out | ios_base::trunc);

    list<string> markets;

    markets.push_back("SZ");
    markets.push_back("SH");

    //output header
    of_code_table << "code, market, cnname, type" << endl;

    list<string>::iterator it;
    for (it = markets.begin(); it != markets.end(); ++it) {
        TDBDefine_Code *pCode = NULL;
        int pCount = 0;

        TDB_GetCodeTable(hTdb, (*it).c_str(), &pCode, &pCount);
        if (pCount && pCode) {
            for (int i = 0; i < pCount; i++) {
                of_code_table << pCode[i].chWindCode << ",";
                of_code_table << pCode[i].chCode << ",";
                of_code_table << pCode[i].chMarket << ",";
                of_code_table << pCode[i].chCNName << ",";
                of_code_table << pCode[i].nType << endl;
            }
        }

        TDB_Free(pCode);
    }

    of_code_table.close();
}


void GetKData(THANDLE hTdb, const char *szCode, const char *szMarket, int nBeginDate, int nEndDate,
              int nCycle, int nUserDef, int nCQFlag, int nAutoComplete, string output_file) {
    //请求K线
    TDBDefine_ReqKLine *req = new TDBDefine_ReqKLine;
    strncpy(req->chCode, szCode, ELEMENT_COUNT(req->chCode));
    strncpy(req->chMarketKey, szMarket, ELEMENT_COUNT(req->chMarketKey));

    req->nCQFlag = (REFILLFLAG) nCQFlag;//除权标志，由用户定义
    req->nBeginDate = nBeginDate;//开始日期
    req->nEndDate = nEndDate;//结束日期
    req->nBeginTime = 0;//开始时间
    req->nEndTime = 0;//结束时间

    req->nCycType = (CYCTYPE) nCycle;
    req->nCycDef = 0;
    req->nAutoComplete = nAutoComplete;

    //返回结构体指针
    TDBDefine_KLine *kLine = NULL;
    //返回数
    int pCount;
    //API请求K线
    TDB_GetKLine(hTdb, req, &kLine, &pCount);
    delete req;
    req = NULL;

    printf("received k line data: code：%s, market: %s, begin date: %d, end date: %d, cycle: %d, count: %d\n",
           szCode, szMarket, nBeginDate, nEndDate, nCycle, pCount);

    if (pCount < 1)
        return;

    ofstream of_output;
    of_output.open(output_file, ios::out | ios::trunc);

    of_output << setiosflags(ios::fixed) << setprecision(2);
    of_output << "code, date, time, open, high, low, close, volume, turnover, matchitem, interest" << endl;

    for (int i = 0; i < pCount; i++) {
        of_output << kLine[i].chCode << "," << kLine[i].nDate << "," << kLine[i].nTime << "," \
 << float(kLine[i].nOpen) / 10000 << "," << float(kLine[i].nHigh) / 10000 << "," \
 << float(kLine[i].nLow) / 10000 << "," << float(kLine[i].nClose) / 10000 << "," \
 << kLine[i].iVolume << "," << kLine[i].iTurover << "," << kLine[i].nMatchItems << "," \
 << kLine[i].nInterest << endl;
    }
    //释放
    TDB_Free(kLine);
}


//tick
void GetStockTickData(THANDLE hTdb, const char *szCode, const char *szMarket, int nDate, string output_file) {
    //请求信息
    TDBDefine_ReqTick req = {0};
    strncpy(req.chCode, szCode, sizeof(req.chCode)); //代码写成你想获取的股票代码
    strncpy(req.chMarketKey, szMarket, sizeof(req.chMarketKey));
    req.nDate = nDate;
    req.nBeginTime = 92500000;
    req.nEndTime = 0;

    TDBDefine_Tick *pTick = NULL;
    int pCount;
    int ret = TDB_GetTick(hTdb, &req, &pTick, &pCount);

    printf("received tick data: code：%s, market: %s, date: %d, count: %d\n", szCode, szMarket, nDate, pCount);
    if (pCount < 1)
        return;

    ofstream of_output;
    of_output.open(output_file, ios::out | ios::trunc);

    of_output << setiosflags(ios::fixed) << setprecision(2);
    of_output << "windcode, date, time, ";
    of_output << "price, volume, turnover, match_items, interest, trade_flag, bs_flag, acc_volume, acc_turnover,";
    of_output << "high, low, open, pre_close, ";

    list<string> price_volume;

    price_volume.push_back("bid_price");
    price_volume.push_back("bid_volume");
    price_volume.push_back("ask_price");
    price_volume.push_back("bid_volume");

    list<string>::iterator it;
    for (it = price_volume.begin(); it != price_volume.end(); ++it) {
        for (int i = 1; i <= 10; i++) {
            of_output << *it << i << ",";
        }
    }
    of_output << "ask_av_price, bid_av_price, total_ask_volume, total_bid_volume" << endl;

    for (int i = 0; i < pCount; i++) {
        TDBDefine_Tick &pTickCopy = pTick[i];
        of_output << pTickCopy.chWindCode << "," << pTickCopy.nDate << "," << pTickCopy.nTime << ",";
        of_output << float(pTickCopy.nPrice) / 10000 << "," << pTickCopy.iVolume << "," << pTickCopy.iTurover << "," \
                  << pTickCopy.nMatchItems << "," << pTickCopy.nInterest << ",";
        of_output << pTickCopy.chTradeFlag << "," << pTickCopy.chBSFlag << "," << pTickCopy.iAccVolume << "," \
                  << pTickCopy.iAccTurover << ",";
        of_output << float(pTickCopy.nHigh) / 10000 << "," << float(pTickCopy.nLow) / 10000 << "," \
                  << float(pTickCopy.nOpen) / 10000 << "," << float(pTickCopy.nPreClose) / 10000 << ",";

        int size = sizeof(pTickCopy.nBidPrice) / sizeof(*pTickCopy.nBidPrice);

        for (int i = 0; i < size; i++) {
            of_output << float(pTickCopy.nBidPrice[i]) / 10000 << ",";
        }

        for (int i = 0; i < size; i++) {
            of_output << pTickCopy.nBidVolume[i] << ",";
        }

        for (int i = 0; i < size; i++) {
            of_output << float(pTickCopy.nAskPrice[i]) / 10000 << ",";
        }

        for (int i = 0; i < size; i++) {
            of_output << pTickCopy.nAskVolume[i] << ",";
        }

        of_output << float(pTickCopy.nAskAvPrice) / 10000 << "," << float(pTickCopy.nBidAvPrice) / 10000 << "," \
                  << pTickCopy.iTotalAskVolume << "," << pTickCopy.iTotalBidVolume << endl;

    }

    of_output.close();
    TDB_Free(pTick);
}

/*
//逐笔成交
void GetTransaction(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
    //请求
    TDBDefine_ReqTransaction req = {0};
    strncpy(req.chCode, szCode, sizeof(req.chCode)); //代码写成你想获取的股票代码
    strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
    req.nDate = nDate;
    req.nBeginTime = 0;
    req.nEndTime = 0;

    TDBDefine_Transaction *pTransaction = NULL;
    int pCount;
    int ret = TDB_GetTransaction(hTdb,&req, &pTransaction, &pCount);

    printf("---------------------------------------Transaction Data------------------------------------------\n");
    printf("收到 %d 条逐笔成交消息，打印 1/10000 条\n", pCount);

    for (int i=0; i<pCount; )
    {
        const TDBDefine_Transaction& trans = pTransaction[i];
        printf("成交时间(Date): %d \n", trans.nDate);
        printf("成交时间: %d \n", trans.nTime);
        printf("成交代码: %c \n", trans.chFunctionCode);
        printf("委托类别: %c \n", trans.chOrderKind);
        printf("BS标志: %c \n", trans.chBSFlag);
        printf("成交价格: %d \n", trans.nTradePrice);
        printf("成交数量: %d \n", trans.nTradeVolume);
        printf("叫卖序号: %d \n", trans.nAskOrder);
        printf("叫买序号: %d \n", trans.nBidOrder);
        printf("------------------------------------------------------\n");
#if 0
		printf("成交编号: %d \n", trans.nBidOrder);
#endif
        i += 10000;
    }
    //释放
    TDB_Free(pTransaction);
}

//逐笔委托
void GetOrder(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
    //请求
    TDBDefine_ReqOrder req = {0};
    strncpy(req.chCode, szCode, sizeof(req.chCode)); //代码写成你想获取的股票代码
    strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
    req.nDate = nDate;
    req.nBeginTime = 0;
    req.nEndTime = 0;

    TDBDefine_Order *pOrder = NULL;
    int pCount;
    int ret = TDB_GetOrder(hTdb,&req, &pOrder, &pCount);

    printf("---------------------Order Data----------------------\n");
    printf("收到 %d 条逐笔委托消息，打印 1/10000 条\n", pCount);
    for (int i=0; i<pCount; )
    {
        const TDBDefine_Order& order = pOrder[i];
        printf("订单时间(Date): %d \n", order.nDate);
        printf("委托时间(HHMMSSmmm): %d \n", order.nTime);
        printf("委托编号: %d \n", order.nOrder);
        printf("委托类别: %c \n", order.chOrderKind);
        printf("委托代码: %c \n", order.chFunctionCode);
        printf("委托价格: %d \n", order.nOrderPrice);
        printf("委托数量: %d \n", order.nOrderVolume);
        printf("-------------------------------\n");

        i += 10000;
    }
    //释放
    TDB_Free(pOrder);
}

//委托队列
void GetOrderQueue(THANDLE hTdb, const char* szCode, const char* szMarketKey, int nDate)
{
    //请求
    TDBDefine_ReqOrderQueue req = {0};
    strncpy(req.chCode, szCode, sizeof(req.chCode)); //代码写成你想获取的股票代码
    strncpy(req.chMarketKey, szMarketKey, sizeof(req.chMarketKey));
    req.nDate = nDate;
    req.nBeginTime = 0;
    req.nEndTime = 0;

    TDBDefine_OrderQueue *pOrderQueue = NULL;
    int pCount;
    TDB_GetOrderQueue(hTdb,&req, &pOrderQueue, &pCount);

    printf("-------------------OrderQueue Data-------------\n");
    printf("收到 %d 条委托队列消息，打印 1/1000 条\n", pCount);

    for (int i=0; i<pCount; i++)
    {
        const TDBDefine_OrderQueue& que = pOrderQueue[i];
        printf("订单时间(Date): %d \n", que.nDate);
        printf("订单时间(HHMMSS): %d \n", que.nTime);
        printf("买卖方向('B':Bid 'A':Ask): %c \n", que.nSide);
        printf("成交价格: %d \n", que.nPrice);
        printf("订单数量: %d \n", que.nOrderItems);
        printf("明细个数: %d \n", que.nABItems);
        printf("订单明细: %s \n", array2str(que.nABVolume, que.nABItems).c_str());
        printf("-------------------------------\n");
        i += 1000;
    }
    //释放
    TDB_Free(pOrderQueue);
}

//指标公式
void UseEZFFormula(THANDLE hTdb)
{
    //公式的编写，请参考<<TRANSEND-TS-M0001 易编公式函数表V1(2).0-20110822.pdf>>;
    std::string strName = "KDJ";
    std::string strContent = "INPUT:N(9), M1(3,1,100,2), M2(3);"
            "RSV:=(CLOSE-LLV(LOW,N))/(HHV(HIGH,N)-LLV(LOW,N))*100;"
            "K:SMA(RSV,M1,1);"
            "D:SMA(K,M2,1);"
            "J:3*K-2*D;";

    //添加公式到服务器并编译，若不过，会有错误返回
    TDBDefine_AddFormulaRes* addRes = new TDBDefine_AddFormulaRes;
    int nErr = TDB_AddFormula(hTdb, strName.c_str(), strContent.c_str(),addRes);
    printf("Add Formula Result:%s",addRes->chInfo);

    //查询服务器上的公式，能看到我们刚才上传的"KDJ"
    TDBDefine_FormulaItem* pEZFItem = NULL;
    int nItems = 0;
    //名字为空表示查询服务器上所有的公式
    nErr = TDB_GetFormula(hTdb, NULL, &pEZFItem, &nItems);

    for (int i=0; i<nItems; i++)
    {
        std::string strNameInner(pEZFItem[i].chFormulaName, 0, sizeof(pEZFItem[i].chFormulaName));
        std::string strParam(pEZFItem[i].chParam, 0, sizeof(pEZFItem[i].chParam));
        printf("公式名称：%s, 参数:%s \n", strNameInner.c_str(), strParam.c_str());
    }

    struct EZFCycDefine
    {
        char chName[8];
        int  nCyc;
        int  nCyc1;
    }
            EZFCyc[5]={
            {"日线", 2, 0},
            {"30分", 0, 30},
            {"5分钟", 0, 5},
            {"1分钟", 0, 1},
            {"15秒", 11, 15}};

    //获取公式的计算结果
    TDBDefine_ReqCalcFormula reqCalc = {0};
    strncpy(reqCalc.chFormulaName, "KDJ", sizeof(reqCalc.chFormulaName));
    strncpy(reqCalc.chParam, "N=9,M1=3,M2=3", sizeof(reqCalc.chParam));
    strncpy(reqCalc.chCode, "000001.SZ", sizeof(reqCalc.chCode));
    strncpy(reqCalc.chMarketKey, "SZ-2-0", sizeof(reqCalc.chMarketKey));
    reqCalc.nCycType = (CYCTYPE)(EZFCyc[0].nCyc); //0表示日线
    reqCalc.nCycDef = EZFCyc[0].nCyc1;
    reqCalc.nCQFlag = (REFILLFLAG)0;		  //除权标志
    reqCalc.nCalcMaxItems = 4000; //计算的最大数据量
    reqCalc.nResultMaxItems = 100;	//传送的结果的最大数据量

    TDBDefine_CalcFormulaRes* pResult = new TDBDefine_CalcFormulaRes;
    nErr = TDB_CalcFormula(hTdb, &reqCalc, pResult);
    //判断错误代码

    printf("计算结果有: %d 条:\n", pResult->nRecordCount);
    char szLineBuf[1024] = {0};
    //输出字段名
    for (int j=0; j<pResult->nFieldCount;j++)
    {
        std::cout << pResult->chFieldName[j] << "  ";
    }
    std::cout << endl << endl;
    //输出数据
    for (int i=0; i<pResult->nRecordCount; i++)
    {
        for (int j=0; j<pResult->nFieldCount;j++)
        {
            std::cout << (pResult->dataFileds)[j][i] << "  ";
        }
        std::cout << endl;
    }

    //删除之前上传的公式指标
    TDBDefine_DelFormulaRes pDel = {0};
    nErr = TDB_DeleteFormula(hTdb, "KDJ", &pDel);
    printf("删除指标信息:%s", pDel.chInfo);
    //释放内存
    delete pEZFItem;
    TDB_ReleaseCalcFormula(pResult);
}
 */
