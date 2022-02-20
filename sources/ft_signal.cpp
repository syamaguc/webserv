#include "ft_signal.hpp"

static void _stop(int sig)
{
	(void)sig;
	std::exit(0);
}

/**
 * @brief signalをセットする
 */
void set_signal()
{
	signal(SIGABRT, _stop);
	signal(SIGINT, _stop);
	signal(SIGTERM, _stop);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
}
