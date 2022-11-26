//sgn
#pragma once

/*
 * HttpClient.h
 *
 *  Created on: 16-Feb-2020
 *      Author: tstone10
 */


#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <queue>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <curl/curl.h>

#define	HTTP_REQ_TYPE_DWLD	(1)
#define	HTTP_REQ_TYPE_UPLD	(2)
#define HTTP_REQ_TYPE_GET	(3)
#define HTTP_REQ_TYPE_POST	(4)
#define HTTP_REQ_TYPE_POST_FORM	(5)
#define MAX_BUFF_SIZE       (2048)
#define DWLD_FILE_NAME      "curl_download.bin"

    class HttpResponse {
    public:
        HttpResponse() {};
        virtual ~HttpResponse() {};
        virtual void onHttpSuccess(const std::string& strResp) = 0;
        virtual void onHttpFailure(const std::string& strErr ) = 0;
    };

    class HttpReqPkt;
    class HttpClient : public std::enable_shared_from_this<HttpClient> {
        Logger& mLogger;
        std::shared_ptr<HttpResponse> pListener;
        std::string strCAFile, strResp;

    public:
        typedef std::shared_ptr<HttpClient> Ptr;
        HttpClient(std::string strCaFile);
        virtual ~HttpClient();

        void uploadFile(int64_t iChatId, std::string fileName, std::string mimeType, std::string strUrl);
        void postReq(std::string strUrl, std::string strPLoad, const std::vector<std::string>& pHeaders, bool skipResp = false);
        void postReqFormData(std::string strUrl, std::map<std::string, std::string> formData, int64_t iChatId, unsigned int iOrderNo);
        void subscribeListener(std::shared_ptr<HttpResponse> pObj) { pListener = pObj; }
        void pushToQ(HttpReqPkt* pReqPkt);

        std::shared_ptr<HttpClient> getSharedPtr() { return shared_from_this(); }

        static size_t write_function(char* ptr, size_t size, size_t nmemb, void* userdata);
        static void* run(void *pArg);
        static HttpReqPkt* readFromQ();

        static std::queue<HttpReqPkt*> reqQ;
        static std::mutex mtxgQ;
        static std::condition_variable mtxgCond;        
    };

    class HttpReqPkt {
        std::vector<std::string> headers;
        std::string strUrl, strFile, strData, strCmd;
        int reqType, cmdNo, iOrderNo;
        int64_t iChatId;
		bool skipResp;

        std::shared_ptr<HttpClient> pHttpClient;
        std::map<std::string, std::string> mapFormData;

    public:
        HttpReqPkt() : reqType{ HTTP_REQ_TYPE_POST }, cmdNo{ 0 }, iOrderNo{ 0 }, iChatId{ 0 }, skipResp{ false } {}
        virtual ~HttpReqPkt() {}

        std::vector<std::string> getHeaders() { return headers; }
        std::string getUrl() { return strUrl; }
        int getReqType() { return reqType; }
        int getCmdNo() { return cmdNo; }
		bool getSkipResp() { return skipResp; }
        std::string getUserData() { return strData; }
        HttpClient::Ptr getClient() { return pHttpClient; }
        std::map<std::string, std::string> getFormData() { return mapFormData; }
        std::string getTgtFile() { return strFile; }
        std::string getCmd() { return strCmd; }

        void addHeader(std::string strHdr) { headers.push_back(strHdr); }
        void setUrl(std::string url) { strUrl = url; }
        void setCmd(std::string cmd) { strCmd = cmd; }
        void setReqType(int iReqType) { reqType = iReqType; }
        void setCmdNo(int iCmdNo) { cmdNo = iCmdNo; }
        void setUserData(std::string strCnt) { strData = strCnt; }
        void setTgtFile(std::string file_name) { strFile = file_name; }
        void setSkipResp(bool pSkip) { skipResp = pSkip; }
        void setClient(HttpClient::Ptr _HttpClient) {pHttpClient = _HttpClient; }
        void setFormData(std::map<std::string, std::string> formData) { mapFormData = formData; }
    };
