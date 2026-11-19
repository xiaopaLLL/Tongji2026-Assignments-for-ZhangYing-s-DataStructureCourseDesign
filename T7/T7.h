// ============================================================================
//  题目七  表达式计算
//  文件结构：常量 → 顺序栈 Stack<T> → 二叉树结点 BinaryNode<T>
//            → 模板二叉树 BinaryTree<T> → 控制台输入 Input
//            → 单词 ExprToken → 表达式 Expression
//  数据结构：后缀表达式数组＋运算符栈（中缀转后缀）＋表达式二叉树（二叉链表）
//  复杂度：中缀转后缀 O(n)；由后缀建树 O(n)；三种遍历各 O(n)；空间 O(n)
//  说明：类与函数都定义在本文件里（成员函数直接在类里实现），T7.cpp 里只有主函数；
//        栈和二叉树都是自己写的，没有使用任何 STL 容器；
//        BinaryTree<T> 与具体数据类型无关，只负责“结点归谁管”和“怎么遍历”，
// ============================================================================

#include <cstdlib>
#include <cstring>
#include <iostream>

// ============================== 常量 ==============================
const unsigned int kMaxExpressionLen = 100;   // 表达式最长 100 个字符
const unsigned int kMaxTokenCount = 200;      // 表达式最多 200 个单词
const unsigned int kTokenLen = 9;             // 一个操作数最多 9 位（不超过 int 范围）
const int kMaxPriority = 3;                   // 操作数当作最高优先级，中序遍历时不用加括号

// 切分表达式的返回码
const int kOk = 0;              // 没有错误
const int kErrorEmpty = 1;      // 空表达式
const int kErrorTooLong = 2;    // 操作数太长
const int kErrorChar = 3;       // 出现了非法字符
const int kErrorBracket = 4;    // 括号不配对
const int kErrorOrder = 5;      // 运算符与操作数的位置不对

// ============================== 顺序栈 ==============================
// 顺序栈：容量固定，后进先出；data_[0] 是栈底，size_ 是栈内元素个数。
// 栈里只存下标、方向或结点指针这类小数据，容量不足时 push 返回 false。
template <typename T>
class Stack {
 public:
  explicit Stack(const unsigned int cap) {
    cap_ = cap < 1 ? 1 : cap;  // 至少留 1 个位置，容量算错时栈也还能用
    data_ = new T[cap_];
    size_ = 0;
  }

  Stack(const Stack&) = delete;  // 内部有裸指针，禁止拷贝
  Stack& operator=(const Stack&) = delete;
  ~Stack() { delete[] data_; }

  bool empty() const { return size_ == 0; }
  unsigned int size() const { return size_; }

  bool push(const T& value) {
    if (size_ == cap_) return false;  // 栈满
    data_[size_] = value;
    size_++;
    return true;
  }

  // 出栈：把栈顶元素取到 out 里；栈空返回 false
  bool pop(T& out) {
    if (size_ == 0) return false;
    size_--;
    out = data_[size_];
    return true;
  }

  // 栈顶元素（可读可写）；由调用者保证栈非空
  T& top() { return data_[size_ - 1]; }
  const T& top() const { return data_[size_ - 1]; }

  // 从栈底开始按下标访问（输出整条路径时用）；由调用者保证下标合法
  const T& at(const unsigned int index) const { return data_[index]; }

  void clear() { size_ = 0; }

 private:
  T* data_;
  unsigned int cap_;
  unsigned int size_;
};

// ============================== 二叉树结点 ==============================
// 二叉树结点：数据 + 左、右孩子指针，新结点的孩子指针都置空
template <typename T>
struct BinaryNode {
  T data;
  BinaryNode<T>* left;
  BinaryNode<T>* right;
  explicit BinaryNode(const T& value) : data(value), left(nullptr), right(nullptr) {}
};

// ============================== 遍历时刻 ==============================
// 一次深度优先遍历经过一个结点时会先后处于三个时刻：
//   kEnterNode       刚进入结点（还没访问左子树）——前序就输出在这里；
//   kBetweenChildren 左子树访问完、右子树还没开始——中序就输出在这里；
//   kLeaveNode       两个孩子都访问完、正要离开——后序就输出在这里。
// 于是“三种遍历”只是同一个遍历在三个时刻的不同用法，不必各写一套递归。
enum TreePhase { kEnterNode, kBetweenChildren, kLeaveNode };

// ============================== 模板二叉树 ==============================
// 二叉链表实现的通用二叉树：只管两件事——结点归谁管、树怎么走。
//   buildFromPostfix：由后缀式建树（操作数建结点入栈，运算符弹出两个孩子）；
//   walk：整棵树做一次深度优先遍历，在每个时刻回调访问器；
//   clear / 析构：递归释放整棵树。
// 所有结点都由本类 new/delete，调用者不再直接管理结点内存，也就不可能漏删。
template <typename T>
class BinaryTree {
 public:
  BinaryTree() : root_(nullptr) {}

  BinaryTree(const BinaryTree&) = delete;  // 独占结点，禁止拷贝
  BinaryTree& operator=(const BinaryTree&) = delete;

  ~BinaryTree() { clear(); }

  bool empty() const { return root_ == nullptr; }

  void clear() {
    destroy(root_);
    root_ = nullptr;
  }

  // 由后缀式建树：data[0..count) 依次处理，isOperator(token) 为真表示它是运算符。
  // 后缀式不合法（运算符缺少操作数、最后不是恰好剩一棵树）时，把已经建好的
  // 结点全部释放并返回 false，保证不泄漏。
  template <typename IsOperator>
  bool buildFromPostfix(const T* data, const unsigned int count, IsOperator isOperator) {
    clear();
    Stack<BinaryNode<T>*> pending(count);  // 待接走的子树；每次循环最多压入一个
    for (unsigned int i = 0; i < count; i++) {
      BinaryNode<T>* node = new BinaryNode<T>(data[i]);
      if (isOperator(data[i])) {
        if (pending.size() < 2) {  // 运算符至少要有两个操作数
          destroy(node);
          clearPending(pending);
          return false;
        }
        BinaryNode<T>* right = nullptr;
        BinaryNode<T>* left = nullptr;
        pending.pop(right);  // 先出栈的是右孩子
        pending.pop(left);
        node->left = left;
        node->right = right;
      }
      if (!pending.push(node)) {  // 容量就是单词上限，正常不会满
        destroy(node);
        clearPending(pending);
        return false;
      }
    }
    if (!pending.pop(root_)) return false;  // 空后缀式
    if (!pending.empty()) {                 // 还剩结点说明后缀式不合法
      clearPending(pending);
      clear();
      return false;
    }
    return true;
  }

  // 深度优先遍历整棵树。访问器实现：
  //   void operator()(const BinaryNode<T>* node, const BinaryNode<T>* parent,
  //                   bool isRightChild, TreePhase phase)
  // parent 为 nullptr 表示根结点；isRightChild 表示 node 是 parent 的右孩子。
  template <typename Visitor>
  void walk(Visitor& visit) const {
    walkNode(root_, nullptr, false, visit);
  }

 private:
  template <typename Visitor>
  static void walkNode(const BinaryNode<T>* node, const BinaryNode<T>* parent,
                       const bool isRightChild, Visitor& visit) {
    if (node == nullptr) return;
    visit(node, parent, isRightChild, kEnterNode);
    walkNode(node->left, node, false, visit);
    visit(node, parent, isRightChild, kBetweenChildren);
    walkNode(node->right, node, true, visit);
    visit(node, parent, isRightChild, kLeaveNode);
  }

  // 递归释放一棵子树
  static void destroy(BinaryNode<T>* node) {
    if (node == nullptr) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
  }

  // 释放栈里还没被接走的那些子树
  static void clearPending(Stack<BinaryNode<T>*>& pending) {
    BinaryNode<T>* node = nullptr;
    while (pending.pop(node)) destroy(node);
  }

  BinaryNode<T>* root_;
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

// ============================== 单词 ==============================
// 表达式里的一个单词：既可以是操作数（一串数字），也可以是运算符（+ - * /）
struct ExprToken {
  char text[kTokenLen + 1];
  bool isOperator;
};

// ============================== 表达式 ==============================
// 处理过程与题目要求一致：先把中缀表达式转换成后缀表达式（逆波兰式），
// 再由后缀表达式建立表达式二叉树，最后用三种遍历分别输出三种表达式。
class Expression {
 public:
  Expression() : tokenCount_(0), postfixCount_(0) {}

  // ---- 每个操作一个成员函数，主函数里直接 expression.xxx() 调用 ----

  // 读一个语法正确的表达式，不合法就说明原因并要求重新输入
  void readExpression() {
    char line[kMaxExpressionLen + 1];
    while (true) {
      Input::readText("请输入表达式：\t", line, kMaxExpressionLen);
      const int code = tokenize(line);
      if (code == kErrorEmpty) {
        std::cout << "输入不合法，表达式不能为空，请重新输入！" << '\n';
        continue;
      }
      if (code == kErrorTooLong) {
        std::cout << "输入不合法，一个操作数最多 " << kTokenLen
                  << " 位数字，请重新输入！" << '\n';
        continue;
      }
      if (code == kErrorChar) {
        std::cout << "输入不合法，表达式只能包含数字、+ - * / 和括号，请重新输入！"
                  << '\n';
        continue;
      }
      const int sequence = checkSequence();
      if (sequence == kErrorBracket) {
        std::cout << "输入不合法，括号不配对，请重新输入！" << '\n';
        continue;
      }
      if (sequence == kErrorOrder) {
        std::cout << "输入不合法，运算符与操作数的位置不对，请重新输入！" << '\n';
        continue;
      }
      return;
    }
  }

  // 中缀转后缀：运算符栈 + 输出序列，遇到优先级不低于栈顶的运算符就先弹出栈顶
  bool toPostfix() {
    Stack<ExprToken> operators(kMaxTokenCount);
    postfixCount_ = 0;
    for (unsigned int i = 0; i < tokenCount_; i++) {
      const ExprToken& token = tokens_[i];
      if (!token.isOperator) {
        postfix_[postfixCount_++] = token;
        continue;
      }
      if (token.text[0] == '(') {
        if (!operators.push(token)) return false;
        continue;
      }
      if (token.text[0] == ')') {
        ExprToken top;
        while (!operators.empty() && operators.top().text[0] != '(') {
          operators.pop(top);
          postfix_[postfixCount_++] = top;
        }
        operators.pop(top);  // 弹出左括号，括号本身不进后缀表达式
        continue;
      }
      ExprToken top;
      while (!operators.empty() && operators.top().text[0] != '(' &&
             priority(operators.top()) >= priority(token)) {
        operators.pop(top);
        postfix_[postfixCount_++] = top;
      }
      if (!operators.push(token)) return false;
    }
    ExprToken top;
    while (operators.pop(top)) postfix_[postfixCount_++] = top;
    return postfixCount_ > 0;
  }

  // 由后缀表达式建立表达式二叉树：建树交给 BinaryTree，结点由它统一管理，
  // 本类不再直接 new/delete 任何结点
  bool buildTree() {
    return tree_.buildFromPostfix(postfix_, postfixCount_, isOperator);
  }

  // 前序遍历 → 波兰表达式（前缀表达式）：进入结点时输出
  void printPrefix() const {
    TokenPrinter printer(kEnterNode);
    tree_.walk(printer);
  }

  // 中序遍历 → 中缀表达式（必要时补括号，保证含义不变）：孩子之间输出
  void printInfix() const {
    InfixPrinter printer;
    tree_.walk(printer);
  }

  // 后序遍历 → 逆波兰表达式（后缀表达式）：离开结点时输出
  void printPostfix() const {
    TokenPrinter printer(kLeaveNode);
    tree_.walk(printer);
  }

 private:
  // 前序、后序输出共用同一个访问器：只在指定的那个时刻输出单词本身。
  // 前序＝进入时输出，后序＝离开时输出，所以两种表达式不需要两套代码。
  class TokenPrinter {
   public:
    explicit TokenPrinter(const TreePhase phase) : phase_(phase) {}

    void operator()(const BinaryNode<ExprToken>* node, const BinaryNode<ExprToken>*,
                    const bool, const TreePhase phase) {
      if (phase == phase_) std::cout << node->data.text;
    }

   private:
    TreePhase phase_;  // 在哪个时刻输出
  };

  // 中缀输出的访问器：进入时补左括号，两个孩子之间输出单词，离开时补右括号
  class InfixPrinter {
   public:
    void operator()(const BinaryNode<ExprToken>* node,
                    const BinaryNode<ExprToken>* parent, const bool isRightChild,
                    const TreePhase phase) {
      const bool needParen = needParenthesis(node, parent, isRightChild);
      if (phase == kEnterNode) {
        if (needParen) std::cout << '(';
      } else if (phase == kBetweenChildren) {
        std::cout << node->data.text;
      } else if (needParen) {
        std::cout << ')';
      }
    }

   private:
    // 子表达式的优先级低于父亲时加一对括号；减号、除号右边同优先级的子表达式
    // 也必须加括号（a-(b-c)、a/(b/c)），保证含义不变。根结点与操作数不加括号。
    static bool needParenthesis(const BinaryNode<ExprToken>* node,
                                const BinaryNode<ExprToken>* parent,
                                const bool isRightChild) {
      if (parent == nullptr || !node->data.isOperator) return false;
      const int myPriority = Expression::priority(node->data);
      const int parentPriority = parent->data.isOperator
                                     ? Expression::priority(parent->data)
                                     : kMaxPriority;
      if (myPriority < parentPriority) return true;
      const char parentOperator = parent->data.text[0];
      return isRightChild && myPriority == parentPriority &&
             (parentOperator == '-' || parentOperator == '/');
    }
  };

  // 把一行字符串切成单词：数字（可多位）和 + - * / ( ) 都算一个单词
  int tokenize(const char* line) {
    tokenCount_ = 0;
    unsigned int i = 0;
    while (line[i] != '\0') {
      const char c = line[i];
      if (c == ' ' || c == '\t') {
        i++;
        continue;
      }
      if (c >= '0' && c <= '9') {
        unsigned int length = 0;
        char text[kTokenLen + 1];
        while (line[i] >= '0' && line[i] <= '9') {
          if (length >= kTokenLen) return kErrorTooLong;
          text[length++] = line[i++];
        }
        text[length] = '\0';
        if (tokenCount_ >= kMaxTokenCount) return kErrorChar;
        tokens_[tokenCount_].isOperator = false;
        strcpy(tokens_[tokenCount_].text, text);
        tokenCount_++;
        continue;
      }
      if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')') {
        if (tokenCount_ >= kMaxTokenCount) return kErrorChar;
        tokens_[tokenCount_].isOperator = true;
        tokens_[tokenCount_].text[0] = c;
        tokens_[tokenCount_].text[1] = '\0';
        tokenCount_++;
        i++;
        continue;
      }
      return kErrorChar;  // 出现了不该有的字符
    }
    return tokenCount_ == 0 ? kErrorEmpty : kOk;
  }

  // 检查单词序列：操作数与运算符必须交替出现，括号必须配对
  int checkSequence() const {
    int depth = 0;
    bool expectOperand = true;  // 下一个位置应该是操作数（或左括号）
    for (unsigned int i = 0; i < tokenCount_; i++) {
      const ExprToken& token = tokens_[i];
      if (expectOperand) {
        if (!token.isOperator) {
          expectOperand = false;
        } else if (token.text[0] == '(') {
          depth++;
        } else {
          return kErrorOrder;  // 运算符、右括号出现在操作数的位置上
        }
      } else {
        if (!token.isOperator) return kErrorOrder;  // 两个操作数挨在一起
        if (token.text[0] == '(') return kErrorOrder;  // 操作数后面不能有左括号
        if (token.text[0] == ')') {
          depth--;
          if (depth < 0) return kErrorBracket;
        } else {
          expectOperand = true;
        }
      }
    }
    if (expectOperand) return kErrorOrder;  // 结尾是运算符
    if (depth != 0) return kErrorBracket;
    return kOk;
  }

  // 运算符的优先级：乘除高于加减，括号最低
  static int priority(const ExprToken& token) {
    if (token.text[0] == '+' || token.text[0] == '-') return 1;
    if (token.text[0] == '*' || token.text[0] == '/') return 2;
    return 0;
  }

  // 一个单词是不是运算符：BinaryTree 建树时回调它
  static bool isOperator(const ExprToken& token) { return token.isOperator; }

  ExprToken tokens_[kMaxTokenCount];   // 切分后的单词
  ExprToken postfix_[kMaxTokenCount];  // 后缀表达式（括号不进后缀，所以不会更短）
  unsigned int tokenCount_;
  unsigned int postfixCount_;
  BinaryTree<ExprToken> tree_;  // 表达式二叉树：结点由 BinaryTree 负责申请与释放
};
