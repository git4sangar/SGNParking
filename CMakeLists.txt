#sgn
cmake_minimum_required(VERSION 3.1)
project (SGNParking)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

message("project source dir: ${PROJECT_SOURCE_DIR}")

find_package(OpenSSL REQUIRED)

add_executable(SGNParking
    src/Daemon.cpp
    src/Logger.cpp
    src/DBInterface.cpp
    src/HttpClient.cpp
    src/TelegramBot.cpp
    src/AdminPage.cpp
    src/HomePage.cpp
    src/EntryPage.cpp
    src/ExitPage.cpp
)

#target_link_libraries(SGNParking sqlite3 SQLiteCpp crypto curl pthread)


 target_link_libraries(SGNParking
   SQLiteCpp
   sqlite3
   pthread
   dl
   ${Boost_LIBRARIES}
   curl
   OpenSSL::SSL
   OpenSSL::Crypto)


