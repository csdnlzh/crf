import sys
infile=open(sys.argv[1])
outfile=open(sys.argv[2],"w")


def toword(sentence):
	words=[]
	word=[]
	for item in sentence:
		if item[0]=="S":
			words.append(item[1])
		if item[0]=="B":
			word.append(item[1])
		if item[0]=="M":
			word.append(item[1])
		if item[0]=="E":
			word.append(item[1])
			words.append("".join(word))
			word=[]
	
	outfile.write(" ".join(words))
	outfile.write("\n")
	

sentence=[]
for line in infile.readlines():
	line=line.strip()
	if line=="":
		toword(sentence)
		sentence=[]
	else:
		sentence.append(line.split())
