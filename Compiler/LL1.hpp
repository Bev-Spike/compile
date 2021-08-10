#pragma once
#include<iostream>
#include<map>
#include<set>
#include<Windows.h>
#include<string>
#include<vector>
#include<fstream>
#include<stack>
using namespace std;

void gotoxy(int x, int y)
{
	HANDLE hout;
	COORD coord;
	coord.X = x;
	coord.Y = y;
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hout, coord);
}

//本LL1文法中，用大写英文字母表示非终结符，#表示结束符，e表示ε空字，其他字符为终结符,默认第一个字符为开始符号
class LL1 {
	enum chType {
		TERM,	//终结符
		NTERM,	//非终结符
		EMPTY,	//空字,这里用^字符代替
		END,		//结束符
		DIVEDE	//分隔符"|"
	};

public:
	LL1(string filename) {
		readFile(filename);
		eliminateLeftRecursion();
		init();
		getFirstSet();
		getFollowSet();
		getFATable();

		/*string s = "*E";
		set<char> temp = getFirstSetFromSuf(s);
		FirstSet['E'].insert(temp.begin(), temp.end());*/
	}
	LL1() {};
	//初始化first表和预测分析表
	void init() {
		//初始化终结符firstSet
		for (auto iter = term.begin(); iter != term.end(); iter++) {
			set<char> s;
			s.insert(*iter);
			FirstSet.insert(make_pair(*iter, s));
		}
		//初始化非终结符的First和followSet
		for (auto iter = nterm.begin(); iter != nterm.end(); iter++) {
			set<char> s = {};
			FirstSet.insert(make_pair(*iter, s));
			FollowSet.insert(make_pair(*iter, s));
		}
		//起始符号的follow集添加一个#
		FollowSet[Start].insert('#');

		//初始化预测分析表，表内全部置为空字符串
		for (auto iter = nterm.begin(); iter != nterm.end(); iter++) {
			map<char, string> m;
			for (auto iter0 = term.begin(); iter0 != term.end(); iter0++) {
				m.insert(make_pair(*iter0, ""));
			}
			m.insert(make_pair('#', ""));//单独添加一个结束符
			FATabale.insert(make_pair(*iter, m));
		}
	}
	//打开并读取文件，储存到文法表中
	void readFile(string filename) {
		ifstream ist(filename);
		if (!ist.is_open()) {
			cout << "文件打开失败." << endl;
			exit(0);
		}
		char* str = new char[100];
		//获取开始符号
		Start = ist.get();
		ist.seekg(-1, SEEK_CUR);

		while (ist.getline(str, 100)) {
			string st = str;
			vector<int> divideIndex;//截取文法的索引
			divideIndex.push_back(3);
			for (int i = 0; i < st.length(); i++) {
				if (i == 0) {
					nterm.insert(st[i]);
					i += 2;
				}
				else {
					switch (getType(st[i]))
					{
					case LL1::TERM:
						term.insert(st[i]);
						break;
					case LL1::NTERM:
						nterm.insert(st[i]);
						break;
					case LL1::DIVEDE://这里对分隔符做特殊处理，将含有分隔符的一个文法分为多个文法存储
						divideIndex.push_back(i);
						divideIndex.push_back(i+1);
						break;
					default:
						break;
					}
				}
			}
			divideIndex.push_back(st.length());

			for (size_t i = 0; i < divideIndex.size()-1; i+=2) {
				pair<char, string> p = make_pair(st[0], st.substr(divideIndex[i], divideIndex[i + 1] - divideIndex[i]));
				grammar.insert(p);
			}
		}
	

	}
	//分析程序
	void analysis() {
		cout << "请输入表达式:";
		string in;//输入字符串
		cin >> in;
		in += "#";//在字符串尾加一个终结符
		stack<char> s;//栈
		s.push('#');
		s.push(Start);//压入结束符和开始符号
		bool flag = true;//结束标识
		int idx = 0;
		char a = in[idx];
		int step = 1;
		system("cls");
		cout << "步骤";
		gotoxy(7, 0);
		cout << "堆栈";
		gotoxy(20, 0);
		cout << "字符串" ;
		gotoxy(30, 0);
		cout << "所用产生式" ;
		gotoxy(43, 0);
		cout << "动作 " << endl;
		printAnalysis(s, in, step++, 's', "", "初始化");
		while (flag == true) {
			string text;//动作字符串
			char x = s.top();
			s.pop();
			if (getType(x) == TERM) {
				if (x == a) {//如果栈顶的非终结符和字符串的首字符相同，栈顶出栈，字符串指针后移一位
					
					a = in[++idx];
					text = "获取下一字符";
				}
				else {
					cout << "error";
					exit(0);
				}
			}
			else if(x == '#'){//如果栈顶字符是结束符号
				if (x == a) {
					cout << "分析结束";
					break;
				}
				else {
					cout << "error";
					exit(0);
				}
			}
			else if(FATabale[x][a]!= "") {
				string tempGrama = FATabale[x][a];
				if (tempGrama != "e") {
					for (int i = tempGrama.length() - 1; i >= 0; i--) {
						s.push(tempGrama[i]);
					}
					text = "POP, PUSH(" ;
					text += tempGrama;
					text += ")";
				}
				else {
					text = "POP";
				}
			}
			else {
				cout << "error";
				exit(0);
			}
			string str = in.substr(idx);
			printAnalysis(s, str, step++, x, FATabale[x][a], text);
		}
	}
	//消除左递归
	void eliminateLeftRecursion() {
		for (auto iter = grammar.begin(); iter != grammar.end(); iter++) {
			char tempC;
			if (iter->first == iter->second[0]) {//如果左部和右部第一个字符相同，则需要对该字符消除左递归
				tempC = iter->first;
				char newNterm = 'A';
				for (int i = 0; i < 26; i++) {//找到新的大写字符
					if (nterm.find(newNterm) == nterm.end()) {
						break;
					}
					newNterm++;
				}
				nterm.insert(newNterm);
				for (auto iter1 = grammar.begin(); iter1 != grammar.end(); ) {
					if (iter1->first == tempC) {
						if (iter1->first != iter1->second[0] && iter1->second != "e") {
							string suf = iter->second;
							suf += newNterm;
							iter1->second = suf;
						}
						else if(iter1->first == iter1->second[0]){
							string suf = iter->second.substr(1);
							suf += newNterm;
							auto p = make_pair(newNterm, suf);
							grammar.insert(p);
							auto temp = iter1;
							if (iter1 == iter) {
								iter++;
							}
							iter1++;
							grammar.erase(temp);
							continue;
						}
					}
					iter1++;
				}
				auto p = make_pair(newNterm, "e");
				grammar.insert(p);
			}
		}
	}
	//建立所有终结符和非终结符的First集
	void getFirstSet() {
		bool flag;//设立标识，表示first集有没有发生变化
		//循环直到所有first集都不再变化为止
		do {
			flag = true;
			for (auto iter = grammar.begin(); iter != grammar.end(); iter++) {
				string str = (*iter).second;//文法右部字符串
				set<char> newSet = getFirstSetFromSuf(str);//获取右部字符串的First集
				
				//判断newSet和左部原Set集是否包含，如果不同则置标识符为false，需要再次循环更新first集
				if (!isContain(FirstSet[(*iter).first], newSet)) {
					flag = false;
					FirstSet[(*iter).first].insert(newSet.begin(), newSet.end());//更新First集
				}
			}
			//printTable();
		} while (flag == false);
	}
	//建立所有非终结符的Follow集
	void getFollowSet() {
		
		bool flag;
		do {
			flag = true;
			//对每个非终结符遍历所有文法，更新follow集，直到follow集不再变化
			for (auto iter0 = nterm.begin(); iter0 != nterm.end(); iter0++) {
				char nowNTern = *iter0;
				for (auto iter = grammar.begin(); iter != grammar.end(); iter++) {
					string right = (*iter).second;
					char left  = (*iter).first;

					int idx = right.find(nowNTern);
					if (idx == -1) {//查无此字符
						continue;
					}
					if (idx == right.length() - 1) {//如果在右部的最后一个字符
						if (!isContain(FollowSet[nowNTern], FollowSet[left])) {//如果左部的follow集没有被包含在当前非终结符的Follow集里，将左部的follow集添加到当前非终结符的follow集中
							FollowSet[nowNTern].insert(FollowSet[left].begin(), FollowSet[left].end());
							flag = false;//改变标志，表示还得更新一次。
						}
					}
					else {
						string suf = right.substr(idx+1);
						set<char> tempSet = getFirstSetFromSuf(suf);
						if (!isContain(tempSet, 'e')) {//不含空字，可直接将后缀的first集全部添加进去
							if (!isContain(FollowSet[nowNTern], tempSet)) {
								FollowSet[nowNTern].insert(tempSet.begin(), tempSet.end());
								flag = false;//改变标志，表示还得更新一次。
							}
						}
						else {//含空字，则将左部的follow集和后缀的first集的并集添加进去
							set<char> Union = {};
							Union.insert(FollowSet[left].begin(), FollowSet[left].end());
							Union.insert(tempSet.begin(), tempSet.end());
							Union.erase('e');//把空字去掉
							if (!isContain(FollowSet[nowNTern], Union)) {
								FollowSet[nowNTern].insert(Union.begin(), Union.end());
								flag = false;//改变标志，表示还得更新一次。
							}
						}
					}
				}
			}
		} while (flag == false);
	}
	//获得预测分析表
	void getFATable() {
		for (auto iter1 = grammar.begin(); iter1 != grammar.end(); iter1++) {
			set<char> temp = getSelect(*iter1);//获取该文法的select集
			for (auto iter2 = temp.begin(); iter2 != temp.end(); iter2++) {
				FATabale[iter1->first][*iter2] = iter1->second;
			}
		}
	}

	//把能输出的东西都输出出来
	void printTable() {
		cout << "终结符集 :";
		
		for (auto iter = term.begin(); iter != term.end(); iter++) {
			cout << *iter << ", ";
		}
		cout << endl;
		cout << "------------------------------------------------------" << endl;

		cout << "非终结符集： ";
		for (auto iter = nterm.begin(); iter != nterm.end(); iter++) {
			cout << *iter << ", ";
		}
		cout << endl;
		cout << "------------------------------------------------------" << endl;
		cout << "gramma集：";
		auto iter = grammar.begin();
		while (iter != grammar.end()) {
			cout << iter->first << ":" << iter->second << endl;
			iter++;
		}
		cout << endl << "------------------------------------------------------" << endl;
		cout << "first集 :" << endl;
		auto iter1 = FirstSet.begin();
		while (iter1 != FirstSet.end()) {
			cout << iter1->first << ":";
			for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
				cout << *iter2 << ",";
			}
			cout << endl;
			iter1++;
		}
		cout << endl << "------------------------------------------------------" << endl;
		cout << "follow集 :" << endl;
		auto _iter = FollowSet.begin();
		while (_iter != FollowSet.end()) {
			cout << _iter->first << ":";
			for (auto iter2 = _iter->second.begin(); iter2 != _iter->second.end(); iter2++) {
				cout << *iter2 << ",";
			}
			cout << endl;
			_iter++;
		}
		cout << endl << "------------------------------------------------------" << endl << endl;
		cout.width(10);
		cout << "";
		cout.width(10);
		cout << left << '#' ;
		for (auto idx = term.begin(); idx != term.end(); idx++) {
			cout.width(10);
			cout <<left<< *idx;
		}
		cout << endl;
		for (auto _iter1 = FATabale.begin(); _iter1 != FATabale.end(); _iter1++) {
			map<char, string>* mapCol = &(_iter1->second);//为了可读性，用指针表示。
			cout.width(10);
			cout << left << _iter1->first;
			for (auto _iter2 = mapCol->begin(); _iter2 != mapCol->end(); _iter2++) {
				cout.width(10);
				cout << left << _iter2->second;
			}
			cout << endl;

		}
		cout << "------------------------------------------------------" << endl;
	}
private:
	//返回当前字符的类型
	chType getType(char ch) {
		if (ch >= 'A' && ch <= 'Z') return NTERM;
		else if (ch == 'e') return EMPTY;
		else if (ch == '#') return END;
		else if (ch == '|') return DIVEDE;
		else return TERM;
	}
	//从给定的字符串中获取其first集
	set<char> getFirstSetFromSuf(string suf) {
		set<char> s = {};
		if (suf.length() == 0) {
			return s;
		}
		//如果后缀只有空字，则直接返回空字
		if (suf == "e") {
			s.insert('e');
			return s;
		}
		//如果第一个字符就是终结符，则将其自身返回
		if (getType(suf[0]) == TERM) {
			s.insert(suf[0]);
			return s;
		}
		//如果不是终结符，就是非终结符，把其除了空字以外的first集全部加入set中
		s.insert(FirstSet[suf[0]].begin(), FirstSet[suf[0]].end());
		if (isContain(s, 'e')) {//如果空字存在，则将其去除,并对后面的字符串进行递归
			s.erase('e');
			string tempStr = suf.substr(1, suf.length() - 1);
			set<char> tempSet = getFirstSetFromSuf(tempStr);//进行递归
			s.insert(tempSet.begin(), tempSet.end());
		}
		
		if (suf.length() == 1 && isContain(FirstSet[suf[0]], 'e')) {//如果给定后缀只有一个字符且其First集包含空字，则将空字加入当前first集中
			s.insert('e');
		}
		
		return s;
	}
	//判断两个集合是否相等
	template <class T>
	bool isSetEqual(set<T> set1, set<T> set2) {
		if (set1.size() != set2.size()) return false;
		typename set<T>::iterator it1;
		typename set<T>::iterator it2;
		for (it1 = set1.begin(), it2 = set2.begin(); it1 != end(set1); it1++, it2++) {
			if (*(it1) != *(it2)) {
				return false;
			}
		}
		return true;
	}
	//set2是否包含在set1中
	template <class T>
	bool isContain(set<T> set1, set<T> set2) {
		if (set2.size() > set1.size()) return false;
		typename set<T>::iterator it;
		for (it = set2.begin(); it != set2.end(); it++)
			if (set1.find(*it) == set1.end())
				return false;
		return true;
	}
	//判断元素c是否在set1中
	template <class T>
	bool isContain(set<T> set1, T c) {
		if (set1.find(c) == set1.end())
			return false;
		else
			return true;
	}

	void printSet(set<char> s) {
		cout << "printset:";
		for (auto iter = s.begin(); iter != s.end(); iter++) {
			cout << *iter << " ";
		}
		cout << endl;
	}
	//取得给定文法的select集
	set<char> getSelect(pair<char, string> p) {
		set<char> s = getFirstSetFromSuf(p.second);
		if (!isContain(s, 'e')) {
			return s;
		}
		else {
			s.erase('e');
			s.insert(FollowSet[p.first].begin(), FollowSet[p.first].end());
			return s;
		}
	}
	//输出分析过程
	void printAnalysis(stack<char> s ,string str, int step, char c, string gra, string action) {
		stack<char> st;
		gotoxy(0, step);
		cout << "第";
		cout.width(2);
		cout << step << "步:  ";
		gotoxy(7, step);
		while(!s.empty()) {
			st.push(s.top());
			s.pop();
		}
		//输出栈内状态;
		while (!st.empty()) {
			cout << st.top();
			st.pop();
		}
		gotoxy(20, step);
		cout << str;
		if (gra != "") {
			gotoxy(30, step);
			cout << c << "->" << gra;
		}
		gotoxy(40, step);
		cout << action << endl;
		cout << endl;
	}

	//取得字符串中指定字符的所有后缀
	string getSuf(string str, char c) {
		int idx = str.find(c);
		if (idx == -1) {//查无此字符
			return "";
		}
		return str.substr(idx);
	}
private:
	char Start ;
	multimap<char, string> grammar;//文法表
	set<char> term;//终结符集
	set<char> nterm;//非终结符集
	map<char, map<char, string>> FATabale;//预测分析表
	map<char, set<char>> FirstSet;//所有元素对应的first集
	map<char, set<char>> FollowSet;//follow集
};

