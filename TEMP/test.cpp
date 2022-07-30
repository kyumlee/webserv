#include <vector>
#include <iostream>
#include <string>
#include <cmath>

class test
{
	// private:
	public:
		int	hhih;
	
	public:
		void	setting(int num)
		{ this->hhih = num; }

		int	gethh()
		{ return (this->hhih); }
};

class sibal
{
	private:
		std::vector<int>	_sib;
		std::vector<test>	_test;
	
	public:
		void	setting(int num)
		{
			this->_sib.push_back(num);
		}

		void	set_test()
		{
			for (size_t i = 0; i < 5; i++)
			{
				test	test1;
				this->_test.push_back(test1);
				this->_test[i].hhih = i;
			}
		}

		void	print_test()
		{
			for (std::vector<test>::iterator it = _test.begin();
				it != this->_test.end(); it++)
			{
				std::cout << "num : " << (*it).gethh() << std::endl;
			}
		}

		void	printsss()
		{
			for (std::vector<int>::iterator it = _sib.begin();
				it != this->_sib.end(); it++)
			{
				std::cout << "num : " << (*it) << std::endl;
			}
		}
};

//string을 받아서 16진수로 이루어져 있으면 1을 리턴, 아니면 0을 리턴
int	checkHex(std::string& hex)
{
	for (std::string::iterator it = hex.begin(); it != hex.end(); it++)
	{
		if ((*it >= 'a' && *it <= 'f') || (*it >= '0' && *it <= '9'))
			continue ;
		else
			return (0);
	}
	return (1);
}

//string형으로 받은 hex가 16진수로 이루어져 있지 않으면 0을 리턴
size_t	hexToDecimal(std::string& hex)
{
	// std::string	hex_str = "0123456789abcdef";
	size_t	ret = 0;
	std::string::iterator	hex_it = hex.begin();
	if (checkHex(hex) == 0)
		return (0);
	while (*hex_it == '0')
		hex_it++;
	for (; hex_it != hex.end(); hex_it++)
	{
		size_t	num = *hex_it;
		if (num >= 'a' && num <= 'f')
			num = num - 'a' + 10;
		else if (num >= '0' && num <= '9')
			num -= '0';
		ret = ret * 16 + num;
	}
	return (ret);
}

int	main()
{
	std::string	str = "00110";
	size_t		ret = hexToDecimal(str);
	std::cout << "ret: " << ret << std::endl;
}