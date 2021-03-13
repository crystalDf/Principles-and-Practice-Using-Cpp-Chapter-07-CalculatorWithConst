# include "std_lib_facilities.h"

const char number = '8';
const char quit = 'q';
const char print = ';';
const string prompt = "> ";
const string result = "= ";

const char name = 'a';
const char let = 'L';
const string declkey = "let";

const char my_sqrt = 'S';
const string sqrtkey = "sqrt";

const char my_pow = 'P';
const string powkey = "pow";

const char my_const = 'C';
const string constkey = "const";

const string quitkey = "exit";

double expression();
double term();
double factorial();
double primary();
double get_factorial(double val);
void calculate();
void clean_up_mess();

double get_value(string s);
void set_value(string s, double d);
double statement();
double set_variable(bool is_const = false, bool is_declaration = true);
double define_name(string var, double val, bool is_const = false);
bool is_declared(string var);

double sqrt_pattern();
double pow_pattern();

class Token
{
public:
    char kind;
    double value;
    string name;

    Token(char ch) : kind { ch } { }
    Token(char ch, double val) : kind { ch }, value { val } { }
    Token(char ch, string n) : kind { ch }, name { n } { }
};

class Token_stream
{
public:
    Token_stream() : full { false }, buffer { ' ' } { }
    Token get();
    void putback(Token t);
    void ignore(char c);
private:
    bool full;
    Token buffer;
};

void Token_stream::putback(Token t)
{
    if (full)
    {
        error("putback() into a full buffer");
    }

    buffer = t;
    full = true;
}

Token Token_stream::get()
{
    if (full)
    {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch)
    {
        case print:
        case quit:
        case '(':
        case ')':
        case '{':
        case '}':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '!':
        case '=':
        case ',':
            return Token { ch };
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                std::cin.putback(ch);
                double val;
                std::cin >> val;
                return Token { number, val };
            }
        case '#':
            return Token { let };
        default:
            {
                if (isalpha(ch) || ('_' == ch))
                {
                    string s;

                    while (true)
                    {
                        s += ch;

                        if (!std::cin.get(ch) || (!isalnum(ch) && ('_' != ch)))
                        {
                            break;
                        }
                    }

                    std::cin.putback(ch);

                    if (s == declkey)
                    {
                        return Token { let };
                    }
                    if (s == sqrtkey)
                    {
                        return Token { my_sqrt };
                    }
                    if (s == powkey)
                    {
                        return Token { my_pow };
                    }
                    if (s == constkey)
                    {
                        return Token { my_const };
                    }
                    if (s == quitkey)
                    {
                        return Token { quit };
                    }

                    return Token { name, s };
                }

                error("Bad token");
                return Token { ' ' };
            }
    }
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind)
    {
        full = false;
        return;
    }

    full = false;

    char ch;
    while (std::cin >> ch)
    {
        if (ch == c)
        {
            return;
        }
    }
}

class Variable
{
public:
    Variable(string var, double val, bool var_is_const = false) : name { var }, value { val }, is_const { var_is_const } { }
    
    string name;
    double value;
    bool is_const;
};

Token_stream ts;
vector<Variable> var_table;

int main()
{
    try
    {
        define_name("pi", 3.1415926535);
        define_name("e", 2.7182818284);

        calculate();

        keep_window_open();
    }
    catch (exception& e)
    {
        std::cerr << e.what() << std::endl;
        keep_window_open();

        return 1;
    }
    catch (...)
    {
        std::cerr << "exception" << std::endl;
        keep_window_open();

        return 2;
    }
}

double expression()
{
    double left = term();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
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
                ts.putback(t);
                return left;
        }
    }
}

double term()
{
    double left = factorial();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
        {
            case '*':
                {
                    left *= factorial();
                    t = ts.get();
                    break;
                }
            case '/':
                {
                    double d = factorial();

                    if (!d)
                    {
                        error("divide by zero");
                    }

                    left /= d;
                    t = ts.get();
                    break;
                }
            case '%':
                {
                    double d = factorial();

                    if (!d)
                    {
                        error("divide by zero");
                    }

                    left = fmod(left, d);
                    t = ts.get();
                    break;
                }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double factorial()
{
    double left = primary();
    Token t = ts.get();

    while (true)
    {
        switch (t.kind)
        {
            case '!':
                {
                    left = get_factorial(left);
                    t = ts.get();
                    break;
                }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double primary()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case '(':
            {
                double d = expression();
                t = ts.get();

                if (t.kind != ')')
                {
                    error("')' expected");
                }
                return d;
            }
        case '{':
            {
                double d = expression();
                t = ts.get();

                if (t.kind != '}')
                {
                    error("'}' expected");
                }
                return d;
            }
        case number:
            return t.value;
        case '-':
            return -primary();
        case '+':
            return primary();
        case name:
            return get_value(t.name);
        default:
            error("primary expected");
            return 0;
    }
}

double get_factorial(double val)
{
    int result = 1;

    for (int i = 1; i <= val; ++i)
    {
        result *= i;
    }

    return result;
}

void calculate()
{
    while (std::cin)
    try 
    {
        std::cout << prompt;

        Token t = ts.get();

        while (t.kind == print)
        {
            t = ts.get();
        }

        if (t.kind == quit)
        {
            return;
        }
        
        ts.putback(t);
        std::cout << result << statement() << std::endl;
    }
    catch (exception& e)
    {
        std::cerr << e.what() << std::endl;
        clean_up_mess();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

double get_value(string s)
{
    for (const Variable& v : var_table)
    {
        if (v.name == s)
        {
            return v.value;
        }
    }

    error("get: undefined variable", s);
    return 0;
}

void set_value(string s, double d)
{
    for (Variable& v : var_table)
    {
        if (v.name == s)
        {
            if (!v.is_const)
            {
                v.value = d;
                return;
            }
            else
            {
                error(v.name, " is const");
            }
        }
    }

    error("set: undefined variable", s);
}

double statement()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case let:
            return set_variable();
        case my_const:
            return set_variable(true);
        case my_sqrt:
            return sqrt_pattern();
        case my_pow:
            return pow_pattern();
        case name:
            {
                char ch;
                std::cin >> ch;
                std::cin.putback(ch);
                
                if (ch == '=')
                {
                    ts.putback(t);
                    return set_variable(false, false);
                }
            }
        default:
            ts.putback(t);
            return expression();
    }
}

double set_variable(bool is_const, bool is_declaration)
{
    Token t = ts.get();

    if (t.kind != name)
    {
        error("name expected in declaration/assignment");
    }

    string var_name = t.name;

    Token t2 = ts.get();

    if (t2.kind != '=')
    {
        error("= missing in declaration/assignment of ", var_name);
    }

    double d = expression();

    if (is_declaration)
    {
        define_name(var_name, d, is_const);
    }
    else
    {
        set_value(var_name, d);
    }

    return d;
}

double define_name(string var, double val, bool is_const)
{
    if (is_declared(var))
    {
        error(var, " declared twice");
    }

    var_table.push_back(Variable(var, val, is_const));

    return val;
}

bool is_declared(string var)
{
    for (const Variable& v : var_table)
    {
        if (v.name == var)
        {
            return true;
        }
    }

    return false;
}

double sqrt_pattern()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case '(':
            {
                double d = expression();
                t = ts.get();

                if (t.kind != ')')
                {
                    error("')' expected");
                }

                if (d < 0)
                {
                    error("sqrt by negative");
                }

                return sqrt(d);
            }
        default:
            error("sqrt_pattern expected");
            return 0;
    }
}

double pow_pattern()
{
    Token t = ts.get();

    switch (t.kind)
    {
        case '(':
            {
                double x = expression();
                t = ts.get();

                if (t.kind != ',')
                {
                    error("',' expected");
                }

                double i = expression();
                t = ts.get();

                if (t.kind != ')')
                {
                    error("')' expected");
                }
                return pow(x, i);
            }
        default:
            error("pow_pattern expected");
            return 0;
    }
}
