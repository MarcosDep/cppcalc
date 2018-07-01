#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <math.h>

using namespace std;


// FOR ERRORS OUTPUT
void Error(string s1)
{
	throw runtime_error(s1);
}

void Error(string s1, string s2)
{
	throw runtime_error(s1+s2);
}
// CONSTANTS

const char number = '8';
const char quit = 'q';
const string quitkey = "exit";
const char print = ';';
const string prompt = "> ";
const string result = "= ";

const char let = 'L';
const char constkey = 'C';
const char name = 'a';
const string declkey = "let";
const string constdeclkey = "const";

const char func = 'f';


// PROTOTYPES
void calculate();

double statement();
double expression();
double term();
double spec();
double primary();

void clean_up_mess();

// Variables
double get_value(string s);
void set_value(string s, double d);
bool is_declared(string var_name);
double define_name(string var, double val, char type);
double declaration(char type);

double name_handler();

//Functions
bool is_func(string);
void define_func(string);

// CLASS DEFENITIONS

//TOKS
class Token
{
public:
	char kind;
	double value;
	char oper;
	string name;
	Token(char ch):
		kind(ch), value(0){}

	Token(char ch, double val):
		kind(ch), value(val){}
	Token(char ch, string s):
		kind(ch), name(s){}
};

//TOKSTREAM
class TokenStream
{
public:
	TokenStream();
	Token get();
	void putback(Token t);
	void ignore(char c);
private:
	bool full;
	Token buffer;
};

//VARS
class Variable
{
public:
	Variable(string n, double v, char s):
		name(n), value(v), type(s){}
	bool is_const();
	string name;
	double value;
	char type;
};





// CLASS IMPLEMINTATIONS
bool Variable::is_const()
{
	if(type == 'C') return true;
	return false;
}


TokenStream::TokenStream():
full(false), buffer(0){}


void TokenStream::putback(Token t) //Put back Token into a stream
{
	if(full) Error("Buffer is not empty!");
	buffer = t;
	full = true;
}

void TokenStream::ignore(char c) // Ignore all from begib to 'c' symbol
{
	if(full && c==buffer.kind)
	{
		full = false;
		return;
	}
	full = false;
	char ch = 0;
	while(cin >> ch)
		if(ch == c) return;
}


Token TokenStream::get() // Get a Token from a stream
{
	if(full)
	{
		full = false;
		return buffer;
	}
	char ch;
	cin >> ch;
	switch(ch)
	{
		case 'q': case print: case '+': case '-': case '*': case '/': case '(': case ')':
		case '{': case '}': case '!': case '%': case '=': case ',':
		return Token(ch);

		case '.': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
		case number: case '9':
		{
			cin.putback(ch);
			double val;
			cin >> val;
			return Token(number, val);
		}
		default:
		{
			if(isalpha(ch))
			{
				string s;
				s += ch;
				while(cin.get(ch) && (isalpha(ch) || ch == '_' || isdigit(ch)))
				{
					s += ch;
				}
				cin.putback(ch);
				if(s == declkey) return Token(let); // 'let' statement
				else if(s == constdeclkey) return Token(constkey);
				else if(s == quitkey) return Token(quit); // 'exit' statement
				return Token(name, s); // variable name
				
			}
			Error("Invalid lexem!");	
		}
	
	}
}
 

TokenStream ts;
vector<Variable> var_table;
vector<string> func_table;

// Two logical parts of programm:
// 1. main() function describe begining and ending of the program
// 2. main() execute a loop of calculating
int main()
{
	cout << "\n\tWelcome to My Calculator.\n"
		<< "\tAllowed operations: +, -, !, *, /.\n"
		<< "\tFunctions: sqrt(x), pow(x, i).\n"
		<< "\tAlso allowed to define own variables(Syntax: let 'name' = 'value';)\n"
		<< "\tAll expressions shall to end with ';'.\n"
		<< "\tTo quit from an application, type 'q' or 'exit'.\n"
		<< "\tType an expression(Example: 2+2*2):\n ";
	try
	{
		define_func("sqrt");
		define_func("pow");
		define_name("pi", 3.14, 'C');

		calculate(); // Calculating loop

		system("PAUSE");
		return 0;
	}
	catch(runtime_error& e)
	{
		cerr << e.what() << endl;
	}
	catch(...)
	{
		cerr << "Exception \n";
	}

}

// FUNCTIONS IMPLEMENTATIONS

void calculate() // Calculating loop
{
	while(cin)
	try
	{
	
		cout << prompt; 
		Token t = ts.get();
		while(t.kind == print)
		{ 
			t = ts.get();
		}
		if(t.kind == quit) return;
		ts.putback(t);
		cout << result << statement() << endl;
	}catch(exception& e)
	{
		cerr << "Exception handler in void calculate();!\n"; // DEBUG!@
		cerr << e.what() << endl;
		clean_up_mess();
	}

}


double expression()
{
	double left = term();
	Token t = ts.get();
	while(true)
	{
		switch(t.kind)
		{
			case '+':
			{
				left += term();
				t = ts.get();
				break;
			}
			case '-':
			{
				left -= term();
				t = ts.get();
				break;
			}
			default:
			{
				ts.putback(t);
				return left;
			}
		}
	}

}

double term()
{
	double left = spec();
	Token t = ts.get();
	while(true)
	{
		switch(t.kind)
		{
			case '*':
			{
				left *= spec();
				t = ts.get();
				break;
			}
			case '/':
			{
				double d = spec();
				if(d == 0)
					Error("Division by zero!");
				left /= d;
				t = ts.get();
				break;
			}
			case '%':
			{
				double d = term();
				int i1 = int(left);
				if(i1 != left)
					Error("left operand % not an integer!");
				int i2 = int(d);
				if(i2 != d)
					Error("right operand % not an integer!");
				if(i2 == 0) 
					Error("%: Division by zero!");
				left = i1 % i2;
				t = ts.get();
				break;
			}
			default:
			{
				ts.putback(t);
				return left;
			}
		}
	}
}

double spec()
{
	double left = primary();
	Token t = ts.get();
		switch(t.kind)
		{
			case '!':
			{
				for(int i = left-1; i >= 1; i--)
				{
					left *= i; 
				}
				return left;
			}
			default:		
			{
				ts.putback(t);
				return left;
			}
		}
}

double primary()
{
	Token t = ts.get();
	switch(t.kind)
	{
		case '(':
		{
			double d = expression();
			t = ts.get();
			if(t.kind != ')')
				Error("')' expected!");
			return d;
		}
		case '{':
		{
			double d = expression();
			t = ts.get();
			if(t.kind != '}')
				Error("'}' expected!");
			return d;
		}
		case number:
			return t.value;
		case '-':
			return - primary();
		case '+':
			return primary();

		default:
		{
			if(t.kind == name)
			{
				ts.putback(t);
				return name_handler();
			}
			cout << t.kind << endl;
			Error("Primary expression expected!");
		}
	}
}

double name_handler()
{
	Token t = ts.get();
	if(is_declared(t.name)) // if a variable
	{
		Token t1 = ts.get();
		if(t1.kind == '=') // assignment
		{
			set_value(t.name, expression());
			return get_value(t.name);	
		}
		ts.putback(t1);
		return get_value(t.name); 
	}	
	else if(is_func(t.name))
	{
		if(t.name == "sqrt")
		{
			Token t1 = ts.get();
			if(t1.kind != '(') Error("'(' expected!");
			double d = expression();
			t1 = ts.get();
			if(t1.kind != ')') Error("')' expected!");
			return sqrt(d);
		}
		else if(t.name == "pow")
		{
			Token t1 = ts.get();
			if(t1.kind != '(') Error("'(' expected!");
			double d1 = expression();
			t1 = ts.get();
			if(t1.kind != ',') Error("Not enough arguments!");
			double d2 = expression();
			t1 = ts.get();
			if(t1.kind != ')') Error("')' expected!");
			int iTest = int(d2);
			if(d2 != iTest) Error("2d argument is not an integer!");
			return pow(d1, d2);
		}
	}
	return 2;
}

void clean_up_mess()
{
	ts.ignore(print);

}

double get_value(string s)
{
	for(int i = 0; i < var_table.size(); i++)
	{
		if(var_table[i].name == s)
			return var_table[i].value;
	}	
	Error("get: undefined variable ", s);

}

void set_value(string s, double d)
{
	for(int i = 0; i < var_table.size(); ++i)
	{
		if(var_table[i].name == s)
		{
			if(var_table[i].is_const())
			{	
				Error("This variable is constant!");
			}
			else
			{
				var_table[i].value = d;
			}
		}
	
	}
			Error("set: undefined variable ", s);

}

bool is_declared(string var_name)
{
	for(int i = 0; i < var_table.size(); ++i)
		if(var_table[i].name == var_name) return true;
		
	return false;
	

}

double define_name(string var, double val, char type)
{
	if(is_declared(var)) Error(var, " declared twice!");
	var_table.push_back(Variable(var, val, type));
	return val;

}


double statement()
{
	Token t = ts.get();
	switch(t.kind)
	{
		case let:
			return declaration(let);
		case constkey:
			return declaration(constkey);
		default:
			ts.putback(t);
			return expression();
	}

}


double declaration(char type)
{
	Token t = ts.get();
	if(t.kind != name) Error("'Name' expected!");
	string var_name = t.name; 
	Token t2 = ts.get();
	if(t2.kind != '=') Error("'=' expected!");
	double d = expression();
	define_name(var_name, d, type);
	return d;
}


bool is_func(string func_name)
{
	for(int i = 0; i < func_table.size(); ++i)
	{
		if(func_table[i] == func_name)
			return true;
	}
	return false;
}

void define_func(string name)
{
	func_table.push_back(name);
}
