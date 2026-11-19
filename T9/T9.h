// ============================================================================
//  题目九  二叉排序树
//  文件结构：常量 → 二叉树结点 BinaryNode<T> → 菜单 Menu
//            → 控制台输入 Input → 二叉排序树 BinarySortTree
//  数据结构：二叉链表（左子树都比根小、右子树都比根大）
//  复杂度：插入、查找平均 O(log n)（最坏 O(n)）；中序遍历 O(n)；空间 O(n)
//  说明：类与函数都定义在本文件里（成员函数直接在类里实现），T9.cpp 里只有主函数；
//        二叉树是自己写的，没有使用任何 STL 容器
// ============================================================================

#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>

// ============================== 常量 ==============================
const unsigned int kKeyLineLen = 500;     // 一次录入多个关键字时，一行最长 500 字节
const unsigned int kMaxKeyCount = 200;    // 一次最多录入 200 个关键字
const unsigned int kMenuWidth = 52;       // 菜单框宽度（与题目示例一致）
const int kKeyEndFlag = 0;                // 建立二叉排序树时，输入 0 表示结束

// ============================== 二叉树结点 ==============================
// 二叉树结点：数据 + 左、右孩子指针，新结点的孩子指针都置空
template <typename T>
struct BinaryNode {
  T data;
  BinaryNode<T>* left;
  BinaryNode<T>* right;
  explicit BinaryNode(const T& value) : data(value), left(nullptr), right(nullptr) {}
};

// ============================== 菜单 ==============================
// 统一打印“** 内容 **”样式的菜单框。汉字在 GBK 里占两个字节、显示两列，
// 因此按显示宽度（displayWidth）居中，菜单框的左右边框才能对齐。
class Menu {
 public:
  // 一行菜单：** ＋ 居中的 content ＋ **，整行共 width 列
  static void printLine(const char* content, const unsigned int width) {
    const unsigned int inner = width - 4;  // 去掉两侧的 **
    const unsigned int textWidth = displayWidth(content);
    const unsigned int left = textWidth >= inner ? 0 : (inner - textWidth) / 2;
    const unsigned int right = textWidth >= inner ? 0 : inner - textWidth - left;
    std::cout << "**";
    printSpaces(left);
    std::cout << content;
    printSpaces(right);
    std::cout << "**" << '\n';
  }

  // 一组选项：按最长的一行整体居中、各行左对齐，选项看起来才整齐
  static void printOptions(const char* const options[], const unsigned int count,
                           const unsigned int width) {
    unsigned int maxWidth = 0;
    for (unsigned int i = 0; i < count; i++) {
      const unsigned int textWidth = displayWidth(options[i]);
      if (textWidth > maxWidth) maxWidth = textWidth;
    }
    const unsigned int inner = width - 4;
    const unsigned int margin = maxWidth >= inner ? 0 : (inner - maxWidth) / 2;
    for (unsigned int i = 0; i < count; i++) {
      std::cout << "**";
      printSpaces(margin);
      std::cout << options[i];
      const unsigned int textWidth = displayWidth(options[i]);
      const unsigned int rest =
          inner > margin + textWidth ? inner - margin - textWidth : 0;
      printSpaces(rest);
      std::cout << "**" << '\n';
    }
  }

  // 分隔线：整行 width 个等号
  static void printSeparator(const unsigned int width) {
    for (unsigned int i = 0; i < width; i++) std::cout << '=';
    std::cout << '\n';
  }

  // 字符串在控制台里占的列数：GBK 汉字占 2 列，其余字符占 1 列
  static unsigned int displayWidth(const char* text) {
    unsigned int width = 0;
    unsigned int i = 0;
    while (text[i] != '\0') {
      if ((unsigned char)text[i] >= 0x80 && text[i + 1] != '\0') {
        width += 2;  // 双字节汉字
        i += 2;
      } else {
        width += 1;
        i += 1;
      }
    }
    return width;
  }

 private:
  static void printSpaces(const unsigned int count) {
    for (unsigned int i = 0; i < count; i++) std::cout << ' ';
  }
};

// ============================== 控制台输入 ==============================
class Input {
 public:
  // 读一个 [min, max] 范围内的整数，输入不合法会提示并要求重输
  static unsigned int readNumber(const char* prompt, const unsigned int min,
                                 const unsigned int max) {
    char line[kLineLen + 1];
    unsigned int value = 0;
    while (true) {
      std::cout << prompt;
      if (!readLine(line, kLineLen)) continue;
      if (!toUnsigned(line, value))
        std::cout << "输入不合法，请输入数字！" << '\n';
      else if (value < min || value > max)
        std::cout << "输入不合法，应输入 " << min << "~" << max << " 之间的数！"
                  << '\n';
      else
        return value;
    }
  }

  // 读一个不含空白的字段（姓名、报考类别）；out 的容量至少 maxLen + 1
  static void readWord(const char* prompt, const char* fieldName,
                       const unsigned int maxLen, char* out) {
    char line[kLineLen + 1];
    while (true) {
      std::cout << prompt;
      if (!readLine(line, kLineLen)) continue;

      // 掐掉首尾空白，得到 [begin, end)
      unsigned int begin = 0;
      unsigned int end = (unsigned int)strlen(line);
      while (begin < end && isBlank(line[begin])) begin++;
      while (end > begin && isBlank(line[end - 1])) end--;

      if (begin == end) {
        std::cout << "输入不合法，" << fieldName << "不能为空，请重新输入！"
                  << '\n';
      } else if (hasBlank(line + begin, end - begin)) {
        std::cout << "输入不合法，" << fieldName << "不能包含空格，请重新输入！"
                  << '\n';
      } else if (end - begin > maxLen) {
        std::cout << "输入不合法，" << fieldName << "最长 " << maxLen
                  << " 字节，请重新输入！" << '\n';
      } else {
        line[end] = '\0';
        strcpy(out, line + begin);
        return;
      }
    }
  }

  // 读一行并按空白切成若干字段：返回字段个数（0 表示空行）；
  // 返回 -1 表示这一行过长（提示已打印），调用者应重新读一次
  static int readFields(const char* prompt, char* line, char* fields[],
                        const unsigned int maxFields, const unsigned int cap) {
    std::cout << prompt;
    if (!readLine(line, cap)) return -1;
    return splitFields(line, fields, maxFields);
  }

  // 读一整行正文（允许空格、允许空行）：读成功返回 true，太长返回 false
  static bool readText(const char* prompt, char* out, const unsigned int cap) {
    std::cout << prompt;
    return readLine(out, cap);
  }

  // 连续录入正文时用：不打印提示，返回 false 表示输入结束（Ctrl+Z / 管道结束）
  static bool readBodyLine(char* out, const unsigned int cap) {
    while (true) {
      std::cin.getline(out, cap + 1);
      if (!std::cin.fail()) return true;
      if (std::cin.eof()) {
        std::cin.clear();
        return false;
      }
      std::cout << "输入过长，请重新输入！" << '\n';
      std::cin.clear();
      char c;
      while (std::cin.get(c) && c != '\n') {
      }
    }
  }

  // 读一个菜单字母：只接受 letters 里出现过的字母，大小写不敏感
  static char readLetter(const char* prompt, const char* letters) {
    char line[kLineLen + 1];
    while (true) {
      std::cout << prompt;
      if (!readLine(line, kLineLen)) continue;
      unsigned int begin = 0;
      unsigned int end = (unsigned int)strlen(line);
      while (begin < end && isBlank(line[begin])) begin++;
      while (end > begin && isBlank(line[end - 1])) end--;
      if (end - begin == 1) {
        char letter = line[begin];
        if (letter >= 'a' && letter <= 'z') letter = (char)(letter - 'a' + 'A');
        for (unsigned int i = 0; letters[i] != '\0'; i++)
          if (letters[i] == letter) return letter;
      }
      std::cout << "输入不合法，请输入 " << letters << " 中的一个字母！" << '\n';
    }
  }

  // 把字符串解析成 int：首尾空白忽略，逐位判界防止回绕
  static bool toInt(const char* text, int& out) {
    unsigned int i = 0;
    while (isBlank(text[i])) i++;
    bool negative = false;
    if (text[i] == '+' || text[i] == '-') {
      negative = (text[i] == '-');
      i++;
    }
    if (text[i] < '0' || text[i] > '9') return false;

    const unsigned long long kPositiveMax = 2147483647ULL;
    const unsigned long long kNegativeMax = 2147483648ULL;
    const unsigned long long limit = negative ? kNegativeMax : kPositiveMax;
    unsigned long long value = 0;
    while (text[i] >= '0' && text[i] <= '9') {
      const unsigned long long digit = (unsigned long long)(text[i] - '0');
      if (value > (limit - digit) / 10) return false;  // 再加一位就越界
      value = value * 10 + digit;
      i++;
    }
    while (isBlank(text[i])) i++;
    if (text[i] != '\0') return false;  // 数字后面还有别的东西

    if (!negative) {
      out = (int)value;
    } else if (value == kNegativeMax) {
      out = -2147483647 - 1;  // int 的最小值不能写成字面量 -2147483648
    } else {
      out = -(int)value;
    }
    return true;
  }

 private:
  static constexpr unsigned int kLineLen = 128;  // 一行最多读 128 个字符

  static bool isBlank(const char c) { return c == ' ' || c == '\t'; }

  static bool hasBlank(const char* text, const unsigned int len) {
    for (unsigned int i = 0; i < len; i++)
      if (isBlank(text[i])) return true;
    return false;
  }

  // 读一行；太长时提示并清掉本行剩余字符；输入结束（Ctrl+Z）直接退出程序
  static bool readLine(char* line, const unsigned int cap) {
    std::cin.getline(line, cap + 1);
    if (!std::cin.fail()) return true;
    if (std::cin.eof()) {
      std::cout << '\n' << "输入结束，程序退出!" << '\n';
      std::exit(0);
    }
    std::cout << "输入过长，请重新输入！" << '\n';
    std::cin.clear();
    char c;
    while (std::cin.get(c) && c != '\n') {
    }
    return false;
  }

  // 把一行按空白切成若干字段，返回字段个数；fields 依次指向各字段起点
  static int splitFields(char* line, char* fields[],
                         const unsigned int maxFields) {
    int count = 0;
    unsigned int i = 0;
    while (true) {
      while (isBlank(line[i])) i++;  // 跳过字段之间的空白
      if (line[i] == '\0') return count;
      if ((unsigned int)count == maxFields) return count + 1;  // 字段太多
      fields[count++] = line + i;  // 记录字段起点
      while (line[i] != '\0' && !isBlank(line[i])) i++;
      if (line[i] == '\0') return count;
      line[i++] = '\0';  // 字段结尾，继续找下一个字段
    }
  }

  // 把字符串解析成 unsigned int：首尾空白忽略，逐位判界防止回绕
  static bool toUnsigned(const char* text, unsigned int& out) {
    unsigned int i = 0;
    while (isBlank(text[i])) i++;
    if (text[i] == '\0') return false;  // 空输入

    const unsigned long long kUintMax = 0xFFFFFFFFULL;
    unsigned long long value = 0;
    while (text[i] >= '0' && text[i] <= '9') {
      const unsigned long long digit = (unsigned long long)(text[i] - '0');
      if (value > (kUintMax - digit) / 10) return false;  // 再加一位就越界
      value = value * 10 + digit;
      i++;
    }
    while (isBlank(text[i])) i++;
    if (text[i] != '\0') return false;  // 数字后面还有别的东西
    out = (unsigned int)value;
    return true;
  }
};

// ============================== 二叉排序树 ==============================
// 依次输入关键字建立二叉排序树：比根小的放左子树，比根大的放右子树，
// 查找时也沿着这条大小关系走，可以不断缩小查找范围。
class BinarySortTree {
 public:
  BinarySortTree() { root_ = nullptr; }

  ~BinarySortTree() { destroyTree(root_); }

  // ---- 每个操作一个成员函数，主函数里直接 tree.xxx() 调用 ----

  static void printMenu() {
    const char* const options[] = {
        "1 --- 建立二叉排序树", "2 --- 插入元素", "3 --- 查询元素", "4 --- 退出程序",
    };
    Menu::printLine("二叉排序树", kMenuWidth);
    Menu::printSeparator(kMenuWidth);
    Menu::printOptions(options, 4, kMenuWidth);
    Menu::printSeparator(kMenuWidth);
  }

  // 1：建立二叉排序树——读入一串以 0 结束的整数，逐个插入
  void createTree() {
    clearTree();
    std::cout << "Please input key to create Bsort_Tree:" << '\n';
    char line[kKeyLineLen + 1];
    char* fields[kMaxKeyCount + 2];
    while (true) {
      const int count = Input::readFields("", line, fields, kMaxKeyCount + 1, kKeyLineLen);
      if (count < 0) continue;  // 这一行太长
      if (count == 0) {
        std::cout << "输入不合法，请输入以 " << kKeyEndFlag << " 结束的整数序列！" << '\n';
        continue;
      }
      int keys[kMaxKeyCount];
      int keyCount = 0;
      bool terminated = false;
      bool valid = true;
      for (int i = 0; i < count; i++) {
        int value = 0;
        if (!Input::toInt(fields[i], value)) {
          valid = false;
          break;
        }
        if (value == kKeyEndFlag) {  // 读到 0，后面的内容不再处理
          terminated = true;
          break;
        }
        keys[keyCount++] = value;
      }
      if (!valid) {
        std::cout << "输入不合法，请输入整数！" << '\n';
        continue;
      }
      if (!terminated) {
        std::cout << "输入不合法，请输入以 " << kKeyEndFlag << " 结束的整数序列！" << '\n';
        continue;
      }
      for (int i = 0; i < keyCount; i++)
        if (!insert(keys[i]))
          std::cout << "The input key(" << keys[i] << ")is have in!" << '\n';
      printTree();
      return;
    }
  }

  // 2：插入一个元素
  void insertKey() {
    const int key = readKey("Please input key which inserted:\t");
    if (!insert(key)) std::cout << "The input key(" << key << ")is have in!" << '\n';
    printTree();
  }

  // 3：查询一个元素
  void searchKey() {
    const int key = readKey("Please input key which searched:\t");
    if (search(key))
      std::cout << "search success!" << '\n';
    else
      std::cout << key << " not exist!" << '\n';
  }

 private:
  // 读一个整数关键字，输入不是整数就重新输入
  int readKey(const char* prompt) {
    char line[kKeyLineLen + 1];
    char* fields[kMaxKeyCount + 2];
    while (true) {
      const int count = Input::readFields(prompt, line, fields, kMaxKeyCount + 1, kKeyLineLen);
      if (count < 0) continue;  // 这一行太长
      int key = 0;
      if (count >= 1 && Input::toInt(fields[0], key)) return key;
      std::cout << "输入不合法，请输入一个整数！" << '\n';
    }
  }

  // 插入：比根小往左走，比根大往右走；关键字已经存在返回 false
  bool insert(const int key) { return insertAt(root_, key); }

  bool insertAt(BinaryNode<int>*& node, const int key) {
    if (node == nullptr) {
      node = new BinaryNode<int>(key);
      return true;
    }
    if (key == node->data) return false;  // 已经有了，不重复插入
    if (key < node->data) return insertAt(node->left, key);
    return insertAt(node->right, key);
  }

  // 查找：沿着二叉排序树的大小关系一路向下，找到返回 true
  bool search(const int key) const {
    const BinaryNode<int>* current = root_;
    while (current != nullptr) {
      if (key == current->data) return true;
      current = key < current->data ? current->left : current->right;
    }
    return false;
  }

  // 中序遍历：输出的关键字是递增的，每个关键字后面跟一个箭头
  void printInOrder(const BinaryNode<int>* node) const {
    if (node == nullptr) return;
    printInOrder(node->left);
    std::cout << node->data << "->";
    printInOrder(node->right);
  }

  void printTree() const {
    std::cout << "Bsort_Tree is:" << '\n';
    printInOrder(root_);
    std::cout << '\n';
  }

  // 释放整棵二叉树，并把根置空
  void clearTree() {
    destroyTree(root_);
    root_ = nullptr;
  }

  void destroyTree(BinaryNode<int>* node) {
    if (node == nullptr) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
  }

  BinaryNode<int>* root_;  // 二叉排序树的根
};
