#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;


struct Num {
    bool neg = false;
    string digits;      // all digits, decimal point removed
    size_t fracLen = 0; 
};

bool isValidString(const string& s) {
    size_t i = 0;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) i++;

    size_t intDigits = 0;
    while (i < s.size() && isdigit(static_cast<unsigned char>(s[i]))) { i++; intDigits++; }
    if (intDigits == 0) return false;          // rejects ".5", "A", "+-1"
    if (i == s.size()) return true;            

    if (s[i] != '.') return false;
    i++;

    size_t fracDigits = 0;
    while (i < s.size() && isdigit(static_cast<unsigned char>(s[i]))) { i++; fracDigits++; }
    return fracDigits > 0 && i == s.size();    // rejects "-5.", "2.3.4", "-5.-5"
}

Num parseNum(const string& s) {
    Num n;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') { n.neg = (s[0] == '-'); i = 1; }
    bool afterDot = false;
    for (; i < s.size(); i++) {
        if (s[i] == '.') afterDot = true;
        else {
            n.digits += s[i];
            if (afterDot) n.fracLen++;
        }
    }
    return n;
}

string stripZeros(const string& s) {
    size_t p = s.find_first_not_of('0');
    return (p == string::npos) ? "0" : s.substr(p);
}

void padLeftEqual(string& a, string& b) {
    while (a.size() < b.size()) a.insert(a.begin(), '0');
    while (b.size() < a.size()) b.insert(b.begin(), '0');
}

// returns -1, 0, 1
char compareMag(string a, string b) {
    a = stripZeros(a); b = stripZeros(b);
    padLeftEqual(a, b);
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

string addMag(string a, string b) {
    padLeftEqual(a, b);
    string r;
    char carry = 0;
    for (size_t i = a.size(); i-- > 0; ) {
        char sum = (a[i] - '0') + (b[i] - '0') + carry;   // char arithmetic only
        if (sum >= 10) { sum -= 10; carry = 1; } else carry = 0;
        r += static_cast<char>('0' + sum);
    }
    if (carry) r += '1';
    reverse(r.begin(), r.end());
    return r;
}

// precondition: a >= b in magnitude
string subMag(string a, string b) {
    padLeftEqual(a, b);
    string r;
    char borrow = 0;
    for (size_t i = a.size(); i-- > 0; ) {
        char d = (a[i] - '0') - (b[i] - '0') - borrow;
        if (d < 0) { d += 10; borrow = 1; } else borrow = 0;
        r += static_cast<char>('0' + d);
    }
    reverse(r.begin(), r.end());
    return stripZeros(r);
}

// Multiply by repeated addition: for each digit of b, add (a shifted) that many times.
string mulMag(const string& a, const string& b) {
    string result = "0";
    string shifted = a;                       // a * 10^position
    for (size_t j = b.size(); j-- > 0; ) {
        for (char k = '0'; k < b[j]; ++k)
            result = addMag(result, shifted);
        shifted += '0';
    }
    return stripZeros(result);
}

// Signing operators 
void alignDecimals(Num& a, Num& b) {
    while (a.fracLen < b.fracLen) { a.digits += '0'; a.fracLen++; }
    while (b.fracLen < a.fracLen) { b.digits += '0'; b.fracLen++; }
}

string formatNum(Num n) {
    string d = stripZeros(n.digits);
    while (d.size() <= n.fracLen) d.insert(d.begin(), '0');   // ensure an integer part exists

    string intPart  = d.substr(0, d.size() - n.fracLen);
    string fracPart = d.substr(d.size() - n.fracLen);
    // trim trailing zeros in fraction
    size_t last = fracPart.find_last_not_of('0');
    fracPart = (last == string::npos) ? "" : fracPart.substr(0, last + 1);

    string out = intPart;
    if (!fracPart.empty()) out += "." + fracPart;

    bool isZero = (out.find_first_not_of("0.") == string::npos);
    return (n.neg && !isZero ? "-" : "") + out;
}
//Runs all the operators by doing addition of the strings
string addNums(Num a, Num b) {
    alignDecimals(a, b);
    Num r;
    r.fracLen = a.fracLen;
    if (a.neg == b.neg) {
        r.digits = addMag(a.digits, b.digits);
        r.neg = a.neg;
    } else {
        char c = compareMag(a.digits, b.digits);
        if (c == 0) { r.digits = "0"; r.neg = false; }
        else if (c > 0) { r.digits = subMag(a.digits, b.digits); r.neg = a.neg; }
        else            { r.digits = subMag(b.digits, a.digits); r.neg = b.neg; }
    }
    return formatNum(r);
}
//runs addition as normal
string addStrings(const string& n1, const string& n2) {
    return addNums(parseNum(n1), parseNum(n2));
}
//Subtraction is addition with flipped operator
string subtractStrings(const string& n1, const string& n2) {
    Num b = parseNum(n2);
    b.neg = !b.neg;
    return addNums(parseNum(n1), b);
}
//repeated addition is multiplication
string multiplyStrings(const string& n1, const string& n2) {
    Num a = parseNum(n1), b = parseNum(n2), r;
    r.digits = mulMag(a.digits, b.digits);
    r.fracLen = a.fracLen + b.fracLen;
    r.neg = (a.neg != b.neg);
    return formatNum(r);
}

//Reads the operators
string compute(const string& a, const string& b, const string& op, bool& ok) {
    ok = true;
    if (op == "+") return addStrings(a, b);
    if (op == "-") return subtractStrings(a, b);
    if (op == "*") return multiplyStrings(a, b);
    ok = false;
    return "";
}

//File I/O managment
void processFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    int lineCount = 1;   // display only, not used in any arithmetic on the values

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string number1, number2, extra;

        if (!(ss >> number1 >> number2) || (ss >> extra)) {
            cout << "line " << lineCount << ": ERROR: expected exactly two numbers\n";
        } else {
            bool okA = isValidString(number1), okB = isValidString(number2);
            if (!okA || !okB)
                cout << "line " << lineCount << ": ERROR: invalid number"
                     << (okA ? "" : " \"" + number1 + "\"")
                     << (okB ? "" : " \"" + number2 + "\"") << "\n";
            else
                cout << "line " << lineCount << ": " << number1 << " + " << number2
                     << " = " << addStrings(number1, number2) << "\n";
        }
        lineCount++;
    }
}

int main(int argc, char* argv[]) {
    // Use a filename from the command line if given, otherwise the default
    string filename = (argc > 1) ? argv[1] : "tests.txt";

    processFile(filename);

    cout << "\nPress Enter to exit...";
    cin.get();
    return 0;
}