#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C"{
#endif

#define TASK_MODE_THREAD 0
#define TASK_MODE_PROCESS (TASK_MODE_THREAD + 1)

#undef TASK_MODE 
#define TASK_MODE	TASK_MODE_THREAD

#undef APP_TILES
#define APP_TILES 1

#undef TIMER_TILES
#define TIMER_TILES 2

#undef TASK_COUNT
#define TASK_COUNT 128

#ifdef __cplusplus
}
#endif

#endif
