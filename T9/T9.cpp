// ============================================================================
//  题目九  二叉排序树 —— 主函数
// ============================================================================
#include "T9.h"

// ============================== 主函数 ==============================
int main() {
  BinarySortTree tree;

  tree.printMenu();       // 打印菜单

  // 菜单循环：每次只打印提示语，操作本身都是 BinarySortTree 的成员函数
  while (true) {
    std::cout << '\n' << "Please select: ";
    unsigned int choice = Input::readNumber("", 1, 4);
    if (choice == 1) tree.createTree();
    if (choice == 2) tree.insertKey();
    if (choice == 3) tree.searchKey();
    if (choice == 4) break;  // 4：退出程序
  }

  system("pause");

  return 0;
}
