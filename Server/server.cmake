set(SOURCES
    Server/main.cpp
    Server/server.cpp
)

set(HEADERS
    Server/server.h
)

add_executable(server ${SOURCES} ${HEADERS})