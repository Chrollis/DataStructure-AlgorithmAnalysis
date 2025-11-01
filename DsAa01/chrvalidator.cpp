#include "chrvalidator.h"

namespace chr {
	// 支持的函数名称（与 header 中声明对应）
	const std::vector<std::string> expression_tokenizer::functions = {
		   "sin", "cos", "tan", "cot", "sec", "csc",
		   "arcsin", "arccos", "arctan", "arccot", "arcsec", "arccsc",
		   "ln", "lg", "deg", "rad", "sqrt", "cbrt"
	};
	// 支持的常量（用于词法识别与打印）
	const std::vector<std::string> expression_tokenizer::constants = { 
		"PI", "E", "PHI" 
	};

	// 判断是否为运算符或括号：使用正则匹配单字符运算符集
	bool expression_tokenizer::is_operator(const std::string& token) {
		// 支持 + - * / ^ ( ) ! %
		return std::regex_match(token, std::regex(R"([+\-*/^()!%])"));
	}

	// 判断是否为函数名（在 functions 列表中）
	bool expression_tokenizer::is_function(const std::string& token) {
		return std::find(functions.begin(), functions.end(), token) != functions.end();
	}

	// 判断是否为常量（PI/E/PHI）
	bool expression_tokenizer::is_constant(const std::string& token) {
		return std::find(constants.begin(), constants.end(), token) != constants.end();
	}

	// 判断是否为数字字面量（包括二/八/十/十六进制、小数、科学计数法以及常量字符串）
	bool expression_tokenizer::is_number(const std::string& token)
	{
		std::regex pattern(
			// 二进制：0b101(.101)?
			R"((0b[01]+(\.[01]*)?)|)"
			// 八进制：0o755(.7)?
			R"((0o[0-7]+(\.[0-7]*)?)|)"
			// 十六进制：0xFF(.A)?
			R"((0x[0-9A-Fa-f]+(\.[0-9A-Fa-f]*)?)|)"
			// 十进制数（包含小数与可选的科学计数法部分）
			R"((\d+\.?\d*|\.\d+)([eE][-+]?\d+)?|)"
			// 常量文字也视为“数字”类别以便后续处理
			R"(PI|E|PHI|)"
		);
		return std::regex_match(token, pattern);
	}

	// 将表达式中的一元 + / - 替换为 "pos"/"neg"（便于解析器区分）
	void expression_tokenizer::proceess_unary_operators() {
		std::vector<std::string> processed_tokens;
		for (size_t i = 0; i < _tokens.size(); ++i) {
			const std::string& token = _tokens[i];
			// 检查是否是 + 或 - 可能是一元运算符
			if (token == "+" || token == "-") {
				// 判断是否为一元运算符的情况：
				// 1. 表达式开头
				// 2. 前一个 token 是运算符（但如果是右括号或阶乘 '!' 则不是一元）
				// 3. 前一个 token 是函数名（函数名后面跟的是左括号，此时 '+'/'-' 为一元符）
				if (i == 0 || (i > 0 && (is_operator(_tokens[i - 1]) && _tokens[i - 1] != ")" && _tokens[i - 1] != "!") || is_function(_tokens[i - 1]))) {
					// 替换为一元运算符标识
					processed_tokens.push_back(token == "+" ? "pos" : "neg");
				}
				else {
					processed_tokens.push_back(token);
				}
			}
			else {
				processed_tokens.push_back(token);
			}
		}
		_tokens = processed_tokens;
	}

	// 返回给定记号的类型字符串（用于调试输出）
	std::string expression_tokenizer::get_token_type(const std::string& token){
		// 检查二进制/八进制/十六进制前缀
		if (std::regex_match(token, std::regex(R"(0b[01]+(\.[01]*)?)"))) {
			return "BINARY";
		}
		else if (std::regex_match(token, std::regex(R"(0o[0-7]+(\.[0-7]*)?)"))) {
			return "OCTAL";
		}
		else if (std::regex_match(token, std::regex(R"(0x[0-9A-Fa-f]+(\.[0-9A-Fa-f]*)?)"))) {
			return "HEXADECIMAL";
		}
		// 十进制（含科学计数法）
		else if (std::regex_match(token, std::regex(R"((\d+\.?\d*|\.\d+)([eE][-+]?\d+)?)"))) {
			return "DECIMAL";
		}
		// 运算符
		else if (is_operator(token)) {
			return "OPERATOR";
		}
		// 常量
		else if (is_constant(token)) {
			return "CONSTANT";
		}
		// 函数
		else if (is_function(token)) {
			return "FUNCTION";
		}
		// 已处理的一元运算符标识
		else if (token == "pos" || token == "neg") {
			return "UNARY_OPERATOR";
		}
		return "UNKNOWN";
	}

	// 将输入表达式拆分为记号序列并做基础错误定位（例如无法识别的字符片段）
	bool expression_tokenizer::tokenize(const std::string& expr) {
		_tokens.clear();
		_errors.clear();
		std::string processed_expr = expr;
		std::regex pattern(
			// 二、八、十六进制数
			R"((0b[01]+(\.[01]*)?)|)"
			R"((0o[0-7]+(\.[0-7]*)?)|)"
			R"((0x[0-9A-Fa-f]+(\.[0-9A-Fa-f]*)?)|)"
			// 十进制数（包括科学计数法）
			R"((\d+\.?\d*|\.\d+)([eE][-+]?\d+)?|)"
			// 运算符
			R"([+\-*/^()!%]|)"
			// 常量
			R"(PI|E|PHI|)"
			// 函数（列出所有受支持的函数）
			R"(sin|cos|tan|cot|sec|csc|)"
			R"(arcsin|arccos|arctan|arccot|arcsec|arccsc|)"
			R"(ln|lg|deg|rad|sqrt|cbrt|)"
		);
		size_t pos = 0;
		auto words_begin = std::sregex_iterator(processed_expr.begin(), processed_expr.end(), pattern);
		auto words_end = std::sregex_iterator();
		for (auto& it = words_begin; it != words_end; it++) {
			std::smatch match = *it;
			std::string token = match.str();
			size_t token_pos = match.position();
			// 跳过纯空白匹配
			if (std::all_of(token.begin(), token.end(), isspace)) {
				continue;
			}
			// 如果当前匹配之前存在未匹配的字符片段，则报为无法识别
			if (token_pos > pos) {
				std::string unknown = processed_expr.substr(pos, token_pos - pos);
				if (!std::all_of(unknown.begin(), unknown.end(), isspace)) {
					_errors.push_back({ unknown,"无法识别的字符或符号" });
				}
			}
			_tokens.push_back(token);
			pos = token_pos + token.length();
		}
		// 如果最后还有剩余未匹配字符，记录错误
		if (pos < processed_expr.length()) {
			std::string remaining = processed_expr.substr(pos);
			if (!std::all_of(remaining.begin(), remaining.end(), isspace)) {
				_errors.push_back({ remaining, "表达式末尾有无法识别的字符" });
			}
		}
		// 处理一元运算符，使后续验证/解析更简单
		proceess_unary_operators();
		return _errors.empty();
	}

	// 调试输出：每个 token 的类型与文本
	void expression_tokenizer::print_tokens(std::ostream& os) const {
		for (const auto& token : _tokens) {
			std::string type = get_token_type(token);
			os << "[" << type << "] " << token << "\n";
		}
	}

	// 输出收集到的错误：以 "位置/片段：描述" 形式打印
	void expression_tokenizer::print_errors(std::ostream& os) const {
		for (const auto& error : _errors) {
			os << "位置【" + error.first + "】：" + error.second + "\n";
		}
	}

	// 外部调用时把错误追加到内部错误列表
	void expression_tokenizer::add_error(const std::pair<std::string, std::string>& error) {
		_errors.push_back(error);
	}


	// ----------------- expression_validator 实现 -----------------

	// 使用栈匹配括号，记录多余的左/右括号及其位置
	void expression_validator::check_parentheses(const std::vector<std::string>& tokens) {
		std::stack<std::pair<std::string, size_t>>paren_stack;
		for (size_t i = 0; i < tokens.size(); i++) {
			const auto& token = tokens[i];
			if (token == "(") {
				paren_stack.push({ token,i });
			}
			else if (token == ")") {
				if (paren_stack.empty()) {
					_tokenizer.add_error({ std::to_string(i),"存在多余的右括弧" });
				}
				else {
					paren_stack.pop();
				}
			}
		}
		// 剩余的左括弧均为未闭合
		while (!paren_stack.empty()) {
			_tokenizer.add_error({ std::to_string(paren_stack.top().second),"存在多余的左括弧" });
			paren_stack.pop();
		}
	}

	// 检查运算符顺序和位置：二元/一元运算符、阶乘的合法前置等
	void expression_validator::check_operator_sequence(const std::vector<std::string>& tokens) {
		const std::vector<std::string> binary_operators = { "+", "-", "*", "/", "^", "%" };
		const std::vector<std::string> unary_operators = { "pos","neg" };
		for (size_t i = 0; i < tokens.size(); ++i) {
			const std::string& token = tokens[i];
			// 检查二元运算符位置：不能出现在表达式开头或结尾，不能连续出现
			if (std::find(binary_operators.begin(), binary_operators.end(), token) != binary_operators.end()) {
				if (i == 0) {
					_tokenizer.add_error({ std::to_string(i),"表达式以二元运算符开头" });
				}
				else if (i == tokens.size() - 1) {
					_tokenizer.add_error({ std::to_string(i),"表达式以运算符结尾" });
				}
				else {
					if (std::find(binary_operators.begin(), binary_operators.end(), tokens[i - 1]) != binary_operators.end()) {
						_tokenizer.add_error({ std::to_string(i),"表达式含有连续二元运算符" });
					}
				}
			}
			// 检查一元运算符位置：一元运算符不能出现在表达式结尾，也不能连续出现
			if (std::find(unary_operators.begin(), unary_operators.end(), token) != unary_operators.end()) {
				if (i == tokens.size() - 1) {
					_tokenizer.add_error({ std::to_string(i),"表达式以运算符结尾" });
				}
				else {
					if (i != 0 && std::find(unary_operators.begin(), unary_operators.end(), tokens[i - 1]) != unary_operators.end()) {
						_tokenizer.add_error({ std::to_string(i),"表达式含有连续一元运算符" });
					}
				}
			}
			// 检查阶乘运算符：必须有有效的前驱（数字/常量/右括号）
			if (token == "!") {
				if (i == 0) {
					_tokenizer.add_error({ std::to_string(i),"表达式以阶乘运算符开头" });
				}
				else {
					// !前面必须是数字、常量、变量或右括号
					const std::string& prev = tokens[i - 1];
					if (!(std::regex_match(prev, std::regex(R"((\d+\.?\d*|\.\d+)([eE][-+]?\d+)?)")) ||
						std::regex_match(prev, std::regex(R"(0[bxo][0-9A-Fa-f.]+)")) ||
						prev == ")" || prev == "PI" || prev == "E" || prev == "PHI")) {
						_tokenizer.add_error({ std::to_string(i),"阶乘运算符前面必须是数字、常量或表达式" });
					}
				}
			}
		}
	}

	// 验证数字格式：检查连续数字、科学计数法（仅十进制）、以及进制字面量的合法性
	void expression_validator::check_number_format(const std::vector<std::string>& tokens) {
		for (size_t i = 0; i < tokens.size(); i++) {
			const auto& token = tokens[i];
			// 仅对被识别为数字且不是常量的记号进行格式检查
			if (expression_tokenizer::is_number(token) && !expression_tokenizer::is_constant(token)) {
				// 连续数字（例如 "12 34"）被视为错误
				if (i > 0 && expression_tokenizer::is_number(tokens[i - 1])) {
					_tokenizer.add_error({ tokens[i - 1] + token,"表达式含有连续数字" });
				}
				else {
					// 如果包含 e/E，确保其为合法的十进制科学计数法（禁止在 0x/0o/0b 前缀中使用 e）
					if ((token.find('e') != std::string::npos || token.find('E') != std::string::npos) &&
						!token.starts_with("0x") && !token.starts_with("0o") && !token.starts_with("0b")) {
						if (!std::regex_match(token, std::regex(R"([+-]?(\d+\.?\d*|\.\d+)[eE][-+]?\d+)"))) {
							_tokenizer.add_error({ token,"科学计数法格式错误" });
						}
					}
					// 二进制格式检查
					if (token.starts_with("0b") &&
						!std::regex_match(token, std::regex(R"(0b[01]+(\.[01]*)?)"))) {
						_tokenizer.add_error({ token,"二进制格式错误" });
					}
					// 八进制格式检查
					else if (token.starts_with("0o") &&
						!std::regex_match(token, std::regex(R"(0o[0-7]+(\.[0-7]*)?)"))) {
						_tokenizer.add_error({ token,"八进制格式错误" });
					}
					// 十六进制格式检查
					else if (token.starts_with("0x") &&
						!std::regex_match(token, std::regex(R"(0x[0-9A-Fa-f]+(\.[0-9A-Fa-f]*)?)"))) {
						_tokenizer.add_error({ token,"十六进制格式错误" });
					}
				}
			}
		}
	}

	// 检查函数使用：函数名后必须紧跟左括号
	void expression_validator::check_function_usage(const std::vector<std::string>& tokens) {
		const std::vector<std::string> functions = {
			"sin", "cos", "tan", "cot", "sec", "csc",
			"arcsin", "arccos", "arctan", "arccot", "arcsec", "arccsc",
			"ln", "lg", "deg", "rad", "sqrt", "cbrt"
		};
		for (size_t i = 0; i < tokens.size(); ++i) {
			const std::string& token = tokens[i];
			// 若是已知函数名但后面不是左括号，则报错
			if (std::find(functions.begin(), functions.end(), token) != functions.end()) {
				if (i + 1 >= tokens.size() || tokens[i + 1] != "(") {
					_tokenizer.add_error({ token,"函数名未紧跟左括号" });
				}
			}
		}
	}

	// 验证入口：先词法分析，若成功则依次执行各类语法检查，返回是否无错误
	bool expression_validator::validate_expression(const std::string& expr) {
		if (!_tokenizer.tokenize(expr)) {
			return 0;
		}
		const auto& tokens = _tokenizer.tokens();
		check_parentheses(tokens);
		check_operator_sequence(tokens);
		check_number_format(tokens);
		check_function_usage(tokens);
		const auto& errors = _tokenizer.errors();
		return errors.empty();
	}

	// 打印详细分析：先列出记号再列出错误
	void expression_validator::print_detailed_analysis(std::ostream& os) const {
		_tokenizer.print_tokens(os);
		_tokenizer.print_errors(os);
	}

}
