#include <vector>
#include <iostream>
#include <string>

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

int	main()
{
	sibal	test1;
	test1.set_test();
	test1.print_test();
}