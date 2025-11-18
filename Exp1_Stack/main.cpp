#include "calculator.hpp"

int main()
{
    std::string str;
    // 简单 REPL：读取行、解析、输出中缀/后缀并计算结果
    while (1) {
        try
        {
            std::cout << "输入表达式：";
            std::getline(std::cin, str);
            // 简易命令：exit 退出，clear 清屏（Windows）
            if (str == "exit") {
                return 0;
            }
            else if (str == "clear") {
                system("cls");
            }
            else {
                // 构造 expression（内部会校验表达式合法性，校验失败抛出异常）
                chr::expression expr(str);
                // 输出中缀表示（可读），后缀表示，以及两种计算方式的结果
                std::cout << "中缀解析：" << expr.infix_expression() << "\n";
                std::cout << "后缀解析：" << expr.postfix_expression() << "\n";
                std::cout << "中缀计算：" << expr.evaluate_from_infix() << "\n";
                std::cout << "后缀计算：" << expr.evaluate_from_postfix() << "\n";
            }
        }
        catch (std::runtime_error& e)
        {
            // 捕获并打印解析/计算时抛出的运行时错误（包含 tokenizer 提供的详细信息）
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}