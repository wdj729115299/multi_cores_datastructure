#include "compete.h"

static cpus_pool_s cpu_pools[CPU_USAGE_NUM];

static task_func_info_s *task_ptr;
static int num_task;

static pthread_t 	threads[TASK_COUNT];
static int num_threads = 0;


static inline task_func_info_s * get_task_func_info(void)
{
	num_task++;
	return &task_ptr[num_task - 1];
}

int cpu_get(cpu_usage_e cpu_usage)
{
	int cpu_number;
	int i;
	cpus_pool_s *cpu_pool = &cpu_pools[cpu_usage];
	//for(i)
}

static void compute_cpus(void)
{
	cpu_set_t	cpu_mask;
	cpus_pool_s 	*cpu_pool = NULL;
	int i;
	int num;
	int cnt = 0;

	// Initialize cpu_set in cpu pool.
	for(i = 0; i < CPU_USAGE_NUM; i++){
		cpu_pool = &cpu_pools[i];
		CPU_ZERO(&cpu_pool->cpus);
	}
	
	unsigned int needed_cpus = APP_TILES + TIMER_TILES;
	unsigned int num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	printf("system has %d processor(s)\n", num);
	
	if(num_cpus < needed_cpus){
		task_die("Insufficient CPUs in affinity set: "
                    "got %u, need %u.\n", num_cpus, needed_cpus);
	}

	cpu_pool = &cpu_pools[CPU_USAGE_APP];
	CPU_ZERO(&cpu_pool->cpus);
	for(i = 0; i < APP_TILES; i++){
		CPU_SET(cnt, &cpu_pool->cpus);
		cnt++;
	}
	cpu_pool->next = 0;
	cpu_pool->usage = CPU_USAGE_APP;
	cpu_pool->cnt = APP_TILES;

	cpu_pool = &cpu_pools[CPU_USAGE_TIMER];
	CPU_ZERO(&cpu_pool->cpus);
	for(i = 0; i < TIMER_TILES; i++){
		CPU_SET(cnt, &cpu_pool->cpus);
		cnt++;
	}
	cpu_pool->next = 0;
	cpu_pool->usage = CPU_USAGE_APP;
	cpu_pool->cnt = TIMER_TILES;

	return;
}

static void *task_func(void *arg)
{
	task_func_info_s *task_info_ptr = (task_func_info_s *)arg;
	task_info_s real_task_info;

	int status = pthread_setaffinity_np(threads[num_threads-1], sizeof(task_info_ptr->cpu_number), &task_info_ptr->cpu_number);
	if(status < 0)
		task_die("pthread_setaffinity_np failed.\n");

	init_task_info(&real_task_info, task_info_ptr);

	//rcu_register_thread();
	task_info_ptr->start_routine((void*)&real_task_info);
	//rcu_unregister_thread();

	return NULL;
}

static pid_t fork_or_die(void)
{
	pid_t pid;
	if( (pid = fork()) == -1)
		task_die("Unable to fork process.\n");

	return pid;
}

void task_dispatch(task_func_info_s *task_info_ptr)
{
	if(TASK_MODE == TASK_MODE_PROCESS){
		pid_t pid;
		if((pid = fork_or_die()) == 0){		/* Child process */
			task_func((void*)task_info_ptr);
			exit(EXIT_SUCCESS);
		}else{
			return
		}
	}

	if(TASK_MODE == TASK_MODE_THREAD){
		int status = pthread_create(&threads[num_threads++], NULL, task_func, (void *)task_info_ptr);

		if(status != 0)
			task_die("Unable to create thread %s\n",
                      strerror(status));

		return;
	}
	
}

void init_task_info(task_info_s       *task_info,
                      task_func_info_s  *task_func_info)
{
  switch (task_func_info->cpu_usage)
  {
    case  CPU_USAGE_TIMER:
      if (task_func_info->cpu_rank >= TIMER_TILES)
        task_die("invalid cpu rank %d for cpu usage TIMER",
                        task_func_info->cpu_rank);
      break;
    case  CPU_USAGE_APP:
      if (task_func_info->cpu_rank >= APP_TILES)
        netlib_task_die("invalid cpu rank %d for cpu usage APP",
                        task_func_info->cpu_rank);
      break;
    default:
      netlib_task_die("invalid cpu usage %d", task_func_info->cpu_usage);
  }

  memset(task_info, 0, sizeof(netlib_task_info_s));

  task_info->cpu_usage = task_func_info->cpu_usage;
  task_info->cpu_rank  = task_func_info->cpu_rank;
  
}


void global_data_init(void)
{
	task_ptr = malloc(sizeof(*task_ptr));
	if(!task_ptr)
		task_die("no memory for task_ptr");
}

void init_data()
{
	compute_cpus();

	global_data_init();

	//rcu_init();
}

void init_task(void)
{
	task_func_info_s *task_info_ptr;
	int cpu_number;
	int i;

	for(i = 0; i < APP_TILES; i++){
		cpu_number                   = cpu_get(CPU_USAGE_APP);
		task_info_ptr				 = get_task_func_info();
		task_info_ptr				 = app_tile_main_loop();
		task_info_ptr->cpu_rank		 = i;
		task_info_ptr->cpu_usage	 = CPU_USAGE_APP;
		task_info_ptr->cpu_number	 = cpu_number;
		
		task_dispatch(task_info_ptr);
	}

	for(i = 0; i < TIMER_TILES; i++){
		cpu_number                   = cpu_get(CPU_USAGE_TIMER);
		task_info_ptr				 = get_task_func_info();
		task_info_ptr				 = timer_tile_main_loop();
		task_info_ptr->cpu_rank		 = i;
		task_info_ptr->cpu_usage	 = CPU_USAGE_TIMER;
		task_info_ptr->cpu_number	 = cpu_number;
		
		task_dispatch(task_info_ptr);
	}
}
