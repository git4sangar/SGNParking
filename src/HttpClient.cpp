//sgn
/*
 * HttpClient.cpp
 *
 *  Created on: 16-Feb-2020
 *      Author: tstone10
 */


#include <queue>
#include <thread>
#include <string.h>
#include <curl/curl.h>
#include <sstream>
#include <memory>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "Logger.h"
#include "HttpClient.h"

	std::queue<HttpReqPkt*> HttpClient::reqQ;
	std::mutex HttpClient::mtxgQ;
	std::condition_variable HttpClient::mtxgCond;

	HttpClient::HttpClient(std::string strCaFile)
		: mLogger{ Logger::getInstance() }
		, pListener{ nullptr }
		, strCAFile{ strCaFile } {}

	HttpClient :: ~HttpClient() {
		curl_global_cleanup();
	}

	void HttpClient::postReq(std::string strUrl, std::string strPLoad, const std::vector<std::string>& pHeaders, bool skipResp) {
		if (strPLoad.empty()) {
			mLogger << "HttpClient: Invalid post payload" << std::endl;
			return;
		}
		HttpReqPkt* pRqPkt = new HttpReqPkt();
		pRqPkt->setClient(shared_from_this());
		pRqPkt->setReqType(HTTP_REQ_TYPE_POST);
		pRqPkt->setUrl(strUrl);
		pRqPkt->setUserData(strPLoad);
		pRqPkt->setSkipResp(skipResp);

		pRqPkt->addHeader(std::string("Accept: application/json"));
		pRqPkt->addHeader(std::string("Content-Type: application/json"));
        for(const auto& header : pHeaders) pRqPkt->addHeader(header);

        mLogger << "Post Req: " << strUrl << std::endl;
		pushToQ(pRqPkt);
	}

	void HttpClient::postReqFormData(std::string strUrl, std::map<std::string, std::string> formData, int64_t iChatId, unsigned int iOrderNo) {
		HttpReqPkt* pRqPkt = new HttpReqPkt();

		pRqPkt->setClient(getSharedPtr());
		pRqPkt->setReqType(HTTP_REQ_TYPE_POST_FORM);
		pRqPkt->setUrl(strUrl);
		pRqPkt->setFormData(formData);

		pRqPkt->addHeader(std::string("Content-Type: application/x-www-form-urlencoded"));

        mLogger << "Post Form Req: " << strUrl << std::endl;
		pushToQ(pRqPkt);
	}

	void HttpClient::uploadFile(int64_t iChatId, std::string fileName, std::string mimeType, std::string strUrl) {
		HttpReqPkt* pRqPkt = new HttpReqPkt();
		pRqPkt->setClient(getSharedPtr());
		pRqPkt->setReqType(HTTP_REQ_TYPE_UPLD);
		pRqPkt->setUrl(strUrl);
		pRqPkt->setTgtFile(fileName);
		pRqPkt->setCmd("upload_logs");

		//	Prepare the headers
		pRqPkt->addHeader(std::string("Content-Type: multipart/form-data"));
        mLogger << "Upload File Req : " << strUrl << std::endl;
		pushToQ(pRqPkt);
	}

	void HttpClient::pushToQ(HttpReqPkt* pReqPkt) {
		if (pReqPkt) {
			std::lock_guard<std::mutex> lck(mtxgQ);
			reqQ.push(pReqPkt);
			mtxgCond.notify_one();
		}
	}

	HttpReqPkt* HttpClient::readFromQ() {
		HttpReqPkt* pReqPkt;
		std::unique_lock<std::mutex> lck(mtxgQ);
		if (reqQ.empty()) {
			mtxgCond.wait(lck);
		}
		pReqPkt = reqQ.front();
		reqQ.pop();
		return pReqPkt;
	}

	void* HttpClient::run(void *pArg) {
		HttpReqPkt* pReqPkt = NULL;
		long respCode = 0;
		std::string file_name;
		CURLcode res = CURLE_OK;
		struct curl_slist* curl_hdrs;
		char pPostPayLoad[MAX_BUFF_SIZE];
        HttpClient::Ptr pThis;

		curl_mime* form = NULL;
		curl_mimepart* field = NULL;
		std::map<std::string, std::string> formData;
		std::map<std::string, std::string>::iterator itrFormData;

        Logger& logger  = Logger::getInstance();
        logger << "HttpClient Curl thread started" << std::endl;

		while (1) {
			form = NULL; field = NULL; curl_hdrs = NULL; formData.clear();
			pReqPkt = HttpClient::readFromQ();
			pThis = pReqPkt->getClient();
            //logger << "HttpClient: Got HTTP req: " << pReqPkt->getUrl() << std::endl;

			CURL* curl = NULL;
			curl = curl_easy_init();
			if (curl && pThis->pListener) {
				curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0L);
				curl_easy_setopt(curl, CURLOPT_URL, pReqPkt->getUrl().c_str());

				//  Set the header
				std::vector<std::string> headers = pReqPkt->getHeaders();
				for (std::string header : headers) {
					curl_hdrs = curl_slist_append(curl_hdrs, header.c_str());
				}
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_hdrs);

				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10);

				//  For verifying server
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
				curl_easy_setopt(curl, CURLOPT_CAINFO, pThis->strCAFile.c_str());

				switch (pReqPkt->getReqType()) {

				case HTTP_REQ_TYPE_UPLD:
					file_name = pReqPkt->getTgtFile();
					form = curl_mime_init(curl);
					field = curl_mime_addpart(form);
					curl_mime_name(field, "file");
					curl_mime_filedata(field, file_name.c_str());
					curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
					break;

				case HTTP_REQ_TYPE_POST:
					memset(pPostPayLoad, 0, MAX_BUFF_SIZE);
					//	Need to copy to a buffer & make POST request. Otherwise not working
					strncpy(pPostPayLoad, pReqPkt->getUserData().c_str(), (MAX_BUFF_SIZE - 1));
					curl_easy_setopt(curl, CURLOPT_POST, 1L);
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pPostPayLoad);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&pThis->strResp);
					break;

				case HTTP_REQ_TYPE_POST_FORM:
					curl_easy_setopt(curl, CURLOPT_POST, 1L);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&pThis->strResp);

					formData = pReqPkt->getFormData();
					std::stringstream ss; std::string prefix;
					for (prefix.clear(), itrFormData = formData.begin(); itrFormData != formData.end(); itrFormData++, prefix = "&") {
						ss << prefix << itrFormData->first << "=";

						strcpy(pPostPayLoad, itrFormData->second.c_str());
						ss << curl_easy_escape(curl, pPostPayLoad, 0);
					}
					strcpy(pPostPayLoad, ss.str().c_str());
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pPostPayLoad);
					break;
				}

				res = curl_easy_perform(curl);

				CURLcode infoResp = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respCode);
				if (CURLE_OK == res && CURLE_OK == infoResp && respCode >= 200 && respCode <= 299) {
					if(!pReqPkt->getSkipResp())
						pThis->pListener->onHttpSuccess(pThis->strResp);
				}
				else {
					logger << "On Curl Fail : Resp " << infoResp << ", res : " << res << ", resCode : " << respCode << std::endl;
					pThis->pListener->onHttpFailure("");
				}
			}
			if (curl_hdrs) { curl_slist_free_all(curl_hdrs); curl_hdrs = NULL; }
			if (curl) curl_easy_cleanup(curl);
			delete pReqPkt;
			pThis->strResp.clear();
		}
        return nullptr;
	}

	size_t HttpClient::write_function(char* ptr, size_t size, size_t nmemb, void* userdata) {
		std::string* pStrRsp = reinterpret_cast<std::string*>(userdata);
		int iTotal = (size * nmemb);

		//  Notification app sends packets of 0 length. So skip those.
		if (0 < iTotal) {
			char* pBuffer = new char[1 + iTotal];
			memcpy(pBuffer, ptr, iTotal);
			pBuffer[iTotal] = '\0';

			std::stringstream ss;
			ss << *pStrRsp << pBuffer;
			*pStrRsp = ss.str();

			delete[] pBuffer;
		}
		return iTotal;
	}


	/*HttpReqPkt* HttpClient::genericUpdate(std::string strUrl, std::string strFolder, std::string strCmd, int cmdNo) {
		HttpReqPkt* pRqPkt = new HttpReqPkt();
		pRqPkt->setReqType(HTTP_REQ_TYPE_DWLD);
		pRqPkt->setUrl(strUrl);
		pRqPkt->setTgtFile(strFolder);
		pRqPkt->setCmdNo(cmdNo);
		pRqPkt->setCmd(strCmd);
		pRqPkt->addHeader(std::string("Content-Type: application/zip"));
		return pRqPkt;
	}

	void HttpClient::smartMeterUpdate(int cmdNo, std::string strUrl) {
		if (strUrl.empty() || 0 > cmdNo) {
			std::cout << "HttpClient: Smart meter update URL empty" << std::endl;
			return;
		}
		HttpReqPkt* pRqPkt = genericUpdate(strUrl, std::string(TECHNO_SPURS_APP_FOLDER), "smart_meter_update", cmdNo);
		std::cout << "HttpClient: Making Smart meter update request with CmdNo: " << cmdNo << std::endl;
		pushToQ(pRqPkt);
	}

	void HttpClient::jabberClientUpdate(int cmdNo, std::string strUrl) {
		if (strUrl.empty() || 0 > cmdNo) {
			std::cout << "HttpClient: Jabber client update URL empty" << std::endl;
			return;
		}
		HttpReqPkt* pRqPkt = genericUpdate(strUrl, std::string(TECHNO_SPURS_JABBER_FOLDER), "jabber_client_update", cmdNo);
		std::cout << "HttpClient: Making Jabber client update request with CmdNo: " << cmdNo << std::endl;
		pushToQ(pRqPkt);
	}

	void HttpClient::watchDogUpdate(int cmdNo, std::string strUrl) {
		if (strUrl.empty() || 0 > cmdNo) {
			std::cout << "HttpClient: Watchdog update update URL empty" << std::endl;
			return;
		}
		HttpReqPkt* pRqPkt = genericUpdate(strUrl, std::string(TECHNO_SPURS_WDOG_FOLDER), "watchdog_update", cmdNo);
		std::cout << "HttpClient: Making Watchdog update request with CmdNo: " << cmdNo << std::endl;
		pushToQ(pRqPkt);
	}*/
