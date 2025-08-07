cmake_minimum_required(VERSION 3.10)
project(chatroom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    

include(Server/server.cmake)
include(Client/client.cmake)