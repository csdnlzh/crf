#ifndef Crf_Test_H
#define Crf_Test_H
#include"base.h"
#include<unordered_map>

class CRFTest
{
	public:
	CRFTest(Data& data,string modelfile):m_testdata(data)
	{		
		m_modelfile=modelfile;
		emit_params.resize(data.tags_count,vector<double>(data.attrs_count));
		tran_params.resize(data.tags_count,vector<double>(data.tags_count));
		read_params();
	}
	double calc_e(int pre_tag,int cur_tag,int cur_position,vector<SeqItem>& sample,vector<vector<vector<double> > >& cache_psai);
	void predict();
	vector<int> predict(vector<SeqItem>& sample);
	vector<vector<double> > emit_params;
	vector<vector<double> > tran_params;
	int debug;

	private:
	Data& m_testdata;
	string m_modelfile;
	void read_params();
};

#endif
