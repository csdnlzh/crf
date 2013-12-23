#ifndef Base_H
#define Base_H
#include<string>
#include<vector>
#include<utility>
#include<unordered_map>
using namespace std;

enum RunMode {Train,Test,All};

struct pair_hash
{
	size_t operator()(const pair<int,int>& p) const 
	{
		return p.first<<16+p.second;
	}
};

class SeqItem
{
	public:
	SeqItem(int tag,vector<int> attrs)
	{
		m_tag=tag;
		m_attrs=attrs;
		for(auto attr:m_attrs)
			m_feas.push_back(make_pair(attr,m_tag));
	}
	int m_tag;
	vector<int> m_attrs ;
	vector<pair<int,int> > m_feas;
};

class Data
{
	public:
	Data()
	{
		start_tag.push_back(0);
		end_tag.push_back(1);

		s2i_tag_dict["START"]=0;
		s2i_tag_dict["END"]=1;
		i2s_tag_dict[0]="START";
		i2s_tag_dict[1]="END";
		s2i_attr_dict["<S>"]=0;
		s2i_attr_dict["</S>"]=1;
		i2s_attr_dict[0]="<S>";
		i2s_attr_dict[1]="</S>";

		tags_count=2;
		attrs_count=2;
		feas_count=0;

	}
	vector<vector<SeqItem> > samples;
	
	vector<int> start_tag;
	vector<int> end_tag;
	vector<int> regular_tag;

	unordered_map<string,int> s2i_tag_dict;
	unordered_map<int,string> i2s_tag_dict;
	unordered_map<string,int> s2i_attr_dict;
	unordered_map<int,string> i2s_attr_dict;	

	int tags_count;
	int attrs_count;
	int feas_count;
	
};

#endif
