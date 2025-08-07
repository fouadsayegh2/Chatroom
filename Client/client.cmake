set(SOURCES
    Client/main.cpp
    Client/client.cpp
)

set(HEADERS
    Client/client.h
)

add_executable(client ${SOURCES} ${HEADERS})