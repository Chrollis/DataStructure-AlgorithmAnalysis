#ifndef CHRTOKEN_H
#define CHRTOKEN_H

#include <string>
#include <cmath>
#include <stdexcept>

namespace chr {
	constexpr double natural_constant = 2.718281828459;
	constexpr double pi = 3.1415826535898;
	constexpr double phi = 0.61803398875;
	// 词元类型枚举：表示解析表达式时可能遇到的所有 token 种类
	enum token_type {
		token_invalid,
		token_number,
		token_left_parentheses,
		token_right_parentheses,
		token_plus,
		token_minus,
		token_multiply,
		token_divide,
		token_posite,				// 一元正号（前缀），与 token_plus 区分
		token_negate,               // 一元负号（前缀），与 token_minus 区分
		token_exponent,             // 幂运算 ^
		token_sine,                 // sin(...)
		token_cosine,               // cos(...)
		token_tangent,              // tan(...)
		token_cotangent,
		token_secant,
		token_cosecant,
		token_arcsine,
		token_arccosine,
		token_arctangent,
		token_arccotangent,
		token_arcsecant,
		token_arccosecant,
		token_common_logarithm,     // lg(...)
		token_natural_logarithm,    // ln(...)
		token_square_root,          // sqrt(...)
		token_cubic_root,
		token_factorial,            // 阶乘后缀运算符 "!"
		token_modulo,					//取余运算 %
		token_degree,					//转角度 deg(...)
		token_radian					//转弧度 rad(...)
	};

	typedef unsigned short token_t; // 用于表示 token_type 的底层整数类型
	typedef unsigned char byte;     // 小整数类型，用于返回操作数数量、优先级等

	// 基本 token：保存 token 类型的共同基类
	class basic_token {
	protected:
		token_t _type;
	public:
		basic_token(token_t type) :_type(type) {}
		virtual ~basic_token() = default;
		token_t type()const { return _type; } // 返回 token 的类型
	};

	// 数字 token：封装数字值
	class number_token :public basic_token {
	private:
		double _value;
	public:
		number_token(double value) :basic_token(token_number), _value(value) {}
		~number_token() = default;
		double value()const { return _value; }
		double& rvalue() { return _value; } // 返回对内部值的引用，允许原地修改
	};

	// 操作符/函数 token 的抽象基类
	class operator_token :public basic_token {
	public:
		operator_token(token_t type) :basic_token(type) {}
		virtual ~operator_token() = default;
		// 返回操作符的文本表示（用于调试 / 打印）
		virtual std::string str()const  = 0;
		// 返回该操作符需要的操作数个数（0、1、2）
		virtual byte operand_num()const  = 0;
		// 返回优先级数值：数值越大绑定越紧（先计算）
		virtual byte priority()const  = 0;
		// 如果为 true，表示该操作符是后缀式的（例如阶乘 "!"）
		virtual bool is_suffix_operator()const  = 0;
		// 执行操作：对于一元操作，使用 left；对于二元操作，使用 left 和 right
		virtual double apply(double left, double right)const  = 0;
	};

	// 操作符/函数 token 类的模板
	/*
	class xxx_token :public operator_token {
	public:
		xxx_token() :operator_token(token_xxx) {}
		~xxx_token() = default;
		std::string str()const  { return ""; }
		byte operand_num()const  { return 0; }
		byte priority()const  { return 0; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return 0; }
	};
	*/

	// 二元加法
	class plus_token :public operator_token {
	public:
		plus_token() :operator_token(token_plus) {}
		~plus_token() = default;
		std::string str()const  { return "+"; }
		byte operand_num()const  { return 2; }
		byte priority()const  { return 1; } // 加减优先级较低
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return left + right; }
	};

	// 二元减法
	class minus_token :public operator_token {
	public:
		minus_token() :operator_token(token_minus) {}
		~minus_token() = default;
		std::string str()const  { return "-"; }
		byte operand_num()const  { return 2; }
		byte priority()const  { return 1; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return left - right; }
	};

	// 乘法
	class multiply_token :public operator_token {
	public:
		multiply_token() :operator_token(token_multiply) {}
		~multiply_token() = default;
		std::string str()const  { return "*"; }
		byte operand_num()const  { return 2; }
		byte priority()const  { return 3; } // 乘除优先级高于加减
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return left * right; }
	};

	// 除法
	class divide_token :public operator_token {
	public:
		divide_token() :operator_token(token_divide) {}
		~divide_token() = default;
		std::string str()const  { return "/"; }
		byte operand_num()const  { return 2; }
		byte priority()const  { return 3; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return left / right; }
	};

	// 一元取正（前缀）：对单目操作符使用 left 作为操作数
	class posite_token :public operator_token {
	public:
		posite_token() :operator_token(token_posite) {}
		~posite_token() = default;
		std::string str()const { return "pos"; } // 用 "pos" 表示一元负号以区别于二元加号
		byte operand_num()const { return 1; }
		byte priority()const { return 4; } // 一元运算优先级通常高
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return left; }
	};

	// 一元取负（前缀）：对单目操作符使用 left 作为操作数
	class negate_token :public operator_token {
	public:
		negate_token() :operator_token(token_negate) {}
		~negate_token() = default;
		std::string str()const  { return "neg"; } // 用 "neg" 表示一元负号以区别于二元减号
		byte operand_num()const  { return 1; }
		byte priority()const  { return 4; } // 一元运算优先级通常高
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return -left; }
	};

	// 幂运算
	class exponent_token :public operator_token {
	public:
		exponent_token() :operator_token(token_exponent) {}
		~exponent_token() = default;
		std::string str()const  { return "^"; }
		byte operand_num()const  { return 2; }
		byte priority()const  { return 5; } // 幂运算优先级高
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return pow(left, right); }
	};

	// 左括号：用于解析分组或函数调用
	class left_parentheses_token :public operator_token {
	public:
		left_parentheses_token() :operator_token(token_left_parentheses) {}
		~left_parentheses_token() = default;
		std::string str()const  { return "("; }
		byte operand_num()const  { return 0; }
		byte priority()const  { return 0; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return 0; }
	};

	// 右括号：用于标识分组结束
	class right_parentheses_token :public operator_token {
	public:
		right_parentheses_token() :operator_token(token_right_parentheses) {}
		~right_parentheses_token() = default;
		std::string str()const  { return ")"; }
		byte operand_num()const  { return 0; }
		byte priority()const  { return 0; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return 0; }
	};

	// 三角函数
	class sine_token :public operator_token {
	public:
		sine_token() :operator_token(token_sine) {}
		~sine_token() = default;
		std::string str()const  { return "sin"; }
		byte operand_num()const  { return 1; }
		byte priority()const  { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return sin(left); }
	};

	class cosine_token :public operator_token {
	public:
		cosine_token() :operator_token(token_cosine) {}
		~cosine_token() = default;
		std::string str()const  { return "cos"; }
		byte operand_num()const  { return 1; }
		byte priority()const  { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return cos(left); }
	};

	class tangent_token :public operator_token {
	public:
		tangent_token() :operator_token(token_tangent) {}
		~tangent_token() = default;
		std::string str()const  { return "tan"; }
		byte operand_num()const  { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return tan(left); }
	};

	class cotangent_token :public operator_token {
	public:
		cotangent_token() :operator_token(token_cotangent) {}
		~cotangent_token() = default;
		std::string str()const { return "cot"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return 1.0 / tan(left); }
	};

	class secant_token :public operator_token {
	public:
		secant_token() :operator_token(token_secant) {}
		~secant_token() = default;
		std::string str()const { return "sec"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return 1.0 / cos(left); }
	};

	class cosecant_token :public operator_token {
	public:
		cosecant_token() :operator_token(token_cosecant) {}
		~cosecant_token() = default;
		std::string str()const { return "csc"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return 1.0 / sin(left); }
	};

	class arcsine_token :public operator_token {
	public:
		arcsine_token() :operator_token(token_arcsine) {}
		~arcsine_token() = default;
		std::string str()const { return "arcsin"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return asin(left); }
	};

	class arccosine_token :public operator_token {
	public:
		arccosine_token() :operator_token(token_arccosine) {}
		~arccosine_token() = default;
		std::string str()const { return "arccos"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return acos(left); }
	};

	class arctangent_token :public operator_token {
	public:
		arctangent_token() :operator_token(token_arctangent) {}
		~arctangent_token() = default;
		std::string str()const { return "arctan"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return atan(left); }
	};

	class arccotangent_token :public operator_token {
	public:
		arccotangent_token() :operator_token(token_arccotangent) {}
		~arccotangent_token() = default;
		std::string str()const { return "arccot"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return atan(1.0 / left); }
	};

	class arcsecant_token :public operator_token {
	public:
		arcsecant_token() :operator_token(token_arcsecant) {}
		~arcsecant_token() = default;
		std::string str()const { return "arcsec"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return acos(1.0 / left); }
	};

	class arccosecant_token :public operator_token {
	public:
		arccosecant_token() :operator_token(token_arccosecant) {}
		~arccosecant_token() = default;
		std::string str()const { return "arccsc"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return asin(1.0 / left); }
	};

	// 常用对数（以 10 为底）
	class common_logarithm_token :public operator_token {
	public:
		common_logarithm_token() :operator_token(token_common_logarithm) {}
		~common_logarithm_token() = default;
		std::string str()const  { return "lg"; }
		byte operand_num()const  { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return log10(left); }
	};

	// 自然对数
	class natural_logarithm_token :public operator_token {
	public:
		natural_logarithm_token() :operator_token(token_natural_logarithm) {}
		~natural_logarithm_token() = default;
		std::string str()const  { return "ln"; }
		byte operand_num()const  { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return log(left); }
	};

	// 平方根
	class square_root_token :public operator_token {
	public:
		square_root_token() :operator_token(token_square_root) {}
		~square_root_token() = default;
		std::string str()const  { return "sqrt"; }
		byte operand_num()const  { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const  { return 0; };
		double apply(double left, double right)const  { return sqrt(left); }
	};

	// 立方根
	class cubic_root_token :public operator_token {
	public:
		cubic_root_token() :operator_token(token_cubic_root) {}
		~cubic_root_token() = default;
		std::string str()const { return "cbrt"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return cbrt(left); }
	};

	// 阶乘：后缀运算，使用 Gamma 函数实现（tgamma(n+1) = n!），注意对非负整数的期望行为
	class factorial_token :public operator_token {
	public:
		factorial_token() :operator_token(token_factorial) {}
		~factorial_token() = default;
		std::string str()const  { return "!"; }
		byte operand_num()const  { return 1; }
		byte priority()const  { return 6; } // 阶乘优先级通常最高
		bool is_suffix_operator()const  { return 1; }; // 表示后缀运算符
		double apply(double left, double right)const  { return tgamma(left + 1); }
	};

	// 取模
	class modulo_token :public operator_token {
	public:
		modulo_token() :operator_token(token_modulo) {}
		~modulo_token() = default;
		std::string str()const { return "%"; }
		byte operand_num()const { return 2; }
		byte priority()const { return 2; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return fmod(left, right); }
	};

	//转角度值
	class degree_token :public operator_token {
	public:
		degree_token() :operator_token(token_degree) {}
		~degree_token() = default;
		std::string str()const { return "deg"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return left / pi * 180; }
	};

	//转弧度值
	class radian_token :public operator_token {
	public:
		radian_token() :operator_token(token_radian) {}
		~radian_token() = default;
		std::string str()const { return "rad"; }
		byte operand_num()const { return 1; }
		byte priority()const { return 7; }
		bool is_suffix_operator()const { return 0; };
		double apply(double left, double right)const { return left / 180 * pi; }
	};

	// 将运算符字符串映射到对应的 token_type（词法分析时使用）
	token_t string_to_operator_token_type(const std::string& operator_str);

	// 由token_type映射到operator_token
	operator_token* token_type_to_operator_token(token_t operator_type);
}
#endif
