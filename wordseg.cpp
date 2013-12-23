#include<iostream>
#include<string>
#include<vector>
#include"base.h"
#include"io.h"
#include"crf_test.h"

using namespace std;

CRFIO io;
Data testdata;
string startstr("START <S>");
string endstr("END </S>");

string getword(vector<string>& words,int i)
{
	if(i>=0&&i<words.size())
		return words[i];
	else
		return " ";
}
vector<vector<string> > gen_fea(string sentence)
{
	vector<string> words;
	for(int i=0;i<sentence.size();)
	{
		words.push_back(sentence.substr(i,3));
		i=i+3;
	}
	
	vector<vector<string> > words_feas;
	for(int i=0;i<words.size();i++)
	{	
		vector<string> feas;
		feas.push_back(words[i]);
		feas.push_back(getword(words,i-2)+string("-2")+words[i]);
		feas.push_back(getword(words,i-1)+string("-1")+words[i]);
		feas.push_back(getword(words,i+1)+string("1")+words[i]);
		feas.push_back(getword(words,i+2)+string("2")+words[i]);
		
		words_feas.push_back(feas);
	}
	return words_feas;
}

vector<SeqItem> build_sample(vector<vector<string> >& sentence_attrs,Data& data)
{
	vector<SeqItem> sample;
	SeqItem start_item=io.make_item(startstr,data);
	SeqItem end_item=io.make_item(endstr,data);
	sample.push_back(start_item);
	for(auto& word_attrs:sentence_attrs)
	{
		string str="S ";
		for(auto& attr:word_attrs)
			str+=attr+" ";
		cout<<str<<endl;
		SeqItem item=io.make_item(str,data);
		sample.push_back(item);
	}
	cout<<endl;
	sample.push_back(end_item);

	return sample;
}

string get_words(vector<string>& tokens,vector<int>& tags,Data& data)
{
	vector<string>  words;
	string word;
	for(int i=0;i<tags.size();i++)
	{
		string tag=data.i2s_tag_dict[tags[i]];
		//cout<<tag;
		string token=tokens[i];
		if(tag=="S")
			words.push_back(token);
		if(tag=="B")
			word+=token;
		if(tag=="M")
			word+=token;
		if(tag=="E")
		{
			word+=token;
			words.push_back(word);
			word="";
		}
	}
	
	string sentence;
	for(auto& word:words)
		sentence+=word+" ";
	return sentence;
}

string wordseg(string sentence,CRFTest& tester)
{
	vector<vector<string> > sentence_attrs=gen_fea(sentence);
	vector<string> raw_tokens;
	for(int i=0;i<sentence_attrs.size();i++)
		raw_tokens.push_back(sentence_attrs[i][0]);

	vector<SeqItem> sample=build_sample(sentence_attrs,testdata);
	vector<int> tags=tester.predict(sample);
	vector<int> useful_tags;
	for(int i=1;i<tags.size()-1;i++)
		useful_tags.push_back(tags[i]);

	return get_words(raw_tokens,useful_tags,testdata);
}

int main(int argc,char*  argv[])
{
	io.read_dict(testdata);
	CRFTest tester(testdata,"model.txt");

	string linestr;
	while(cin>>linestr)
	{	
		wordseg(linestr,tester);
	}
	return 0;
}
