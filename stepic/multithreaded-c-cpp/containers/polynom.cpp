#include <iostream>

#include <map>
#include <sstream>
#include <algorithm>

std::map<int, int> powerToCoef;

void add_term_to_map(std::string term) {
	//std::cout << "Analysing term: " << term << std::endl;
	size_t power_pos = term.find('^');
	size_t symbol_pos = term.find('x');

	int power = 0;
	if (power_pos != std::string::npos) { // '^' found, get the power
		power = std::stoi(term.substr(power_pos + 1));
	} else if (symbol_pos != std::string::npos) { //symbol present, but whithout '^'
	    power = 1;
	}
	
	int coef = 1;
	if (symbol_pos == std::string::npos) { // no symbol, the whole term is coef
		coef = std::stoi(term);
	} else if (symbol_pos == 0) { // just x
		coef = 1;
	} else if (term.find("+x") == 0) {
		coef = 1;
	} else if (term.find("-x") == 0) {
		coef = -1;
	} else if (symbol_pos > 1) {
		coef = std::stoi(term.substr(0, symbol_pos - 1)); // -1 to skip the '*'
	}

	coef *= power;
	power--;

	if (power < 0) {
		return; // it was a constant
	}
	
	if (powerToCoef.find(power) != powerToCoef.end()) {
		powerToCoef[power] = powerToCoef[power] + coef;
	} else {
		powerToCoef[power] = coef;
	}
}

size_t find_next_sign_index(const std::string& polynomial) {
	size_t next_plus = polynomial.find('+');
	if (next_plus == 0) {
		next_plus = polynomial.find('+', 1);
	}

	size_t next_minus = polynomial.find('-');
	if (next_minus == 0) {
		next_minus = polynomial.find('-', 1);
	}

	if (next_plus == std::string::npos && next_minus == std::string::npos) {
		return std::string::npos;
	} else if (next_plus == std::string::npos) {
		return next_minus;
	} else if (next_minus == std::string::npos) {
		return next_plus;
	} else {
		return std::min(next_plus, next_minus);
	}
}

std::string collect_derivative() {
	std::stringstream derivative;
	for (auto riter = powerToCoef.rbegin(); riter != powerToCoef.rend(); ++riter) {
		int coef = riter->second;
		int power = riter->first;
		if (coef != 0) {
			if (power == 0) {
				if (coef > 0) {
					derivative << '+' << coef;
                } else {
                	derivative << coef;
                }
			} else {
				if (coef > 1) {
					derivative << '+' << coef << "*x";
                } else if (coef < -1) {
                	derivative << coef << "*x";
                } else if (coef == 1) {
                	derivative << "+x";
                } else if (coef == -1) {
                	derivative << "-x";
                }
                if (power > 1) {
                	derivative << '^' << power;
                }
			}
		}
	}
	std::string result = derivative.str();
	if (result.find('+') == 0) {
		result.erase(0, 1);
	}
	return result;
}

std::string derivative(std::string polynomial) {
	size_t next_sign_index = find_next_sign_index(polynomial);
	while (next_sign_index != std::string::npos) {
		std::string next_term = polynomial.substr(0, next_sign_index);
		add_term_to_map(next_term);
		polynomial.erase(0, next_sign_index);
		next_sign_index = find_next_sign_index(polynomial);
	}
	add_term_to_map(polynomial);
    return collect_derivative();
}

int main(int argc, char **argv) {
	std::string str;
	std::cin >> str;
	std::cout << "Derivative: " << derivative(str) << std::endl;
	return 0;
}

