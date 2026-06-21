/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:27:30 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:35:41 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H
# include <pthread.h>
# include <stdio.h>
# include <stdbool.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include <limits.h>

typedef long long			t_ms;
typedef struct timespec		t_timespec;

# define LOG_TAKE_DONGLE	"%lld %d has taken a dongle\n"
# define LOG_COMPILING		"%lld %d is compiling\n"
# define LOG_DEBUGGING		"%lld %d is debugging\n"
# define LOG_REFACTOR		"%lld %d is refactoring\n"
# define LOG_BURNS_OUT		"%lld %d burned out\n"

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef enum e_state
{
	WAITING,
	TAKING_DONGLE,
	COMPILING,
	DEBUGGING,
	REFACTORING,
	BURNED_OUT,
	FINISHED
}	t_state;

typedef struct s_codexion	t_codexion;
typedef struct s_coder		t_coder;
typedef struct s_dongle		t_dongle;
typedef struct s_args		t_args;
typedef struct s_qnode		t_qnode;
typedef struct s_queue		t_queue;
typedef struct s_monitoring	t_monitoring;

struct s_args
{
	long long	number_of_coders;
	t_ms		time_to_burnout;
	t_ms		time_to_compile;
	t_ms		time_to_debug;
	t_ms		time_to_refactor;
	long long	number_of_compiles_required;
	t_ms		dongle_cooldown;
	t_scheduler	scheduler;
};

struct s_qnode
{
	int		coder_id;
	t_ms	priority;
};

struct s_queue
{
	t_qnode			*nodes;
	int				size;
	int				capacity;
	pthread_mutex_t	heap_lock;
};

struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	bool			available;
	t_ms			last_use_time;
	t_queue			*queue;
};

struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*left;
	t_dongle		*right;
	int				compile_count;
	t_ms			last_compile;
	t_codexion		*codex;
	t_state			state;
	pthread_mutex_t	mutex_coder;
};

struct s_monitoring
{
	pthread_t		monitoring;
	pthread_mutex_t	stop_lock;
	bool			is_stopped;
};

struct s_codexion
{
	t_args			args;
	t_ms			start_time;
	t_coder			*coders;
	t_dongle		*dongles;
	t_monitoring	*monitoring;
	pthread_mutex_t	write_mutex;
	int				n_dongles_init;
	int				n_coders_init;
};

/* parse */
int			ft_atol_positive(const char *str, t_ms *out);
int			parse_args(int argc, char **argv, t_args *args);
int			checker_parse_args(int argc, char **argv, t_args *args);

/* init / destroy */
int			init_queue(t_queue *queue, int capacity);
void		destroy_queue(t_queue *queue);
int			init_dongles(t_codexion *codex);
int			init_coders(t_codexion *codex);
int			init_codexion(t_codexion *codex, t_args *args);
void		destroy_codexion(t_codexion *codex);

/* utils */
t_ms		get_time_ms(void);
t_ms		timeval_to_ms(struct timeval time);
int			ft_usleep(t_codexion *codex, t_ms milliseconds);

/* heap */
void		swap_nodes(t_qnode *a, t_qnode *b);
int			get_prio(t_qnode *a, t_qnode *b);
int			get_id(t_queue *queue);
void		heapify_up(t_queue *queue, int i);
void		insert(t_queue *queue, int id, t_ms priority);
void		pop(t_queue *queue);
void		remove_from_queue(t_queue *queue, int coder_id);

/* dongles */
t_ms		calc_priority(t_coder *coder);
int			cooldown_passed(t_dongle *dongle, t_ms cooldown);
int			is_queue_head(t_dongle *dongle, int coder_id);
t_timespec	calc_timeout(t_dongle *dongle, t_codexion *codex);
void		sort_dongles(t_coder *coder, t_dongle **first, t_dongle **second);
int			acquire_loop(t_dongle *first, t_dongle *second, t_coder *coder);
int			take_dongles(t_coder *coder);
void		release_dongles(t_coder *coder);

/* coder routine */
void		change_state(t_coder *coder, t_state new_state);
void		do_compile(t_coder *coder);
void		do_debug(t_coder *coder);
void		do_refactor(t_coder *coder);
void		*coder_routine(void *arg);

/* logs */
void		print_logs(int index, t_state state, t_codexion *codex);

/* stop / monitor */
void		set_stopped_true(t_codexion *codex);
bool		is_stopped(t_codexion *codex);
int			all_done(t_codexion *codex);
void		*monitor_routine(void *arg);

/* threads */
int			launch_threads(t_codexion *codex);
void		join_threads(t_codexion *codex);

#endif
