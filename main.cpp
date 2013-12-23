#include"basic/getopt_pp_standalone.h"
#include"io.h"
#include"base.h"
#include"crf_trainer.h"
#include"crf_test.h"
#include<iostream>
using namespace std;
using namespace GetOpt;
void show_help()
{
	cout<<"Condition Random Field V0.1"<<endl;
}
int main(int argc,char* argv[])
{
	show_help();
	GetOpt_pp ops(argc,argv);
	RunMode mode;
	string trainfile;
	string testfile;


	if(!(ops>>Option("train",trainfile)) && !(ops>>Option("test",testfile))) 
	{
		cout<<"please use --train option to specify the train file "<<endl;
		cout<<"or use --test to specify the test file! "<<endl;
		return -1;
	}
	
	if(trainfile=="")
		mode=RunMode::Test;
	if(testfile=="")
		mode=RunMode::Train;
	if(trainfile!="" && testfile!="")
		mode=RunMode::All;

	int it=1;
	ops>>Option("it",it);
	
	int thread_count=1;
	ops>>Option("thread",thread_count);

	double learning_rate=0.0001;
	ops>>Option("learn_rate",learning_rate);

	int debug=0;
	ops>>Option("debug",debug);

	string modelfile="model.txt";
	ops>>Option("model",modelfile);

	Data traindata,testdata;
	CRFIO io;
	if(mode==RunMode::Train)
	{
		cout<<"iteration: "<<it<<endl;
		io.read_data(trainfile,traindata);
		io.write_dict(traindata);
		CRFTrainer trainer(traindata);
		trainer.init_learning_rate=learning_rate;
		trainer.debug=debug;
		trainer.train(it,thread_count);
		trainer.write_modelfile(modelfile);
	}
	if(mode==RunMode::Test)
	{
		io.read_dict(testdata);
		io.read_data(testfile,testdata);
		CRFTest test(testdata,modelfile);
		test.predict();
	}
	if(mode==RunMode::All)
	{
		io.read_data(trainfile,traindata);
		io.write_dict(traindata);
		io.read_dict(testdata);
		io.read_data(testfile,testdata);
		CRFTrainer trainer(traindata);
		trainer.init_learning_rate=learning_rate;
		trainer.debug=debug;
		trainer.train(it,thread_count);
		trainer.write_modelfile(modelfile);
		CRFTest test(testdata,modelfile);
		test.predict();

	}
	return 0;
}
