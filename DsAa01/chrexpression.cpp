#include "chrexpression.h"

namespace chr {

	void basic_expression::calculate(std::stack<number_token>& operand_stack, const operator_token& op)
	{
		// 根据运算符需要的操作数个数，从操作数栈取值并执行运算
		int operands = op.operand_num();
		if (operands == 0) {
			throw std::runtime_error("计算时出现零操作数运算符");
		}
		else if (operands == 1) {
			// 一元操作符：使用栈顶作为唯一操作数（作为 left），将结果写回栈顶
			double left = operand_stack.top().value();
			operand_stack.top().rvalue() = op.apply(left, 0);
		}
		else if (operands == 2) {
			// 二元操作符：注意右操作数在栈顶，先取右再取左
			double right = operand_stack.top().value();
			operand_stack.pop();
			double left = operand_stack.top().value();
			operand_stack.top().rvalue() = op.apply(left, right);
		}
		else {
			// 不支持超过两个操作数的运算符
			throw std::runtime_error("计算时出现操作数多于两个的运算符");
		}
	}

	basic_expression::~basic_expression()
	{
		// 释放所有在 _content 中分配的令牌，避免内存泄漏
		for (auto token : _content) {
			delete token;
		}
	}

	std::ostream& operator<<(std::ostream& os, const basic_expression& expr) 
	{
		// 以可读形式打印表达式：数字直接打印值，其他令牌打印其字符串表示
		std::vector<basic_token*> token_list = expr.content();
		for (const basic_token* const token : token_list) {
			if (token->type() == token_number) {
				os << dynamic_cast<const number_token*>(token)->value() << ' ';
			}
			else {
				os << dynamic_cast<const operator_token*>(token)->str() << ' ';
			}
		}
		return os;
	}

	infix_expression::infix_expression(const std::string& infix_expr_str)
	{
		expression_validator validator;
		if (!validator.validate_expression(infix_expr_str)) {
			std::ostringstream oss;
			oss << "表达式非法：\n";
			validator.print_detailed_analysis(oss);
			std::string error = oss.str();
			error.pop_back();
			throw std::runtime_error(error);
		}
		std::vector<std::string> tokens = validator.tokenizer().tokens();
		for (const auto& token : tokens) {
			if (expression_tokenizer::is_number(token)) {
				std::string type = expression_tokenizer::get_token_type(token);
				if (type == "DECIMAL") {
					_content.push_back(new number_token(std::stod(token)));
				}
				else if (type == "CONSTANT") {
					if (token == "E") {
						_content.push_back(new number_token(natural_constant));
					}
					else if (token == "PI") {
						_content.push_back(new number_token(pi));
					}
					else if (token == "PHI") {
						_content.push_back(new number_token(phi));
					}
					else {
						throw std::runtime_error("无效常数");
					}
				}
				else {
					double out = 0;
					int radix = 10;
					std::string integer;
					std::string fraction;
					if (type == "BINARY") {
						radix = 2;
					}
					else if (type == "OCTAL") {
						radix = 8;
					}
					else if (type == "HEXADECIMAL") {
						radix = 16;
					}
					else {
						throw std::runtime_error("无效进制");
					}
					size_t dot_pos = token.find('.');
					if (dot_pos == std::string::npos) {
						integer = token.substr(2);
					}
					else {
						integer = token.substr(2, dot_pos - 2);
						fraction = token.substr(dot_pos + 1);
					}
					for (size_t i = 0; i < integer.length(); i++) {
						char t = integer[integer.length() - i - 1];
						out += pow(radix, i) * (t <= '9' ? t - '0' : t <= 'Z' ? t - 'A' + 10 : t - 'a' + 10);
					}
					for (size_t i = 0; i < fraction.length(); i++) {
						char t = fraction[i];
						out += pow(radix, -(int(i) + 1)) * (t <= '9' ? t - '0' : t <= 'Z' ? t - 'A' + 10 : t - 'a' + 10);
					}
					_content.push_back(new number_token(out));
				}
				continue;
			}
			token_t type = string_to_operator_token_type(token);
			_content.push_back(token_type_to_operator_token(type));
		}
	}

	double infix_expression::evaluate()const {
		// 使用经典的两个栈算法：一个为操作数栈（number_token），一个为运算符栈（operator_token*）
		std::stack<number_token> operand_stack;
		std::stack<operator_token*> operator_stack;
		for (auto token : _content) {
			if (token->type() == token_number) {
				// 将数字值拷贝入操作数栈（注意使用 value() 构造）
				operand_stack.push(number_token(dynamic_cast<number_token*>(token)->value()));
			}
			else {
				operator_token* op = dynamic_cast<operator_token*>(token);
				if (op->type() == token_left_parentheses) {
					// 左括号直接入栈（作为标记）
					operator_stack.push(token_type_to_operator_token(op->type()));
				}
				else if (op->type() == token_right_parentheses) {
					while (!operator_stack.empty()) {
						if (operator_stack.top()->type() == token_left_parentheses) {
						    delete operator_stack.top();
							operator_stack.pop();
							break;
						}
						else {
							// 普通运算符，应用并释放
							calculate(operand_stack, *operator_stack.top());
							delete operator_stack.top();
							operator_stack.pop();
						}
					}
				}
				else {
					// 对于普通运算符，基于优先级决定是否先弹出栈顶运算符求值
					while (!operator_stack.empty() && operator_stack.top()->priority() >= op->priority()) {
						calculate(operand_stack, *operator_stack.top());
						delete operator_stack.top();
						operator_stack.pop();
					}
					operator_stack.push(token_type_to_operator_token(op->type()));
				}
			}
		}
		// 处理剩余运算符
		while (!operator_stack.empty()) {
			calculate(operand_stack, *operator_stack.top());
			delete operator_stack.top();
			operator_stack.pop();
		}
		// 最终操作数栈中应只剩一个结果
		if (operand_stack.size() != 1) {
			throw std::runtime_error("运算结束时出错，操作数栈不只有一个元素");
		}
		else {
			return operand_stack.top().value();
		}
	}

	postfix_expression::postfix_expression(const std::string& infix_expr_str)
	{
		// 先构造中缀表达式并获取其令牌流，再使用类似 shunting-yard 的方法生成后缀序列
		infix_expression* infix_expr = new infix_expression(infix_expr_str);
		std::vector<basic_token*> infix_tokens = infix_expr->content();
		std::stack<operator_token*> op_stack;
		for (auto token : infix_tokens) {
			int type = token->type();
			if (type == token_number) {
				// 数字直接追加到后缀序列
				_content.push_back(new number_token(dynamic_cast<number_token*>(token)->value()));
			}
			else {
				operator_token* op = dynamic_cast<operator_token*>(token);
				if (op->type() == token_left_parentheses) {
					// 左括号入栈
					op_stack.push(op);
				}
				else if (type == token_right_parentheses) {
					// 右括号：弹出直到左括号
					while (!op_stack.empty()) {
						if (op_stack.top()->type() == token_left_parentheses) {
							op_stack.pop();
							break;
						}
						else {
							_content.push_back(token_type_to_operator_token(op_stack.top()->type()));
							op_stack.pop();
						}
					}
				}
				else {
					// 按优先级弹栈再入栈
					while (!op_stack.empty() && op_stack.top()->priority() >= op->priority()) {
						_content.push_back(token_type_to_operator_token(op_stack.top()->type()));
						op_stack.pop();
					}
					op_stack.push(dynamic_cast<operator_token*>(token));
				}
			}
		}
		// 将剩余运算符依次输出
		while (!op_stack.empty()) {
			_content.push_back(token_type_to_operator_token(op_stack.top()->type()));
			op_stack.pop();
		}
		delete infix_expr;
	}

	double postfix_expression::evaluate()const {
		// 后缀求值：遇到数字入栈，遇到运算符则调用 calculate
		std::stack<number_token> operand_stack;
		for (auto token : _content) {
			if (token->type() == token_number) {
				operand_stack.push(number_token(dynamic_cast<number_token*>(token)->value()));
			}
			else {
				calculate(operand_stack, *dynamic_cast<operator_token*>(token));
			}
		}
		if (operand_stack.size() != 1) {
			throw std::runtime_error("运算结束时出错，操作数栈不只有一个元素");
		}
		else {
			return operand_stack.top().value();
		}
	}
}