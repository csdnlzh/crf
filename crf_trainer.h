#ifndef Crf_Trainer_H
#define Crf_Trainer_H
#include"base.h"
#include<unordered_map>

class CRFTrainer
{
	public:
	CRFTrainer(Data& data):m_traindata(data)
	{
		emit_params.resize(data.tags_count,vector<double>(data.attrs_count));
		tran_params.resize(data.tags_count,vector<double>(data.tags_count));
		emit_grads.resize(data.tags_count,vector<double>(data.attrs_count));
		tran_grads.resize(data.tags_count,vector<double>(data.tags_count));

		learning_rate=0.0001;
	}

	void train(int iteration,int thread_count);
	static void* thread_train(void* context);
	double compute_subgrad(vector<SeqItem>& sample,vector<vector<double> >& tran_subgrad,unordered_map<pair<int,int>,double,pair_hash>& emit_subgrad) ;
	double calc_e(int pre_tag,int cur_tag,int cur_position,vector<SeqItem>& sample,vector<vector<vector<double> > >& cache_psai);
	void update_params(vector<vector<double> >& tran_subgrad,unordered_map<pair<int,int>,double,pair_hash>& emit_subgrad);
	double calc_gradnorm();
	void print_params();
	void write_model(ostream& out);
	void write_modelfile(string& modelfile);

	vector<vector<double> > emit_params;
	vector<vector<double> > tran_params;
	vector<vector<double> > emit_grads;
	vector<vector<double> > tran_grads;

	double learning_rate;
	double init_learning_rate;
	int debug;

	private:
	Data& m_traindata;
	int m_piece;
	vector<double> thread_loglike;
};

class Thread_context
{
	public:
	CRFTrainer* trainer;
	int pid;
	double loglike;
};
#endif
