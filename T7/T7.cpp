// ============================================================================
//  题目七  表达式计算 —— 主函数
// ============================================================================
#include "T7.h"

// ============================== 主函数 ==============================
int main() {
  Expression expression;

  expression.readExpression();   // 读入表达式并做完整的语法校验

  if (!expression.toPostfix() || !expression.buildTree()) {
    std::cout << "表达式太复杂，无法建立表达式二叉树！" << '\n';
    system("pause");
    return 0;
  }

  std::cout << "波兰表达式：\t";
  expression.printPrefix();      // 前序遍历 → 波兰表达式
  std::cout << '\n';
  std::cout << "中缀表达式：\t";
  expression.printInfix();       // 中序遍历 → 中缀表达式
  std::cout << '\n';
  std::cout << "逆波兰表达式：\t";
  expression.printPostfix();     // 后序遍历 → 逆波兰表达式
  std::cout << '\n';

  system("pause");

  return 0;
}
