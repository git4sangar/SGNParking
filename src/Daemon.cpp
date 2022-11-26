//sgn

#include <iostream>
#include <unistd.h>     // fork()
#include <stdlib.h>     // exit()
#include <memory>
#include <curl/curl.h>
#include <pthread.h>

#include "Logger.h"
#include "Constants.h"
#include "BaseBot.h"
#include "HttpClient.h"

#include "HomePage.h"
#include "EntryPage.h"
#include "ExitPage.h"
#include "AdminPage.h"

std::map<std::string, BaseBot::Ptr>     BaseBot::mPages;
std::map<int64_t, VInfo::VehicleType>   BaseBot::mVType;

void init() {
    std::stringstream ss;
    ss << BOT_ROOT_PATH << BOT_DB_PATH << BOT_DB_NAME; 
    DBInterface::Ptr pDB        = std::make_shared<DBInterface>(ss.str());

    HomePage::Ptr pHomePage = std::make_shared<HomePage>(pDB);
    BaseBot::addPage(HOME_PAGE,     pHomePage);
    BaseBot::addPage(START_PAGE,    pHomePage);
    BaseBot::addPage(START_SLASH,   pHomePage);
    BaseBot::addPage(START_SLASH,   pHomePage);

    EntryPage::Ptr pEntryPage = std::make_shared<EntryPage>(pDB);
    BaseBot::addPage(ENTRY_PAGE,    pEntryPage);
    BaseBot::addPage(VEHICLE_CAR,   pEntryPage);
    BaseBot::addPage(VEHICLE_BIKE,  pEntryPage);

    ExitPage::Ptr pExitPage = std::make_shared<ExitPage>(pDB);
    BaseBot::addPage(EXIT_PAGE, pExitPage);

    AdminPage::Ptr  pAdminPage = std::make_shared<AdminPage>(pDB);
    BaseBot::addPage(ADMIN_PAGE,        pAdminPage);
    BaseBot::addPage(REPORTS_DAILY,     pAdminPage);
    BaseBot::addPage(REPORTS_WEEKLY,    pAdminPage);
    BaseBot::addPage(REPORTS_MONTHLY,   pAdminPage);
    BaseBot::addPage(REPORTS_ANNUAL,    pAdminPage);

    curl_global_init(CURL_GLOBAL_ALL);
    pthread_t curl_thread_id;
    pthread_create(&curl_thread_id, NULL, &HttpClient::run, NULL);
    pthread_detach(curl_thread_id);
}

int main() {
    pid_t pid;
    pid = fork(); 
    //  Exit the parent process
    if(pid > 0) {
        Logger::getInstance() << "Exiting pid " << getpid() << std::endl;
        exit(EXIT_SUCCESS);
    }

    //  -ve return means error
    if(pid < 0) {
        Logger::getInstance() << "Error creating child process" << std::endl;
        return -1;
    }
    Logger::getInstance() << "Successfully started client process" << std::endl;

    init();
    TelegramBot::Ptr pBot   = std::make_shared<TelegramBot>();
    pBot->init(std::string(BOT_ROOT_PATH) + std::string(CA_CERT_FILE));
    pBot->startGettingUpdates(0, 10);

    while(true) sleep(3600);
    return 0;
}
