#ifndef VECTOR_H
#define VECTOR_H

#include "Config.h"
#include "Coordinator.h"

// Размер заголовка + стоп байт + максимальный пакет, а это количество мак адресов, но не меньше чем 257 для передачи отладочных сообщений
#define VECTOR_MAX_SIZE 10 + MAX(257, DEF_MAX_DEVICE_COUNT * 8 + 4 )

typedef struct {
	uint8 buf[VECTOR_MAX_SIZE];
	uint16 size;
	uint16 maxSize;
} Vector;

void VectorClear (Vector* const v);
uint16 VectorAvailSize (Vector const * const  v);
void VectorPushBack(Vector * const  v, uint8 value);
uint8 VectorGet(Vector const * const v, uint8 pos);

#endif
