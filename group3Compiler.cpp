#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <set>

using namespace std;

class Token
{
private:
	string TYPE, VALUE;

public:
	Token(string type_, string value_)
	{
		TYPE = type_;
		VALUE = value_;
	}
	string GetType() { return TYPE; }
	string GetValue() { return VALUE; }
};

class Lexer
{
private:
	vector<Token> TOKEN_LIST;			// for displaying all the tokens, also used as backup
	queue<Token> TOKEN_QUEUE;			// for error checking
	queue<Token> COMPILING_TOKEN_QUEUE; // for compiling the code if there are no errors
	string code_string;					// storing code extracted from the file into a string

public:
	void AddToken(string type_, string *value_)
	{
		if (*value_ != "")
		{
			Token t = Token(type_, *value_);
			TOKEN_LIST.push_back(t);
			COMPILING_TOKEN_QUEUE.push(t);
			TOKEN_QUEUE.push(t);
			*value_ = "";
		}
	}

	queue<Token> GetTokenQueue() { return COMPILING_TOKEN_QUEUE; }
	vector<Token> GetTokenVector() { return TOKEN_LIST; }

	void ShowTokens()
	{ // print out the tookens extracted
		for (int i = 0; i < TOKEN_LIST.size(); i++)
			cout << TOKEN_LIST.at(i).GetValue() << "\t" << TOKEN_LIST.at(i).GetType() << '\n';
	}

	bool GetCodeFromFile(string file_directory)
	{ // returns true if there were errors reading from file else it returns false
		string line;
		ifstream file_to_compile;

		// imports file from computer
		file_to_compile.open(file_directory);

		if (file_to_compile.is_open())
		{
			// as long as there is something in the file.
			while (!file_to_compile.eof())
			{
				// get a line from the file and put it into "line"
				getline(file_to_compile, line);
				// add the line to string "file"
				code_string += line;
				// add a new line between each line
				code_string.push_back('\n');
			}
			// close file
			file_to_compile.close();
			return false;
		}
		else
		{
			cout << "Failed to read from file" << '\n';
			cout << "Check if you entered the correct file path." << '\n';
			return true;
		}
	}

	void GetTokensFromCode(string file_directory)
	{
		string token_value = "";
		string token_type = "";

		// THIS LOOP CHECKS EACH CHARACTER IN THE GIVEN LINE.
		// 0-9		: INTEGER LITERAL
		// A-C		: IDENTIFIER
		// +,-,*,/ 	: EXPRESSION OPERATORS
		// ;		: END OF STATEMENT
		if (!GetCodeFromFile(file_directory))
		{
			for (int i = 0; i < code_string.length(); i++)
			{
				// loop through the code character by character
				char character = code_string.at(i);
				if (isdigit(character))
				{
					// if current character is a number
					while (isdigit(character))
					{
						// add it and every following number to the end of it to complete the full number
						token_value += character;
						i++;
						if (i < code_string.length())
							character = code_string.at(i);
						else
							break;
					}

					this->AddToken("INTEGER", &token_value);
				}
				// if current character is an alphabet
				if (isalpha(character))
				{
					// add it and every following alphabet to the end of it to complete the full word
					while (isalpha(character))
					{
						token_value += character;
						i++;
						if (i < code_string.length())
							character = code_string.at(i);
						else
							break;
					}
					if (token_value == "begin") // if the resulting word is display, then it is our display keyword
						this->AddToken("PROGRAM_START", &token_value);
					else if (token_value == "end") // if the resulting word is display, then it is our display keyword
						this->AddToken("PROGRAM_END", &token_value);
					if (token_value == "input") // if the resulting word is input, then it is our input keyword
						this->AddToken("Input Call", &token_value);
					else if (token_value == "display") // if the resulting word is display, then it is our display keyword
						this->AddToken("Display Call", &token_value);
					else // else the resulting word is just an identifier
						this->AddToken("IDENTIFIER", &token_value);
				}
				if (character == ';')
				{
					token_value = character;
					this->AddToken("SEMICOLON", &token_value);
				}
				if (character == '+')
				{
					token_value = character;
					this->AddToken("ADDITION_OPERATOR", &token_value);
				}
				if (character == '-')
				{
					token_value = character;
					this->AddToken("SUBTRACTION_OPERATOR", &token_value);
				}
				if (character == '*')
				{
					token_value = character;
					this->AddToken("MULTIPLICATION_OPERATOR", &token_value);
				}
				if (character == '=')
				{
					token_value = character;
					this->AddToken("ASSIGNMENT_OPERATOR", &token_value);
				}
				if (character == '/')
				{
					token_value = character;
					this->AddToken("DIVISION_OPERATOR", &token_value);
				}
				if (character == '"')
				{
					// Note: if a string is missing a closing parenthesis, loop until the end of expression or new line and notify the user
					do
					{
						token_value += character;
						i++;
						character = code_string.at(i);
					} while (character != '"' && character != ';' && character != '\n');
					if (character == ';')
					{
						cout << "Missing (\") at the end of string" << '\n';
						// add the end end quote to avoid any other errors
						token_value += '\"';
					}
					token_value += character;
					this->AddToken("STRING", &token_value);
				}
			}
		}
	}

	bool LexicalErrors()
	{ // returns true if an error is found, false otherwise
		if (TOKEN_QUEUE.empty())
		{
			return true;
		}
		bool found_error = false;

		// these flags check if the variable was declared before being assigned to another variable or added to another variable in a statement list
		bool was_A_assigned, was_B_assigned, was_C_assigned = false;

		string variable_to_flag, current_token;

		// all lines in the code should begin with { begin || end || input || display || A || B || C }
		// output an error if the above is not true

		// 'begin' not included since it's checked for and removed at the start of the iteration
		set<string> tkns = {"end", "display", "input", "A", "B", "C"};
		set<string> operators = {"+", "-", "*", "/"};
		set<string> variables = {"A", "B", "C"};

		// if the front of the queue is not begin
		if (TOKEN_QUEUE.front().GetValue() != "begin")
		{
			cout << "SyntaxError: Missing 'begin' statement at the start of program. " << '\n';
			found_error = true;
		}
		// if the token is 'begin' pop it from front of the queue
		else
			TOKEN_QUEUE.pop();

		// iterate through all the tokens and check for valid statements
		// if a statement is invalid, pop every token in the queue until you reach the end of statement and repeat.
		while (!TOKEN_QUEUE.empty())
		{
			// temporary string to hold the front token: this is also the start of each line in the code
			string token = TOKEN_QUEUE.front().GetValue();
			string next_token, next_token_type;

			// if token does not begin with a key word (in set tkns)
			if (tkns.find(token) == tkns.end())
			{
				found_error = true;
				cout << "Syntax Error: " << TOKEN_QUEUE.front().GetValue() << " is not a valid " << TOKEN_QUEUE.front().GetType() << '\n';
				cout << "Solution: each line of code  must begin with {display, input, end, A, B, C} key words" << '\n';
				// delete all the tokens in the entire line of code from the queue since it if faulty
				while (TOKEN_QUEUE.front().GetValue() != ";")
					TOKEN_QUEUE.pop();
				// remove the end of statement
				TOKEN_QUEUE.pop();
			}

			// if the token is one of the variables
			if (variables.find(token) != variables.end())
			{
				// assign token to the variable_to_flag: mark it as true if the assignment is successful
				// variables are only flagged as true if they are assigned to integers or other variables which have already been assigned to integers
				variable_to_flag = token;

				// pop the token from the queue
				TOKEN_QUEUE.pop();
				next_token = TOKEN_QUEUE.front().GetValue();

				// after a variable is declared, the next token should be an equal sign, if not, there is an error
				if (next_token != "=")
				{
					found_error = true;
					cout << "Missing an assignment operator after variable declaration" << '\n';
					// delete all the tokens in the entire line of code from the queue since it if faulty
					while (TOKEN_QUEUE.front().GetValue() != ";")
						TOKEN_QUEUE.pop();
					// remove the end of statement
					TOKEN_QUEUE.pop();
				}

				// validate the last part of the statement and indicate the variable as used
				else
				{
					// pop out the assignment operator
					TOKEN_QUEUE.pop();
					next_token = TOKEN_QUEUE.front().GetValue();
					next_token_type = TOKEN_QUEUE.front().GetType();
					// if next token is not another variable, or an integer, return an error since variables can only be assigned to other variables
					if (variables.find(next_token) == variables.end() && next_token_type != "INTEGER")
					{
						found_error = true;
						cout << next_token << " cannot be assigned to a variable" << '\n';
						// delete all the tokens in the entire line of code from the queue since it if faulty
						while (TOKEN_QUEUE.front().GetValue() != ";")
							TOKEN_QUEUE.pop();
						// remove the end of statement
						TOKEN_QUEUE.pop();
					}
					// if there is a correct assignment, indicate the variable as used
					else
					{
						if (next_token == "A" && was_A_assigned == false)
						{
							found_error = true;
							cout << "Unassigned variable A used" << '\n';
						}
						else if (next_token == "B" && was_B_assigned == false)
						{
							found_error = true;
							cout << "Unassigned variable B used" << '\n';
						}
						else if (next_token == "C" && was_C_assigned == false)
						{
							found_error = true;
							cout << "Unassigned variable C used" << '\n';
						}
						// remove the variable or integer from the queue
						TOKEN_QUEUE.pop();
						next_token = TOKEN_QUEUE.front().GetValue();

						// if the next token is not an end of statement or an assignment operator
						if (next_token != ";" && operators.find(next_token) == operators.end())
						{
							found_error = true;
							cout << "Syntax error: Wrong expression used" << '\n';
							cout << "Expressions can only be <var> + <var> | <var>-<var> | <var>*<var> | <var>/<var> | <var>" << '\n';
							while (next_token != ";")
							{
								TOKEN_QUEUE.pop();
								next_token = TOKEN_QUEUE.front().GetValue();
							}
							// pop out end of statement
							TOKEN_QUEUE.pop();
						}

						// if the next token is an operator
						else if (operators.find(next_token) != operators.end())
						{
							// remove the operator from the queue
							TOKEN_QUEUE.pop();
							next_token = TOKEN_QUEUE.front().GetValue();
							// if the variables are not in the accepted variables set, then there is an error
							if (variables.find(next_token) == variables.end())
							{
								found_error = true;
								cout << next_token << "cannot be assigned to a variable" << '\n';
								// delete all the tokens in the entire line of code from the queue since it if faulty
								while (TOKEN_QUEUE.front().GetValue() != ";")
									TOKEN_QUEUE.pop();
								// remove the end of statement
								TOKEN_QUEUE.pop();
							}
							// if the variable is valid, check if it was assigned before
							else
							{
								// check if A was assigned
								if (next_token == "A")
								{
									if (was_A_assigned == false)
									{
										found_error = true;
										cout << "Unassigned variable A used" << '\n';
									}
									// remove token from queue
									TOKEN_QUEUE.pop();
								}
								// check if B was assigned
								else if (next_token == "B")
								{
									if (was_B_assigned == false)
									{
										found_error = true;
										cout << "Unassigned variable B used" << '\n';
									}
									// remove token from queue
									TOKEN_QUEUE.pop();
								}
								// check if C was assigned
								else if (next_token == "C")
								{
									if (was_C_assigned = false)
									{
										found_error = true;
										cout << "Unassigned variable C used" << '\n';
									}
									// remove token from queue
									TOKEN_QUEUE.pop();
								}

								// next token should be a end of statement operator
								if (TOKEN_QUEUE.front().GetValue() != ";")
								{
									found_error = true;
									cout << "Syntax Error: missing end of statement operator ;" << '\n';
									// delete all the tokens in the entire line of code from the queue since it if faulty
									while (TOKEN_QUEUE.front().GetValue() != ";")
										TOKEN_QUEUE.pop();
									// remove the end of statement
									TOKEN_QUEUE.pop();
								}
								else
								{
									if (variable_to_flag == "A")
										was_A_assigned = true;
									else if (variable_to_flag == "B")
										was_B_assigned = true;
									else if (variable_to_flag == "C")
										was_C_assigned = true;
								}
							}
							TOKEN_QUEUE.pop();
							next_token = TOKEN_QUEUE.front().GetValue();
						}

						// now if the next token is an end of assignment, mark the variable as used
						else
						{
							if (variable_to_flag == "A")
								was_A_assigned = true;
							else if (variable_to_flag == "B")
								was_B_assigned == true;
							else if (variable_to_flag == "C")
								was_C_assigned = true;
							// remove the end of statement from the queue
							TOKEN_QUEUE.pop();
						}
					}
				}
			}
			// display can only display a string or a variable
			else if (token == "input" || token == "display")
			{
				current_token = token;
				// pop token from the queue
				TOKEN_QUEUE.pop();
				next_token = TOKEN_QUEUE.front().GetValue();
				next_token_type = TOKEN_QUEUE.front().GetType();
				// after display or input, the next token should be A | B | C or a string (only for display)
				if (variables.find(next_token) == variables.end() && next_token_type != "STRING")
				{
					found_error = true;
					if (current_token == "input")
						cout << "Invalid Syntax: Input can only be A | B | C" << '\n';
					else if (current_token == "display")
						cout << "Invalid Syntax: Only A | B | C | 'sample string' can be displayed" << '\n';

					// delete all the tokens in the entire line of code from the queue since it if faulty
					while (TOKEN_QUEUE.front().GetValue() != ";")
						TOKEN_QUEUE.pop();
					// remove the end of statement
					TOKEN_QUEUE.pop();
				}
				// if the next token is a variable
				else if (variables.find(next_token) != variables.end())
				{
					if (next_token == "A")
					{
						// only assigned variables can be displayed
						if (current_token == "display" && was_A_assigned == false)
						{
							found_error = true;
							cout << "The variable A was not assigned" << '\n';
						}
						else if (current_token == "input")
							was_A_assigned = true;

						// remove the variable from the queue
						TOKEN_QUEUE.pop();
					}
					if (next_token == "B")
					{
						// only assigned variables can be displayed
						if (current_token == "display" && was_B_assigned == false)
						{
							found_error = true;
							cout << "The variable B was not assigned" << '\n';
						}
						else if (current_token == "input")
							was_B_assigned = true;

						// remove the variable from the queue
						TOKEN_QUEUE.pop();
					}
					if (next_token == "C")
					{
						// only assigned variables can be displayed
						if (current_token == "display" && was_C_assigned == false)
						{
							found_error = true;
							cout << "The variable C was not assigned" << '\n';
						}
						else if (current_token == "input")
							was_C_assigned = true;

						// remove the variable from the queue
						TOKEN_QUEUE.pop();
					}

					if (TOKEN_QUEUE.front().GetValue() != ";")
					{
						found_error = true;
						cout << "Syntax Error: Missing end of statement" << '\n';
					}
					// remove the end of statement token ; from the queue
					else
						TOKEN_QUEUE.pop();
				}
				// if the next token is a string
				else if (next_token_type == "STRING")
				{
					// pop string token from queue
					TOKEN_QUEUE.pop();

					if (current_token == "display")
					{
						if (TOKEN_QUEUE.front().GetValue() != ";")
						{
							found_error = true;
							cout << "end of statement is missing" << '\n';
						}
						else
							TOKEN_QUEUE.pop();
					}
					else if (current_token == "input")
					{
						// pop string token from queue
						found_error = true;
						cout << "input can only be A | B | C" << '\n';
						// delete all the tokens in the entire line of code from the queue since it if faulty
						while (TOKEN_QUEUE.front().GetValue() != ";")
							TOKEN_QUEUE.pop();
						// remove the end of statement
						TOKEN_QUEUE.pop();
					}
				}
			}

			// break out of the loop if we reach the end of program
			else if (token == "end")
			{
				// break from the loop as this signals the end of program
				TOKEN_QUEUE.pop();
				break;
			}
		}
		// if the end of the queue is not end
		if (TOKEN_QUEUE.back().GetValue() != "end")
		{
			cout << "SyntaxError: Missing 'end' statement at the end of program. " << '\n';
			found_error = true;
		}
		return found_error;
	}
};

void CompileCode(string file_directory);

int main()
{
	// Get file directory from user
	string file_directory;
	cout << "Enter file path: ";
	cin >> file_directory;

	// Compile the code
	CompileCode(file_directory);
	return 0;
}

void CompileCode(string file_directory)
{ // this function only executes if and only if the code has been checked for errors and none were found
	// initialize lex objext
	Lexer lex;
	lex.GetTokensFromCode(file_directory); // get all the tokens

	// execute if and only if no errors were found in the code
	if (!lex.LexicalErrors())
	{
		queue<Token> token_queue = lex.GetTokenQueue(); // get the queue of tokens
		int A = 0, B = 0, C = 0;						// variables initialized with value of 0

		while (!token_queue.empty())
		{
			if (token_queue.front().GetValue() == "A")
			{					   // assigning a value to A
				token_queue.pop(); // pop the A
				token_queue.pop(); // pop the equal sign (=)

				string value = token_queue.front().GetValue();
				token_queue.pop();

				if (token_queue.front().GetValue() == ";")
				{
					// if value to be assigned is a variable or an integer
					if (value == "B")
						A = B;
					else if (value == "C")
						A = C;
					else
						A = stoi(value);
				}
				else
				{
					// if assigning an expression, evaluate the expression then assign it to the variable
					int result;
					int value_2;

					if (value == "A")
						value_2 = A;
					else if (value == "B")
						value_2 = B;
					else if (value == "C")
						value_2 = C;

					if (token_queue.front().GetValue() == "+")
					{
						token_queue.pop();

						if (token_queue.front().GetValue() == "A")
						{
							A = value_2 + A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							A = value_2 + B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							A = value_2 + C;
						}
						else
						{
							A = value_2 + stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "-")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							A = value_2 - A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							A = value_2 - B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							A = value_2 - C;
						}
						else
						{
							A = value_2 - stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "*")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							A = value_2 * A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							A = value_2 * B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							A = value_2 * C;
						}
						else
						{
							A = value_2 * stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "/")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							A = value_2 / A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							A = value_2 / B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							A = value_2 / C;
						}
						else
						{
							A = value_2 / stoi(token_queue.front().GetValue());
						}
					}
				}
			}
			else if (token_queue.front().GetValue() == "B")
			{					   // ASSIGNING VALUE TO B
				token_queue.pop(); // pop the B
				token_queue.pop(); // pop the equal sign (=)

				string value = token_queue.front().GetValue();
				token_queue.pop();

				if (token_queue.front().GetValue() == ";")
				{
					// if value to be assigned is a variable or an integer
					if (value == "A")
					{
						B = A;
					}
					else if (value == "C")
					{
						B = C;
					}
					else
					{
						B = stoi(value);
					}
				}
				else
				{
					// if assigning an expression, evaluate the expression then assign it to the variable
					int result;
					int value_2;

					if (value == "A")
						value_2 = A;
					else if (value == "B")
						value_2 = B;
					else if (value == "C")
						value_2 = C;

					if (token_queue.front().GetValue() == "+")
					{
						token_queue.pop();

						if (token_queue.front().GetValue() == "A")
						{
							B = value_2 + A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							B = value_2 + B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							B = value_2 + C;
						}
						else
						{
							B = value_2 + stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "-")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							B = value_2 - A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							B = value_2 - B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							B = value_2 - C;
						}
						else
						{
							B = value_2 - stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "*")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							B = value_2 * A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							B = value_2 * B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							B = value_2 * C;
						}
						else
						{
							B = value_2 * stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "/")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							B = value_2 / A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							B = value_2 / B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							B = value_2 / C;
						}
						else
						{
							B = value_2 / stoi(token_queue.front().GetValue());
						}
					}
				}
			}
			else if (token_queue.front().GetValue() == "C")
			{					   // ASSIGNING VALUE TO C
				token_queue.pop(); // pop the C
				token_queue.pop(); // pop the equal sign (=)
				string value = token_queue.front().GetValue();
				token_queue.pop();

				if (token_queue.front().GetValue() == ";")
				{
					// if value to be assigned is a variable or an integer
					if (value == "A")
					{
						C = A;
					}
					else if (value == "B")
					{
						C = B;
					}
					else
					{
						C = stoi(value);
					}
				}
				else
				{
					// if assigning an expression, evaluate the expression then assign it to the variable
					int result;
					int value_2;

					if (value == "A")
						value_2 = A;
					else if (value == "B")
						value_2 = B;
					else if (value == "C")
						value_2 = C;

					if (token_queue.front().GetValue() == "+")
					{
						token_queue.pop();

						if (token_queue.front().GetValue() == "A")
						{
							C = value_2 + A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							C = value_2 + B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							C = value_2 + C;
						}
						else
						{
							C = value_2 + stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "-")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							C = value_2 - A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							C = value_2 - B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							C = value_2 - C;
						}
						else
						{
							C = value_2 - stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "*")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							C = value_2 * A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							C = value_2 * B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							C = value_2 * C;
						}
						else
						{
							C = value_2 * stoi(token_queue.front().GetValue());
						}
					}
					else if (token_queue.front().GetValue() == "/")
					{
						token_queue.pop();
						if (token_queue.front().GetValue() == "A")
						{
							C = value_2 / A;
						}
						else if (token_queue.front().GetValue() == "B")
						{
							C = value_2 / B;
						}
						else if (token_queue.front().GetValue() == "C")
						{
							C = value_2 / C;
						}
						else
						{
							C = value_2 / stoi(token_queue.front().GetValue());
						}
					}
				}
			}
			else if (token_queue.front().GetValue() == "input")
			{ // pop input from queue and assign a value to the variable selected
				token_queue.pop();
				if (token_queue.front().GetValue() == "A")
				{
					cout << "Enter a value for A: ";
					cin >> A;
				}
				else if (token_queue.front().GetValue() == "B")
				{
					cout << "Enter a value for B: ";
					cin >> B;
				}
				else if (token_queue.front().GetValue() == "C")
				{
					cout << "Enter a value for C: ";
					cin >> C;
				}
			}
			else if (token_queue.front().GetValue() == "display")
			{ // pop display from queue and print the next token
				token_queue.pop();
				if (token_queue.front().GetValue() == "A")
					cout << A << '\n';
				else if (token_queue.front().GetValue() == "B")
					cout << B << '\n';
				else if (token_queue.front().GetValue() == "C")
					cout << C << '\n';
				else if (token_queue.front().GetType() == "STRING")
					cout << token_queue.front().GetValue() << '\n';
			}
			token_queue.pop(); // advance to the next token
		}
		// CompileCode(token_queue);
	}
	else
		cout << "\nSolution: Fix the identified errors and compile again" << '\n';
}

// Compiling c++ in Visual Studio Code
// g++ group3Compiler.cpp && ./a.out