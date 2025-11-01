#include "chrexpression.h"

int main()
{
    std::string expr;
    while (1) {
        try
        {
            std::cout << "输入表达式：";
            std::getline(std::cin, expr);
            if (expr == "exit") {
                return 0;
            }
            else if (expr == "clear") {
                system("cls");
            }
            else {
                chr::infix_expression infix_expr(expr);
                chr::postfix_expression postfix_expr(expr);
                std::cout << "中缀解析：" << infix_expr << "\n";
                std::cout << "后缀解析：" << postfix_expr << "\n";
                std::cout << "中缀计算：" << infix_expr.evaluate() << "\n";
                std::cout << "后缀计算：" << postfix_expr.evaluate() << "\n";
            }
        }
        catch (std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    return 0;
}