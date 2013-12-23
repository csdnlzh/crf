#include "io.h"
#include<iostream>
#include<fstream>
#include<sstream>
using namespace std;

//write the dict of the train data to files
void CRFIO::write_dict(Data& traindata)
{
	ofstream tagdict("tag_dict");
	for(auto& item:traindata.s2i_tag_dict)
		tagdict<<item.second<<" "<<item.first<<endl;
	ofstream attrdict("attr_dict");
	for(auto& item:traindata.s2i_attr_dict)
		attrdict<<item.second<<" "<<item.first<<endl;
		
	tagdict.close();
	attrdict.close();

}

//read the dict files to the test data
void CRFIO::read_dict(Data& testdata)
{
	ifstream tagdict("tag_dict");
	ifstream attrdict("attr_dict");

	string linestr;
	while(getline(tagdict,linestr))
	{
		stringstream ss(linestr);
		string tag;
		int tag_index;
		ss>>tag_index;
		ss>>tag;
		testdata.s2i_tag_dict[tag]=tag_index;
		testdata.i2s_tag_dict[tag_index]=tag;
	}
	tagdict.close();

	while(getline(attrdict,linestr))
	{
		stringstream ss(linestr);
		string attr;
		int attr_index;
		ss>>attr_index;
		ss>>attr;
		testdata.s2i_attr_dict[attr]=attr_index;
		testdata.i2s_attr_dict[attr_index]=attr;
	}
	attrdict.close();

	testdata.tags_count=testdata.s2i_tag_dict.size();
	testdata.attrs_count=testdata.s2i_attr_dict.size();
	for(int i=2;i<testdata.tags_count;i++)
		testdata.regular_tag.push_back(i);
}

void CRFIO::print_data(Data& data)
{
	for(auto& sample:data.samples)
	{
		for(auto& item:sample)
		{
			cout<<data.i2s_tag_dict[item.m_tag]<<" ";
			for(auto attr:item.m_attrs)
				cout<<data.i2s_attr_dict[attr]<<" ";
			cout<<endl;
		}
		cout<<endl;
	}
}


SeqItem CRFIO::make_item(string& itemstr, Data& data)
{
	stringstream ss(itemstr);
	string stag;
	ss>>stag;
	if(data.s2i_tag_dict.find(stag)==data.s2i_tag_dict.end())
	{		
		data.s2i_tag_dict[stag]=data.tags_count;
		data.i2s_tag_dict[data.tags_count]=stag;
		data.regular_tag.push_back(data.tags_count);
		data.tags_count++;
	}
	int itag=data.s2i_tag_dict[stag];

	string sattr;
	vector<int> attrs;
	while(ss>>sattr)
	{
		if(data.s2i_attr_dict.find(sattr)==data.s2i_attr_dict.end())
		{
			data.s2i_attr_dict[sattr]=data.attrs_count;
			data.i2s_attr_dict[data.attrs_count]=sattr;
			data.attrs_count++;
		}
		
		attrs.push_back(data.s2i_attr_dict[sattr]);
	}
	
	SeqItem item(itag,attrs);
	data.feas_count+=item.m_feas.size();
	return item;
}

void CRFIO::read_data(string filename,Data& data)
{
	ifstream infile(filename.c_str());
	if(!infile)
	{
		cout<<"open file wrong: "<<filename<<endl;
	}
	vector<SeqItem> sample;
	string startstr("START <S>");
	string endstr("END </S>");
	SeqItem start_item=make_item(startstr,data);
	SeqItem end_item=make_item(endstr,data);

	sample.push_back(start_item);
	string linestr;
	int linecount=0;
	while(getline(infile,linestr))
	{
		if(linestr=="")
		{
			sample.push_back(end_item);
			data.samples.push_back(sample);
			sample.clear();
			sample.push_back(start_item);
		}
		else
		{
			sample.push_back(make_item(linestr,data));
		}	
		linecount++;
		if(linecount%10==0)
		{
			cout<<linecount<<"\r";
			fflush(stdout);
		}
	}
	if(sample.size()>1)
	{
		sample.push_back(end_item);
		data.samples.push_back(sample);
		sample.clear();
	}
	
	cout<<"read data ok! "<<endl;
	cout<<"samples count: "<<data.samples.size()<<endl;
	cout<<"tags count: "<<data.tags_count<<endl;
	cout<<"attrs count: "<<data.attrs_count<<endl;
	cout<<"features count: "<<data.feas_count<<endl;

}
