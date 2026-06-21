/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_args.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaatif-a <jaatif-a@student.42urduliz.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/21 17:28:35 by jaatif-a          #+#    #+#             */
/*   Updated: 2026/06/21 17:36:30 by jaatif-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_scheduler(char *str, t_scheduler *sched)
{
	if (strcmp(str, "fifo") == 0)
		*sched = FIFO;
	else if (strcmp(str, "edf") == 0)
		*sched = EDF;
	else
		return (0);
	return (1);
}

int	ft_atol_positive(const char *str, t_ms *out)
{
	t_ms	num;
	int		i;
	int		digit;

	if (!str[0])
		return (0);
	num = 0;
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		digit = str[i] - '0';
		if (num > (LLONG_MAX - digit) / 10)
			return (0);
		num = num * 10 + digit;
		i++;
	}
	*out = num;
	return (1);
}

int	parse_args(int argc, char **argv, t_args *args)
{
	if (argc != 9)
		return (0);
	if (!ft_atol_positive(argv[1], &args->number_of_coders))
		return (0);
	if (!ft_atol_positive(argv[2], &args->time_to_burnout))
		return (0);
	if (!ft_atol_positive(argv[3], &args->time_to_compile))
		return (0);
	if (!ft_atol_positive(argv[4], &args->time_to_debug))
		return (0);
	if (!ft_atol_positive(argv[5], &args->time_to_refactor))
		return (0);
	if (!ft_atol_positive(argv[6], &args->number_of_compiles_required))
		return (0);
	if (!ft_atol_positive(argv[7], &args->dongle_cooldown))
		return (0);
	if (!parse_scheduler(argv[8], &args->scheduler))
		return (0);
	return (1);
}

int	checker_parse_args(int argc, char **argv, t_args *args)
{
	if (!parse_args(argc, argv, args))
		return (0);
	if (args->number_of_coders < 1)
		return (0);
	return (1);
}
