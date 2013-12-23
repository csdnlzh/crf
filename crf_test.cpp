#include"crf_test.h"
#include<sstream>
#include<iostream>
#include<fstream>
#include<algorithm>

using namespace std;

void CRFTest::read_params()
{
	ifstream modelfile(m_modelfile.c_str());
	string linestr;
	cout<<"reading model file ..."<<endl;
	while(getline(modelfile,linestr))
	{
		if(linestr=="##")
			break;

		int pre_tag;
		int cur_tag;
		double tran_prob;
		stringstream ss(linestr);
		ss>>pre_tag;
		ss>>cur_tag;
		ss>>tran_prob;
		tran_params[pre_tag][cur_tag]=tran_prob;
	}
	while(getline(modelfile,linestr))
	{
		int tag;
		int attr;
		double emit_prob;
		stringstream ss(linestr);
		ss>>tag;
		ss>>attr;
		ss>>emit_prob;
		emit_params[tag][attr]=emit_prob;
	}
	cout<<"read mode file ok! "<<endl;
}
double CRFTest::calc_e(int pre_tag,int cur_tag,int cur_position, vector<SeqItem>& sample,vector<vector<vector<double> > >& cache_psai)
{
	double result= cache_psai[cur_position][pre_tag][cur_tag];
	if(!result)
	{
		result+=tran_params[pre_tag][cur_tag];
		SeqItem& item=sample[cur_position];
		for(auto attr:item.m_attrs)
		{
			result+=emit_params[cur_tag][attr];
		}

		cache_psai[cur_position][pre_tag][cur_tag]=result;
		//if(debug) cout<<m_traindata.i2s_tag_dict[pre_tag]<<" "<<m_traindata.i2s_tag_dict[cur_tag]<<" "<<cur_position<<" "<<result<<endl;

	}
//	cout<<"psai potential: "<<result<<endl;
	return result;
}

void CRFTest::predict()
{
	ofstream outtag("out_tag");
	vector<vector<SeqItem> >& samples=m_testdata.samples;
	int tags_count=m_testdata.tags_count;

	int total_correct=0;
	int total_predict_tags=0;
	for(int i=0;i<samples.size();i++)
	{	
		cout<<i<<"\r";
		fflush(stdout);
		vector<SeqItem>& sample=samples[i];
		vector<int> predict_tag=predict(sample);
		int correct_count=0;
		int sample_len=sample.size();
		for(int i=0;i<sample_len;i++)
		{
			if(predict_tag[i]==sample[i].m_tag)
				correct_count++;
		}
		total_correct+=(correct_count-2);
		total_predict_tags+=(sample_len-2);

		for(int i=1;i<sample_len-1;i++)
		{
			outtag<<m_testdata.i2s_tag_dict[predict_tag[i]]<<" "<<m_testdata.i2s_attr_dict[sample[i].m_attrs[0]]<<endl;
		}
		outtag<<endl;

	}
	cout<<total_correct<<" / "<<total_predict_tags<<" total precision: "<<(double)total_correct/total_predict_tags<<endl;
}

vector<int> CRFTest::predict(vector<SeqItem>& sample)
{
	int sample_len=sample.size();
	int tags_count=m_testdata.tags_count;
	vector<vector<double> > alpha(sample_len,vector<double>(tags_count));
	vector<vector<int> > record_pre_tag(sample_len,vector<int>(tags_count));
	vector<vector<double> > vv(tags_count,vector<double>(tags_count));
	vector<vector<vector<double> > > psai(sample_len,vv);

	for(int i=1;i<sample_len;i++)
	{
	
		for(auto cur_tag:i==sample_len-1?m_testdata.end_tag:m_testdata.regular_tag)
		{	
			double max_value=-1e+10;
			for(auto pre_tag:i==1?m_testdata.start_tag:m_testdata.regular_tag)
			{
				double v=alpha[i-1][pre_tag]+calc_e(pre_tag,cur_tag,i,sample,psai);
				if(v>max_value)
				{
					max_value=v;
					record_pre_tag[i][cur_tag]=pre_tag;
					alpha[i][cur_tag]=v;
				}
			}
		}
	}
	vector<int> predict_tag;
	int cur_tag=sample[sample_len-1].m_tag;
	int pre_tag;
	predict_tag.push_back(cur_tag);
	for(int i=sample_len-1;i>0;i--)
	{
		pre_tag=record_pre_tag[i][cur_tag];
		predict_tag.push_back(pre_tag);
		cur_tag=pre_tag;
	}
	reverse(predict_tag.begin(),predict_tag.end());

	return predict_tag;
}
