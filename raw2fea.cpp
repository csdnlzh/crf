#include<glibmm.h>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
using namespace std;
using Glib::ustring;

string get_attr(vector<string>& s,int i)
{
	if(i>=0&&i<s.size())
		return s[i];
	else
		return " ";
}

void apply_template(vector<string>&tags,vector<string>& s)
{
	int len=tags.size();
	for(int i=0;i<len;i++)
	{
		string cur_attr=get_attr(s,i);
		cout<<tags[i]<<" ";
		cout<<cur_attr<<" ";
		cout<<get_attr(s,i-2)<<"-2"<<cur_attr<<" ";
		cout<<get_attr(s,i-1)<<"-1"<<cur_attr<<" ";
		cout<<get_attr(s,i+1)<<"+1"<<cur_attr<<" ";
		cout<<get_attr(s,i+2)<<"+2"<<cur_attr<<endl;;
	}
	cout<<endl;
}

void process_predict_sentence(string& s)
{
	ustring ustr(s);
	vector<string> tags;
	vector<string> sentence;
	for(int i=0;i<ustr.size();i++)
	{
		tags.push_back("S");
		sentence.push_back(ustr.substr(i,1).raw());
	}
	apply_template(tags,sentence);
}
void process_train_sentence(string& s)
{
	vector<string> tags;
	vector<string> sentence;

	stringstream ss(s);
	string word;
	vector<string>  words;
	while(ss>>word)
	{
		words.push_back(word);
	}
	for(auto& word:words)
	{
		ustring ustr(word);
		vector<string> chars;
		for(int i=0;i<ustr.size();i++)
		{
			chars.push_back(ustr.substr(i,1).raw());
		}

		int word_len=chars.size();
		if(word_len==1)
		{
			tags.push_back("S");
			sentence.push_back(chars[0]);
		}
		else 
		{
			tags.push_back("B");
			sentence.push_back(chars[0]);
			for(int i=1;i<word_len-1;i++)
			{
				tags.push_back("M");
				sentence.push_back(chars[i]);
			}
			tags.push_back("E");
			sentence.push_back(chars[word_len-1]);
		}

	}
	apply_template(tags,sentence);
}

int main(int argc,char* argv[])
{
	string mode(argv[1]);
	string linestr;
	if(mode=="train")
	{
		while(getline(cin,linestr))
		{
			process_train_sentence(linestr);
			
		}
	}
	else if(mode=="predict")
	{
		while(getline(cin,linestr))
		{
			process_predict_sentence(linestr);
		}
	}

	return 0;
}
