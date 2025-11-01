#ifndef CHREXPRESSION_H
#define CHREXPRESSION_H

#include "chrtoken.h"
#include "chrvalidator.h"
#include <sstream>

namespace chr {
	class basic_expression {
	protected:
		// 表示表达式的令牌序列（按从左到右的顺序）
		std::vector<basic_token*> _content;
		// 在求值过程中对栈顶操作数应用运算符（静态工具函数）
		static void calculate(std::stack<number_token>& operand_stack, const operator_token& op);
	public:
		basic_expression() = default;
		// 返回当前表达式中存储的令牌列表（按值拷贝返回，保持封装）
		std::vector<basic_token*> content()const { return _content; }
		virtual ~basic_expression();
		// 具体表达式的求值接口：中缀/后缀派生类需实现
		virtual double evaluate()const = 0;
	};

	// 将表达式以可读形式输出到流（用于调试）
	std::ostream& operator<<(std::ostream& os, const basic_expression& expr);

	// 中缀表达式：负责从字符串解析为令牌流并实现中缀求值
	class infix_expression :public basic_expression {
	public:
		// 从中缀字符串构建表达式（解析为内部令牌序列）
		infix_expression(const std::string& infix_expr_str);
		~infix_expression() = default;
		// 使用栈（运算数栈 + 运算符栈）对中缀表达式求值
		double evaluate()const;
	};

	// 后缀表达式：从中缀字符串转换为后缀（逆波兰）并求值
	class postfix_expression :public basic_expression {
	public:
		postfix_expression(const std::string& infix_expr_str);
		~postfix_expression() = default;
		// 对后缀表达式直接进行基于栈的求值
		double evaluate()const;
	};
};

#endif
