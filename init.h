#ifndef __INIT_H
#define __INIT_H

#ifdef __cplusplus
extern "C"{
#endif

typedef struct cpus_pool{
	cpu_usage_e 	usage;
	cpu_set_t 		cpus;
	int				next;
	int 			cnt;
}cpus_pool_s;

typedef struct{
	void *(*start_routine)(void*);
	cpu_usage_e		cpu_usage;
	int 	cpu_rank;
	int		cpu_number;
}task_func_info_s;

#ifdef __cplusplus
}
#endif

#endif
