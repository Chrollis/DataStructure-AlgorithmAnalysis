#ifndef CHRVALIDATOR_H
#define CHRVALIDATOR_H

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <stack>
#include <algorithm>

namespace chr {
	// 表达式词法分析器：将输入表达式拆分成记号(token)并做初步分类与错误收集
	class expression_tokenizer {
	private:
		std::vector<std::string> _tokens; // 解析出的记号序列（按出现顺序）
		std::vector<std::pair<std::string, std::string>> _errors; // 收集的错误：{位置或片段, 错误描述}

		// 支持的函数列表（小写）
		static const std::vector<std::string> functions;
		// 支持的常量（大写）
		static const std::vector<std::string> constants;
	public:
		// 判断是否为单字符运算符或括号/阶乘/模运算符
		static bool is_operator(const std::string& token);
		// 判断是否为函数名（sin, cos 等）
		static bool is_function(const std::string& token);
		// 判断是否为常量（PI, E, PHI）
		static bool is_constant(const std::string& token);
		// 判断是否为数值（支持二/八/十/十六进制、小数与科学计数法、以及常量文字）
		static bool is_number(const std::string& token);
		// 返回记号类别字符串，便于打印与调试（例如 "DECIMAL"/"OPERATOR"）
		static std::string get_token_type(const std::string& token);
	private:
		// 处理一元 + / -：将其替换为 "pos"/"neg"，以便后续语法检查和解析
		void proceess_unary_operators();
	public:
		// 将表达式字符串切分为记号；解析成功返回 true，否则 false（并收集错误）
		bool tokenize(const std::string& expr);
		// 调试用：打印当前记号及其类型
		void print_tokens(std::ostream& os)const;
		// 打印收集到的错误（位置/片段与描述）
		void print_errors(std::ostream& os)const;
		// 外部或验证器可调用以添加错误项
		void add_error(const std::pair<std::string, std::string>& error);
		// 访问器
		const std::vector<std::string>& tokens()const { return _tokens; }
		const std::vector<std::pair<std::string, std::string>>& errors()const { return _errors; }
	};

	// 表达式验证器：在词法分析基础上进行括号、运算符序列、数字格式、函数使用等语法检查
	class expression_validator {
	private:
		expression_tokenizer _tokenizer;
	private:
		// 检查括号匹配；记录多余的左/右括号位置
		void check_parentheses(const std::vector<std::string>& tokens);
		// 检查运算符/操作数的合法序列（连续运算符、表达式开头/结尾的二元运算符等）
		void check_operator_sequence(const std::vector<std::string>& tokens);
		// 验证数字格式：科学计数法只允许十进制，同时检查二/八/十六进制格式
		void check_number_format(const std::vector<std::string>& tokens);
		// 检查函数名后必需跟左括号
		void check_function_usage(const std::vector<std::string>& tokens);
	public:
		// 执行完整验证流程：词法 -> 多项检查 -> 返回是否无错误
		bool validate_expression(const std::string& expr);
		// 打印详细分析（记号与错误）
		void print_detailed_analysis(std::ostream& os)const;
		// 获取内部 tokenizer（用于外部调试）
		const expression_tokenizer& tokenizer() { return _tokenizer; }
	};
}

#endif // CHRVALIDATOR_H
