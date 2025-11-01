#include "chrtoken.h"

namespace chr {
	token_t string_to_operator_token_type(const std::string& operator_str)
	{
		if (operator_str == "(") {
			return token_left_parentheses;
		}
		else if (operator_str == ")") {
			return token_right_parentheses;
		}
		else if (operator_str == "+") {
			return token_plus;
		}
		else if (operator_str == "-") {
			return token_minus;
		}
		else if (operator_str == "*") {
			return token_multiply;
		}
		else if (operator_str == "/") {
			return token_divide;
		}
		else if (operator_str == "pos") {
			return token_posite;
		}
		else if (operator_str == "neg") {
			return token_negate;
		}
		else if (operator_str == "^") {
			return token_exponent;
		}
		else if (operator_str == "sin") {
			return token_sine;
		}
		else if (operator_str == "cos") {
			return token_cosine;
		}
		else if (operator_str == "tan") {
			return token_tangent;
		}
		else if (operator_str == "cot") {
			return token_cotangent;
		}
		else if (operator_str == "sec") {
			return token_secant;
		}
		else if (operator_str == "csc") {
			return token_cosecant;
		}
		else if (operator_str == "arcsin") {
			return token_arcsine;
		}
		else if (operator_str == "arccos") {
			return token_arccosine;
		}
		else if (operator_str == "arctan") {
			return token_arctangent;
		}
		else if (operator_str == "arccot") {
			return token_arccotangent;
		}
		else if (operator_str == "arcsec") {
			return token_arcsecant;
		}
		else if (operator_str == "arccsc") {
			return token_arccosecant;
		}
		else if (operator_str == "lg") {
			return token_common_logarithm;
		}
		else if (operator_str == "ln") {
			return token_natural_logarithm;
		}
		else if (operator_str == "sqrt") {
			return token_square_root;
		}
		else if (operator_str == "cbrt") {
			return token_cubic_root;
		}
		else if (operator_str == "!") {
			return token_factorial;
		}
		else if (operator_str == "%") {
			return token_modulo;
		}
		else if (operator_str == "deg") {
			return token_degree;
		}
		else if (operator_str == "rad") {
			return token_radian;
		}
		else {
			throw std::runtime_error("位置的运算符令牌");
		}
	}

	operator_token* token_type_to_operator_token(token_t operator_type)
	{
		switch (operator_type) {
		case token_left_parentheses:
			return new left_parentheses_token;
		case token_right_parentheses:
			return new right_parentheses_token;
		case token_plus:
			return new plus_token;
		case token_minus:
			return new minus_token;
		case token_multiply:
			return new multiply_token;
		case token_divide:
			return new divide_token;
		case token_posite:
			return new posite_token;
		case token_negate:
			return new negate_token;
		case token_exponent:
			return new exponent_token;
		case token_sine:
			return new sine_token;
		case token_cosine:
			return new cosine_token;
		case token_tangent:
			return new tangent_token;
		case token_cotangent:
			return new cotangent_token;
		case token_secant:
			return new secant_token;
		case token_cosecant:
			return new cosecant_token;
		case token_arcsine:
			return new arcsine_token;
		case token_arccosine:
			return new arccosine_token;
		case token_arctangent:
			return new arctangent_token;
		case token_arccotangent:
			return new arccotangent_token;
		case token_arcsecant:
			return new arcsecant_token;
		case token_arccosecant:
			return new arccosecant_token;
		case token_common_logarithm:
			return new common_logarithm_token;
		case token_natural_logarithm:
			return new natural_logarithm_token;
		case token_square_root:
			return new square_root_token;
		case token_cubic_root:
			return new cubic_root_token;
		case token_factorial:
			return new factorial_token;
		case token_modulo:
			return new modulo_token;
		case token_degree:
			return new degree_token;
		case token_radian:
			return new radian_token;
		default:
			throw std::runtime_error("未知的运算符令牌类型");
		}
	}
};