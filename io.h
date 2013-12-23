#ifndef IO_H
#define IO_H
#include"base.h"
#include<vector>
#include<string>
using namespace std;

class CRFIO
{
	public:
	CRFIO()
	{
	}

	SeqItem make_item(string& itemstr,Data& data);

	void read_dict(Data& testdata);
	void write_dict(Data& traindata);
	void read_data(string filename,Data& data);
	void print_data(Data& data);
};
#endif
