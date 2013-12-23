#include"crf_trainer.h"
#include<cmath>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<pthread.h>
using namespace std;


double logsumexp(vector<double>& v)
{
	double result=0;
	double max_value=*std::max_element(v.begin(),v.end());
//	cout<<"max value: "<<max_value<<endl;
	for(auto value:v)
		result+=exp(value-max_value);
	
	return log(result)+max_value;

}
void CRFTrainer::print_params()
{
	write_model(cout);
}

void CRFTrainer::write_modelfile(string& modelfile)
{
	cout<<"writing model file: "<<modelfile<<endl;
	ofstream outfile(modelfile.c_str());	
	write_model(outfile);
	outfile.close();
	cout<<"model file saved!"<<endl;
}
void CRFTrainer::write_model(ostream& out)
{
	for(int i=0;i<m_traindata.tags_count;i++)
	{
		for(int j=0;j<m_traindata.tags_count;j++)
		{
			out<<i<<" "<<j<<" "<<tran_params[i][j]<<endl;
		}
	}
	
	out<<"##"<<endl;

	for(int i=0;i<m_traindata.tags_count;i++)
	{
		for(int j=0;j<m_traindata.attrs_count;j++)
		{
			out<<i<<" "<<j<<" "<<emit_params[i][j]<<endl;
		}
	}
	
}
double CRFTrainer::calc_gradnorm()
{
	double norm=0;
	for(auto&v :tran_params)
		for(auto g:v)
			norm+=g*g;
	for(auto&v: emit_params)
		for(auto g:v)
			norm+=g*g;
	return norm;	
}

void CRFTrainer::update_params(vector<vector<double> >& tran_subgrad,unordered_map<pair<int,int>,double,pair_hash>& emit_subgrad)
{
	//upate the parameters
	double l1=0;
	for(auto& f_v:emit_subgrad)
		l1+=abs(f_v.second);
	for(int i=0;i<m_traindata.tags_count;i++)
	{
		for(int j=0;j<m_traindata.tags_count;j++)
		{
			l1+=abs(tran_subgrad[i][j]);
		}
	}

	for(auto& f_v:emit_subgrad)
		emit_params[f_v.first.first][f_v.first.second]+=f_v.second*learning_rate;
	for(int i=0;i<m_traindata.tags_count;i++)
	{
		for(int j=0;j<m_traindata.tags_count;j++)
		{
			tran_params[i][j]+=tran_subgrad[i][j]*learning_rate;
			tran_subgrad[i][j]=0;
		}
	}

}

void* CRFTrainer::thread_train(void* context)
{
	Thread_context* thread_context=(Thread_context*)context;
	CRFTrainer* trainer=thread_context->trainer;
	int pid=thread_context->pid;
	vector<vector<SeqItem> >& samples=trainer->m_traindata.samples;
	vector<vector<double> > tran_subgrad(trainer->m_traindata.tags_count,vector<double>(trainer->m_traindata.tags_count));;
	unordered_map<pair<int,int>,double,pair_hash> emit_subgrad;
	long long id=0;

	double loglike=0;
	for(int i=trainer->m_piece*pid;i<trainer->m_piece*(pid+1);i++)
	{
		emit_subgrad.clear();
		if(trainer->debug) cout<<"sample "<<i<<endl;
		double log_likelihood=trainer->compute_subgrad(samples[i],tran_subgrad,emit_subgrad);	
		loglike+=log_likelihood;
		trainer->update_params(tran_subgrad,emit_subgrad);
	}
	thread_context->loglike=loglike;

}

void CRFTrainer::train(int iteration,int thread_count)
{
	
	cout<<"train begin!"<<endl;	
	m_piece=m_traindata.samples.size()/thread_count;
	int opt_wrong_count=0;
	double pre_loglike=-1e20;
	this->learning_rate=this->init_learning_rate;

	for(int it=0;it<iteration;it++)
	{
	
		//this->learning_rate=this->init_learning_rate/(1+log(it+1)/100);
		cout<<"learning rate: "<<this->learning_rate<<endl;

		double loglike=0;
		pthread_t *pt=(pthread_t*)malloc(thread_count*sizeof(pthread_t));
		vector<Thread_context*> thread_contexts;
		for(int i=0;i<thread_count;i++)
		{
			Thread_context* context=new Thread_context();
			thread_contexts.push_back(context);
			context->trainer=this;
			context->pid=i;
			pthread_create(&pt[i],NULL,&CRFTrainer::thread_train,(void*)context);
		}
		for(int i=0;i<thread_count;i++)
		{
			pthread_join(pt[i],NULL);
		}

		for(int i=0;i<thread_count;i++)
		{
			loglike+=thread_contexts[i]->loglike;
			delete thread_contexts[i];
		}
		cout<<it<<"  log likelihood: "<<loglike<<endl;
		//if(debug)
		//{
	        //	this->print_params();
		//	cout<<"grad norm: "<<this->calc_gradnorm()<<endl;
		//}
		if(loglike<pre_loglike)
		{
			break;
		}
		else
		{
			pre_loglike=loglike;
		}
	}
	
}

double CRFTrainer::compute_subgrad(vector<SeqItem>& sample,vector<vector<double> >& tran_subgrad,unordered_map<pair<int,int>,double,pair_hash>& emit_subgrad)
{
	int tags_count=m_traindata.tags_count;
	//calculate the actually feature value
	int sample_len=sample.size();
	for(int i=0;i<sample_len;i++)
	{
		SeqItem& cur_item=sample[i];
		for(auto attr:cur_item.m_attrs)
		{
//			cout<<m_traindata.i2s_attr_dict[attr]<<endl;
			emit_subgrad[make_pair(cur_item.m_tag,attr)]+=1;
		}
		if(i>0)
		{
			SeqItem& pre_item=sample[i-1];
			tran_subgrad[pre_item.m_tag][cur_item.m_tag]+=1;
		}
	}
	
	//use forward backward algorithm to compute the expected feature value;
	double alpha[sample_len][tags_count];
	double beta[sample_len][tags_count];
	for(int i=0;i<sample_len;i++)
	{
		for(int j=0;j<tags_count;j++)
		{
			alpha[i][j]=0;
			beta[i][j]=0;
		}
	}

	vector<vector<double> > vv(tags_count,vector<double>(tags_count));
	vector<vector<vector<double> > > psai(sample_len,vv);

	//******************************forward algorithm**********************************
	for(int i=1;i<sample_len;i++)
	{
		for(auto cur_tag:i<sample_len-1?m_traindata.regular_tag:m_traindata.end_tag)
		{
			vector<double> vv;
			for(auto pre_tag:i==1?m_traindata.start_tag:m_traindata.regular_tag)
			{ 
				double v=alpha[i-1][pre_tag]+calc_e(pre_tag,cur_tag,i,sample,psai);
				vv.push_back(v);
				//if(debug) cout<<i<<cur_tag<<pre_tag<<" v: "<<v<<endl;
			}
			alpha[i][cur_tag]=logsumexp(vv);
			//if(debug) cout<<i<<" "<<cur_tag<<" "<<alpha[i][cur_tag]<<endl;
		}

	}
	//if(debug) cout<<"back forward"<<endl;
	//*******************************backward algorithm**********************************
	for(int i=sample_len-2;i>=0;i--)
	{
		for(auto cur_tag:i==0?m_traindata.start_tag:m_traindata.regular_tag)
		{
			vector<double> vv;
			for(auto next_tag:i==sample_len-2?m_traindata.end_tag:m_traindata.regular_tag)
			{
				double v=beta[i+1][next_tag]+calc_e(cur_tag,next_tag,i+1,sample,psai);
				vv.push_back(v);	
			}
			beta[i][cur_tag]=logsumexp(vv);
		}
	}

	//****************************calculate the likelihood of P(y|x)***********************
	double loglik=0;
	double norm=alpha[sample_len-1][1];
//	cout<<"norm: "<<norm<<endl;
	for(int i=0;i<tags_count;i++)
		for(int j=0;j<tags_count;j++)
			loglik+=tran_subgrad[i][j]*tran_params[i][j];
	for(auto& emit:emit_subgrad)
		loglik+=emit.second*emit_params[emit.first.first][emit.first.second];
	

	loglik=loglik-norm;

	//******************************calculate the expected feature value*******************
	emit_subgrad[make_pair(0,0)]=0;
	for(int i=1;i<sample_len;i++)
	{
		SeqItem& item=sample[i];
		for (auto cur_tag:i==sample_len-1?m_traindata.end_tag:m_traindata.regular_tag)
		{
			for(auto pre_tag: i==1?m_traindata.start_tag:m_traindata.regular_tag)
			{					
				double p =exp(alpha[i-1][pre_tag]+calc_e(pre_tag,cur_tag,i,sample,psai)+beta[i][cur_tag]-norm);
				//if(debug)
				//{
				//	cout<<exp(alpha[i-1][pre_tag])<<endl;
				//	cout<<exp(calc_e(pre_tag,cur_tag,i,sample,psai))<<endl;
				//	cout<<exp(beta[i][cur_tag])<<endl;
				//	cout<<exp(norm)<<endl;
				//	cout<<"except: "<<p<<endl;
				//}
				tran_subgrad[pre_tag][cur_tag]-=p;
				for(auto attr:item.m_attrs)
				{
					emit_subgrad[make_pair(cur_tag,attr)]-=p;
				}
				
			}
		}
	}
	emit_subgrad[make_pair(1,1)]=0;	
	return loglik;
}

double CRFTrainer::calc_e(int pre_tag,int cur_tag,int cur_position, vector<SeqItem>& sample,vector<vector<vector<double> > >& cache_psai)
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
	return result;
}

