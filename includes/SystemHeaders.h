/*
 * Модуль SystemHeaders содержит основные заголовочные файлы
 * для обеспечения клиент-серверного взаимодействия, а также
 * некоторые ф-и для работы со строками, временем и пр.
 * Этот модуль включается в след. модули: botCore, clientCore, serverCore, PolizElem
 */

#ifndef SYSTEM_HEADERS_H
#define SYSTEM_HEADERS_H

#include <asm-generic/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#ifdef __CPP_FILE__
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#else
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#endif

#include <fcntl.h>

#endif
