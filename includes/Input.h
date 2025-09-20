#ifndef INPUT_H_SENTRY
#define INPUT_H_SENTRY

enum
{
			MAX_READ_BUF_SIZE			=			255
};


int get_any_key(void);
int get_str(char* buffer, int buffer_size);

#endif
