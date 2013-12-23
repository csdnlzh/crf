#include<glibmm.h>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
using namespace std;
using Glib::ustring;

void process_sentence(string& s)
{
	stringstream ss(s);
	vector<string> words;
	string word;
	while(ss>>word)
		words.push_back(word);
	vector<string> sentence;
	for(auto& w:words)
	{
		ustring ustr(w);
		for(int i=0;i<ustr.size();i++)
			sentence.push_back(ustr.substr(i,1).raw());
	}
	for(auto& word :sentence)
		cout<<word<<" ";
	cout<<endl;
}
int main(int argc,char* argv[])
{
	string linestr;
	int count=0;
	while(getline(cin,linestr))
	{
		process_sentence(linestr);
		count++;
		if(count%100==0)
			cerr<<count<<"\r";
	}
	return 0;
}
