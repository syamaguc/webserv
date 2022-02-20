#include "Cluster.hpp"
#include "Configuration.hpp"
#include "WebServ.hpp"
#include "ft_signal.hpp"

int main(int argc, char** argv)
{
	Configuration config;
	char* path;

	if(argc != 2)
	{
		std::cerr << "Usage: ./webserv <config file>" << std::endl;
		return (0);
	}
	try
	{
		set_signal();
		path = argv[1];
		config = Configuration(path);
#if DEBUG_ACTIVE == 1
		config.show_server_info();
#endif
		Log("Start Server(s)...");

		Cluster C(config);
		C.loop();
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return (0);
}
