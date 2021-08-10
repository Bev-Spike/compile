#include<iostream>
#include"LexAnalysis.hpp"
#include"LL1.hpp"
using namespace std;

int main() {
	string filename = "Gramma.txt";
	LL1 ll1(filename);
	ll1.printTable();
	ll1.analysis();
	//LexAnalysis lex(filename);
	//lex.Scan();
	//lex.printTable();
}