#include "./includes/Config.hpp"

int	main(int argc, char** argv)
{
	if (argc > 2)
		return (printErr("invalid number of arguments"));

	std::string	confFile = (argc == 1) ? DEFAULT_CONF : argv[1];

	Config	conf;
	conf.initServer(confFile);
	return (0);
}
