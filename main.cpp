/**
  Daniel Wang
  Professor Wilkinson
  CS 45
  Test 2
  Implements set operations and sllows user to store values, load/save, and display variables
  **/




#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

//translating integer into color set
void processLine(string &line, int set[], string setMembers[]);

//receive input from user
bool getInput(string & line);
//populate set with set members of colors
void loadSetMembers(string setMembers[]);

vector<string> tokenize(string &line);
vector<string> transformTokens(vector<string> tokens, int set[], string setMembers[]);
//remove braces and replace commas with space
void standardize(string &line);

//converts from infix to postfix
vector<string> infixToPostfix(vector<string> expression);

//evaluates postfix expression
int evaluateRPN(vector<string> newRPN);


//for LIST
void listSets(int set[], string setMembers[]);
//for SHOW
void showSet(string &line, int set[], string setMembers[]);
//for SAVE
void writeFile(string filename, int set[]);
//for LOAD
void readFile(string fileName, int set[]);
//for HELP
void help();


//bitwise operators
int setUnion(int x, int y);
int setIntersection(int x, int y);
int setCompliment(int x);
int isSubset(int x, int y);
int isSuperset(int x, int y);
int isEqual(int x, int y);

//displays integer value for setMembers
void process(int colorCode, string setMembers[]);


int main()
{
    string setMembers[16];
    loadSetMembers(setMembers);
    int set[26] = {0};
    string line;
    while(getInput(line))
    {
        processLine(line, set, setMembers);
    }


    return 0;
}






bool getInput(string &line)
{
    cout<<"Input: ";
    getline(cin, line);
    return !line.empty();
}




void processLine(string &line, int set[], string setMembers[])
{
    stringstream ss;
    string firstWord;
    for(int i = 0;i<line.size();i++)
    {
        line[i] = toupper(line[i]);
    }
    ss << line;
    ss >> firstWord;

    //branch statements for processing

    if(firstWord == "SET")
    {
        vector<string> tokens;
        string varToSet;
        ss >> varToSet;
        int indexOfEqual;
        for(int i = 0;i<line.size();i++)
        {
            if(line[i] == '=')
                indexOfEqual = i;
        }
        line = line.substr(indexOfEqual + 2);
        //direct integer assignment
        if (isdigit(line[0]) || isdigit(line[1]))
        {
            set[varToSet[0]-65] = stoi(line);
        }
        //otherwise tokenize and assess using rpn
        else
        {
            tokens = tokenize(line);
            tokens = transformTokens(tokens, set, setMembers);
            tokens = infixToPostfix(tokens);
            evaluateRPN(tokens);
            set[varToSet[0] - 65] = evaluateRPN(tokens);
        }
    }
    else if (firstWord == "SHOW")
    {
        showSet(line, set, setMembers);
    }
    else if (firstWord == "LIST")
    {
           listSets(set,setMembers);
    }
    else if (firstWord == "HELP")
    {
        help();
    }
    else if (firstWord == "LOAD" )
    {
        line = line.substr(5);
        readFile(line, set);
    }
    else if (firstWord == "SAVE")
    {
        line = line.substr(5);
        writeFile(line, set);

    }
    //any of the commands for evaluating
    else if (isalpha(line[0]) || line[0] == '~')
    {
        vector<string> tokens;
        tokens = tokenize(line);
        tokens = transformTokens(tokens, set, setMembers);
        tokens = infixToPostfix(tokens);
        if (evaluateRPN(tokens) < 65535)
         process(evaluateRPN(tokens),setMembers);
        else if (evaluateRPN(tokens) == 65536)
            cout<<"true"<<endl;
        else
            cout<<"false"<<endl;

    }
    else
        cout<<"ILLEGAL INPUT!!"<<endl;

}

//set members
void loadSetMembers(string setMembers[])
{
    setMembers[0] = "WHITE";
    setMembers[1] = "GOLD";
    setMembers[2] = "GREY";
    setMembers[3] = "RED";
    setMembers[4] = "GREEN";
    setMembers[5] = "BLUE";
    setMembers[6] = "ORANGE";
    setMembers[7] = "PURPLE";
    setMembers[8] = "YELLOW";
    setMembers[9] = "VIOLET";
    setMembers[10] = "MAGENTA";
    setMembers[11] = "CYAN";
    setMembers[12] = "RUST";
    setMembers[13] = "NAVY";
    setMembers[14] = "BURGUNDY";
    setMembers[15] = "BLACK";
}



//tokenizes line by separating operations and operands
vector<string> tokenize(string &line)
{
    vector<string> tokens;
    for(size_t i = 0;i<line.size();i++)
    {
        if (line[i] == '{')
        {
            string s = "";
            int j;
            for(j = i;j <line.size() && line[j] != '}';j++)
            {
                s += line[j];
            }
            s += line[j];
            tokens.push_back(s);
            i = j;

        }
        else
        {
            string s = "";
            s += line[i];
            tokens.push_back(s);
        }


    }
    return tokens;

}


//change variable and set tokens into integer representations
vector<string> transformTokens(vector<string> tokens, int set[], string setMembers[])
{
    for(size_t i = 0;i<tokens.size();i++)
    {
        if (isalpha(tokens[i][0]))
        {
            tokens[i] = to_string(set[tokens[i][0] - 'A']);
        }

        else if (tokens[i][0] == '{')
        {
            stringstream ss;
            string color;
            standardize(tokens[i]);
            int value = 0;
            ss << tokens[i];
            while (ss >> color)
            {
                for(size_t i = 0;i< 16;i++)
                {
                    if (color == setMembers[i])
                    {
                        value += pow(2,i);
                    }
                }
                cout<<"COLOR NOT FOUND"<<endl;
                break;
            }
            tokens[i] = to_string(value);
        }
    }


    return tokens;
}

//remove brace replace commas with space
void standardize(string &line)
{
    size_t pos = 0;
    line = line.substr(1,line.size() - 2);
    while((pos = line.find(',')) < string::npos)
        line[pos] = ' ';
}

//uses shunting yard algorithm to convert infix notation of expression to postfix
vector<string> infixToPostfix(vector<string> expression)
{
    string precedence = "~*+<>=";
    vector<string> outputQueue;
    vector<string> operatorStack;
    //while there are tokens to be read
    for(size_t i = 0;i<expression.size();i++)
    {

        if(isdigit(expression[i][0]) || isdigit(expression[i][1]))
            outputQueue.push_back(expression[i]);
        else if (expression[i] == "(")
            operatorStack.push_back(expression[i]);
        else if (expression[i] == ")")
        {

            while(!operatorStack.empty() && operatorStack.back() != "(")
            {
                outputQueue.push_back(operatorStack.back());
                operatorStack.pop_back();
            }
            operatorStack.pop_back();
        }
        else if (precedence.find(expression[i]) + 1)
        {

            while(!operatorStack.empty() && precedence.find(expression[i]) >= precedence.find(operatorStack.back()))
            {
                    outputQueue.push_back(operatorStack.back());
                    operatorStack.pop_back();


            }
            operatorStack.push_back(expression[i]);
        }
        else
        {
            cout<<"Unrecognized operator or operand!"<<endl;

            exit(1);
        }



    }
    while(!operatorStack.empty())
    {
        outputQueue.push_back(operatorStack.back());
        operatorStack.pop_back();
    }
    return outputQueue;
}

//evaluate RPN expression
int evaluateRPN(vector<string> newRPN)
{
        string precedence = "~*+<>=";
        //while there are input tokens left
        vector<int> stack;
        int index = 0;
        //follows RPN algorithm based on wikipedia page
        while(index != newRPN.size())
        {
            if(isdigit(newRPN[index][0]) || isdigit(newRPN[index][1]))
                stack.push_back(stoi(newRPN[index]));
            else
            {
                if(newRPN[index] == "~")
                {
                    int val = stack.back();
                    stack.pop_back();
                    stack.push_back(setCompliment(val));
                }
                else if (newRPN[index] == "*")
                {
                    int val1 = stack.back();
                    stack.pop_back();
                    int val2 = stack.back();
                    stack.pop_back();
                    stack.push_back(setIntersection(val1,val2));
                }
                else if (newRPN[index] == "+")
                {
                    int val1 = stack.back();
                    stack.pop_back();
                    int val2 = stack.back();
                    stack.pop_back();
                    stack.push_back(setUnion(val1,val2));
                }
                else if (newRPN[index] == "<")
                {
                    int val1 = stack.back();
                    stack.pop_back();
                    int val2 = stack.back();
                    stack.pop_back();
                    stack.push_back(isSubset(val1,val2));
                }
                else if (newRPN[index] == ">")
                {
                    int val1 = stack.back();
                    stack.pop_back();
                    int val2 = stack.back();
                    stack.pop_back();
                    stack.push_back(isSuperset(val1,val2));
                }
                else if (newRPN[index] == "=")
                {
                    int val1 = stack.back();
                    stack.pop_back();
                    int val2 = stack.back();
                    stack.pop_back();
                    stack.push_back(isEqual(val1,val2));
                }
            }
            index++;
        }


        return stack.back();

}


void process(int colorCode, string setMembers[])
{
    cout<<"{";
    for(int i = 0;i<16;++i)
        if((colorCode &(1<<i)))
            cout<<setMembers[i]<<", ";
    cout<<"\b\b}\n";
}

//for LIST
void listSets(int set[], string setMembers[])
{
    for(int i = 0;i<26;i++)
    {
        if(set[i])
        {
            cout<<(char)(i+65)<<" ";
            process(set[i],setMembers);
        }
    }
}

//for SHOW
void showSet(string &line, int set[], string setMembers[])
{
    stringstream ss;
    ss <<line;
    string firstWord, varToView;
    ss >> firstWord;
    ss >> varToView;
    process(set[(int)(varToView[0] - 65)],setMembers);
}

//for SAVE
void writeFile(string filename, int set[])
{
    ofstream myFile;
    myFile.open(filename.c_str());

    for(size_t i = 0;i<26;i++)
    {
        cout<<set[i]<<endl;
        myFile<<set[i]<<endl;
    }
    myFile.close();
}

//for LOAD
void readFile(string fileName, int set[])
{
    string line;
    ifstream myFile(fileName.c_str());
    int index = 0;
    if(myFile.fail())
    {
        cout<<"Error!"<<endl;
    }
    while(getline(myFile,line))
    {
        set[index] = stoi(line);
        index++;
    }

    myFile.close();
}

//for HELP
void help()
{
    cout<<"SET UNIVERSE IS: WHITE, GOLD, GREY, RED, GREEN, BLUE, ORANGE, PURPLE, YELLOW, VIOLET, MAGENTA, CYAN, RUST, NAVY, BURGUNDY, BLACK."<<endl;
    cout<<"UNION(+)"<<endl;
    cout<<"INTERSECTION(*)"<<endl;
    cout<<"COMPLIMENT(~)"<<endl;
    cout<<"SUBSET (<)"<<endl;
    cout<<"SUPERSET (>)"<<endl;
    cout<<"EQUALITY (=)"<<endl;
    cout<<"SET to create a set using numeric notation, or set notation, or variable"<<endl;
    cout<<"SHOW to see individual content of particular set"<<endl;
    cout<<"LIST to see contents of all sets"<<endl;
    cout<<"LOAD/SAVE 'filename' to read/write to that file"<<endl;
}




//BITWISE OPERATIONS
int setUnion(int x, int y)
{
    return x | y;
}

int setIntersection(int x, int y)
{
    return x & y;
}

int setCompliment(int x)
{
    return ~x;
}

int isSubset(int x, int y)
{
    if (x|y == y)
        return 65536;
    else
        return 65537;
}

int isSuperset(int x, int y)
{
    if(x|y == x && x!=y)
        return 65536;
    else
        return 65537;
}

int isEqual(int x, int y)
{
    if (x==y)
        return 65536;
    else
        return 65537;
}
