// ============================================================================
//  题目八  电网建设造价模拟系统 —— 主函数
// ============================================================================
#include "T8.h"

// ============================== 主函数 ==============================
int main() {
  PowerGrid grid;

  grid.printMenu();       // 打印菜单

  // 菜单循环：每次只打印提示语，操作本身都是 PowerGrid 的成员函数
  while (true) {
    std::cout << '\n' << "请选择操作：";
    char operation = Input::readLetter("", "ABCDE");
    if (operation == 'A') grid.createVertices();
    if (operation == 'B') grid.addEdges();
    if (operation == 'C') grid.buildTree();
    if (operation == 'D') grid.printTree();
    if (operation == 'E') break;  // E：退出程序
  }

  system("pause");

  return 0;
}
