#include<iostream>
#include<string>
#include<fstream>
#include<vector>
using namespace std;


class LexAnalysis {
public:
	//关键词的枚举类型，作为助记符
	enum keyType {
		NONE = -1,	//什么也不是
		KEY_IF = 0,	//关键字if 下同，都是小写
		KEY_FOR = 1,
		KEY_ELSE = 2,
		KEY_WHILE = 3,
		KEY_DO = 4,
		KEY_RETURN = 5,
		ID = 10,	//标识符
		DIGIT = 11, //数字
		ERROR = 15,	//错误标识
		SEMICOLON = 20,	//分号“;”
		ASSIGN = 21,	//等号“=”
		PLUS = 22,	//加号“+”
		MINUS = 23,	//减号“-”
		STAR = 24,	//乘号“*”
		DIVISION = 25,	//除号“/”
		COLON = 26,	//冒号
		LPAR = 27,	//左括号“(”
		RPAR = 28,	//右括号“)”
		LBRACE = 29,	//左大括号“{”
		RBRACE = 30,	//右大括号“}”
		COMMA = 31,	//逗号“,”
		ENTER = 32,//回车“\n”
	};
	enum State {
		STATE_START = 1,//开始态
		STATE_IDENT = 2,//标识符
		STATE_KEYWORD = 3,//关键字
		STATE_DIGIT = 4,//数字
		STATE_DELIM = 5,//分界符
		STATE_DONE = 6,	//当前token结束状态
		STATE_ERROR = 7, //错误态
	};

	//单词表，储存包括单词本身、单词种别、单词属性、位置等属性
	struct wordTable {
		string word;
		int index;//相同标识符的索引
		keyType type;//词种类
		int row;//行
		int col;//列
	};
public:
	LexAnalysis(string filename) {
		openFile(filename);
		init();
		row = 1;
		col = 0;

	}
	//初始化分界符、关键词、标识符表
	void init() {
		keyword.push_back("do");
		keyword.push_back("while");
		keyword.push_back("for");
		keyword.push_back("if");
		keyword.push_back("return");
		keyword.push_back("int");

		delimiter.push_back(";");
		delimiter.push_back("=");
		delimiter.push_back("+");
		delimiter.push_back("-");
		delimiter.push_back("*");
		delimiter.push_back("/");
		delimiter.push_back(":");
		delimiter.push_back("(");
		delimiter.push_back(")");
		delimiter.push_back("{");
		delimiter.push_back("}");
		delimiter.push_back(",");
		delimiter.push_back("\n");
		delimiter.push_back(" ");
		delimiter.push_back("\t");
	}
	~LexAnalysis() {
		ist.close();
		/*ofstream file;
		file.open("code.txt",ios::in);
		file.seekp(-1, ios::end);
		file << "1";
		file.close();*/
	}
	void openFile(string fileName) {
		ist = ifstream(fileName);
		if (!ist.is_open()) {
			cout << "文件打开失败." << endl;
			exit(0);
		}
		Pretreatment();
	}
	//主程序，逐行扫描所有字符，生成单词表
	void Scan() {
		char ch;//临时读取的字符
		string tempLine;
		string token = "";//起始token为空串
		//int nRow = row, nCol = col;//临时行列数，用于换行
		string delim;
		State state = STATE_START;
		while (ist.peek() != EOF) {
			switch (state)
			{
			case STATE_START:
				getBC();
				ch = ist.get();
				token = "";
				if (isLetter(ch)) {
					state = STATE_IDENT;
					token += ch;
				}
				else if (isDigit(ch)) {
					state = STATE_DIGIT;
					token += ch;
				}
				else if (isDelimiter(ch)) {
					token += ch;
					state = STATE_DELIM;
				}
				else {
					state = STATE_ERROR;
				}
				break;
			case STATE_IDENT://标识符状态，也有可能是关键字
				ch = ist.get();
				if (isLetter(ch) || isDigit(ch)) {
					token += ch;
				}
				else if (isDelimiter(ch)) {//如果是分界符，直接结束,并回退一个字符
					state = STATE_DONE;
					reTract();

				}
				else {//其他字符,进入错误状态
					state = STATE_ERROR;
				}

				break;
			case STATE_DIGIT:
				ch = ist.get();
				if (isDigit(ch)) {
					token += ch;
				}
				else if (isDelimiter(ch)) {
					state = STATE_DONE;
					reTract();
				}
				else {//如果既不是数字也不是分界符，说明语法错误
					state = STATE_ERROR;
				}
				break;
			case STATE_DELIM:
				//ch = ist.get();
				delim = "";
				delim += ch;
				switch (ch)
				{
				case '(':
					col++;
					insertId(delim, LPAR);
					break;
				case ')':
					col++;
					insertId(delim, RPAR);
					break;
				case '{':
					col++;
					insertId(delim, LBRACE);
					break;
				case '}':
					col++;
					insertId(delim, RBRACE);
					break;
				case ',':
					col++;
					insertId(delim, COMMA);
					break;
				case ';':
					col++;
					insertId(delim, SEMICOLON);
					break;
				case ':':
					col++;
					insertId(delim, COLON);
					break;
				case '+':
					col++;
					insertId(delim, PLUS);
					break;
				case '-':
					col++;
					insertId(delim, MINUS);
					break;
				case '*':
					col++;
					insertId(delim, STAR);
					break;
				case '/':
					col++;
					insertId(delim, DIVISION);
					break;
				case ' ':
					getBC();
					break;
				case '\n'://换行符
					//col++;
					//insertId(delim, ENTER);
					col = 0;
					row++;
					break;
				default:
					break;
				}
				state = STATE_START;
				break;
			case STATE_DONE:
				keyType t;
				col++;
				
				if (isDigit(token[0])) {
					insertId(token, DIGIT);

				}
				else if ((t = Reserver_Key(token)) != NONE) {
					insertId(token, t);
				}
				else {//若不是关键字，则为标识符
					int n;
					if ((n = Reserver_Identi(token)) != -1) {//已存在于标识符表中
						insertId(token, ID, n);
					}
					else {
						identifier.push_back(token);
						insertId(token, ID, identifier.size() - 1);
					}

				}
				state = STATE_START;
				break;
			case STATE_ERROR:
				//错误态，则直接读取字符直到分界符出现
				do {
					token += ch;
					ch = ist.get();
				} while (!isDelimiter(ch));
				reTract();
				col++;
				insertId(token, ERROR);
				state = STATE_START;
				break;
			default:
				break;
			}


		}

	}
	//将表输出出来
	void printTable() {
		cout.width(15);
		cout.setf(ios::left);
		cout << "单词" <<"二元序列                 "<<"类型          " <<"位置" << endl;
		for (size_t i = 0; i < table.size(); i++) {
			switch (table[i].type) {
			case LexAnalysis::KEY_IF:
				printLine(table[i].word, table[i].type, table[i].word, "关键字");

				break;
			case LexAnalysis::KEY_FOR:
				//cout << "for" << "           (" << table[i].type << ", for)           关键字         ";
				printLine(table[i].word, table[i].type, table[i].word, "关键字");

				break;
			case LexAnalysis::KEY_ELSE:
				//cout << "else" << "          (" << table[i].type << ", else)          关键字         ";
				printLine(table[i].word, table[i].type, table[i].word, "关键字");

				break;
			case LexAnalysis::KEY_WHILE:
				//cout << "while" << "         (" << table[i].type << ", while)         关键字         ";
				printLine(table[i].word, table[i].type, table[i].word, "关键字");

				break;
			case LexAnalysis::KEY_DO:
				//cout << "do" << "            (" << table[i].type << ", do)            关键字         ";
				printLine("do", table[i].type, table[i].word, "关键字");
				break;
			case LexAnalysis::KEY_RETURN:
				//cout << "return" << "        (" << table[i].type << ", return)        关键字         ";
				printLine(table[i].word, table[i].type, table[i].word, "关键字");
				break;
			case LexAnalysis::ID:
				//cout << table[i].word << "            (" << table[i].type << ", " << table[i].word  << ")            标识符         ";
				printLine(table[i].word, table[i].type, table[i].word, "标识符");

				break;
			case LexAnalysis::ERROR:
				//cout << table[i].word << "            (" << table[i].type << ", error)            错误        ";
				printLine(table[i].word, table[i].type, "ERROR", "ERROR");

				break;
			case LexAnalysis::SEMICOLON:
				//cout << ";" << "            (" << table[i].type << ", ;)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");

				break;
			case LexAnalysis::ASSIGN:
				//cout << "=" << "            (" << table[i].type << ", =)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::PLUS:
				//cout << "+" << "            (" << table[i].type << ", +)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::MINUS:
				//cout << "-" << "            (" << table[i].type << ", -)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::STAR:
				//cout << "*" << "            (" << table[i].type << ", *)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::DIVISION:
				//cout << "/" << "            (" << table[i].type << ", /)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::COLON:
				//cout << ":" << "            (" << table[i].type << ", :)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::LPAR:
				//cout << "(" << "            (" << table[i].type << ", ()            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::RPAR:
				//cout << ")" << "            (" << table[i].type << ", ))            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");

				break;
			case LexAnalysis::LBRACE:
				//cout << "{" << "            (" << table[i].type << ", {)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::RBRACE:
				//cout << "}" << "            (" << table[i].type << ", })            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::COMMA:
				//cout << "," << "            (" << table[i].type << ", ,)            分界符         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			case LexAnalysis::ENTER:
				//cout << "\\n" << "            (" << table[i].type << ", \\n)            分界符         ";
				printLine("\\n", table[i].type, "\\n", "分界符");

				break;
			case LexAnalysis::DIGIT:
				//cout << table[i].word << "            (" << table[i].type << ", " << table[i].word << ")            常数         ";
				printLine(table[i].word, table[i].type, table[i].word, "分界符");
				break;
			default:
				break;
			}
			cout << "(" << table[i].row << ", " << table[i].col << ")"<<endl;
		}
	}
	void printLine(string s1, keyType type, string word, string s2) {
		if (Reserver_Key(s1) != NONE) {
			type = (keyType)1;
		}
		cout.width(15);
		cout << s1;
		cout << "(";
		cout.width(2);
		cout << type;
		cout << ",";
		cout.width(7);
		cout.setf(ios::right);
		cout << word;
		cout.unsetf(ios::right);
		cout.setf(ios::left);
		cout.width(15);
		cout << ")";
		cout.width(15);
		cout << s2;
	}
private:
	//预处理，这里往结尾加了一个空格
	void Pretreatment() {
		ist.close();
		ofstream file;
		file.open("code.txt", ios::app);
		file << " ";
		file.close();
		ist.open("code.txt");
	}
	//判断是否为英文字母
	bool isLetter(char ch) {
		return (ch <= 'z' && ch >= 'a' || ch <= 'Z' && ch >= 'A');
	}
	//判断是否为数字
	bool isDigit(char ch) {
		return (ch <= '9' && ch >= '0');
	}
	//判断是否为分界符
	bool isDelimiter(char ch) {
		string s = "";
		s += ch;
		for (size_t i = 0; i < delimiter.size(); i++) {
			if (s == delimiter[i]) {
				return true;
			}
		}
		return false;

	}
	bool isDelimiter(string token) {
		for (size_t i = 0; i < delimiter.size(); i++) {
			if (token == delimiter[i]) {
				return true;
			}
		}
		return false;

	}
	//跳过当前输入流除了\t以外的空字符
	void getBC() {
		char ch;
		ist.get(ch);
		while (ch == ' ' || ch == '\t') {
			ist.get(ch);
			if (ist.peek() == EOF) {
				return;
			}
		};
		reTract();//回退一格
	}
	//文档指针回退一格
	void reTract() {
		ist.seekg(-1, SEEK_CUR);
	}
	//将识别到的字符串放入到单词表中
	void insertId(string token, keyType type, int idx = 0) {
		wordTable t;
		t.col = col;
		t.row = row;
		t.index = idx;
		t.word = token;
		t.type = type;
		table.push_back(t);

	}
	//将识别到的token与关键词表中的匹配，返回其种别编码
	keyType Reserver_Key(string token) {
		int flag = 0;
		for (size_t i = 0; i < keyword.size(); i++) {
			if (token == keyword[i]) {
				flag = 1;
				break;
			}
		}
		if (flag == 1) {
			if (token == "if") return  KEY_IF;
			if (token == "for") return KEY_FOR;
			if (token == "else") return  KEY_ELSE;
			if (token == "while") return  KEY_WHILE;
			if (token == "return") return KEY_RETURN;
			if (token == "do") return KEY_DO;
		}
		else {
			return NONE;
		}
	}
	//将识别到的token与标识符表中的匹配，返回其索引
	int Reserver_Identi(string token) {
		for (size_t i = 0; i < identifier.size(); i++) {
			if (token == identifier[i]) {
				return i;
			}
		}
		return -1;
	}
private:
	vector<string> keyword;//关键字数组
	vector<string> delimiter;//分界符数组
	vector<string> identifier;//标识符数组
	ifstream ist;
	int row, col;
	vector<wordTable> table;

};