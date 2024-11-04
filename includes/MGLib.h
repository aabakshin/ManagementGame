/* 
 *	Модуль MGLib содержит вспомогательные функции общего назначения
 *	для реализации различных частей программы
 *	Этот модуль вызывается в след. модулях: botCore, clientCore, CommandsHandler, serverCore, PolizElem
 */


#ifndef MGLIB_H
#define MGLIB_H

/* Пирамидальная сортировка */
void heap_sort(int* values, int size, int ascending);

/* Удаляет лишние пробелы из строки */
void delete_spaces(char* buffer, int* bufsize);

/* Превращает число в строку */
void itoa(int number, char* num_buf, int max_buf_len);

/* Читает строку из станд. потока ввода. Является аналогом fgets */
int readline(int fd, char* buf, int bufsize);

#endif
