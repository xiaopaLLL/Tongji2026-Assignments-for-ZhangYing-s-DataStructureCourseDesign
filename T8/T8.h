// ============================================================================
//  题目八  电网建设造价模拟系统
//  文件结构：常量 → 哈希表 HashMap<K, V> → 菜单 Menu → 控制台输入 Input
//            → 电网 PowerGrid
//  数据结构：邻接矩阵（顶点之间的造价）＋ 哈希表（顶点名称 → 下标，平均 O(1)）
//  复杂度：加 n-1 条边 O(n²)；Prim 算法 O(n²)；显示最小生成树 O(n)；空间 O(n²)
//  说明：类与函数都定义在本文件里（成员函数直接在类里实现），T8.cpp 里只有主函数；
//        哈希表是自己写的，没有使用任何 STL 容器
// ============================================================================

#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>

// ============================== 常量 ==============================
const unsigned int kMinVertexCount = 2;      // 小区（顶点）个数下限
const unsigned int kMaxVertexCount = 20;     // 小区（顶点）个数上限
const unsigned int kVertexNameLen = 5;       // 顶点名称最多 5 字节
const unsigned int kMaxWeight = 10000;       // 一条线路的造价上限
const unsigned int kVertexLineLen = 200;     // 输入顶点名称时一行最长 200 字节
const unsigned int kEdgeLineLen = 100;       // 输入一条边时一行最长 100 字节
const unsigned int kMenuWidth = 52;          // 菜单框宽度（与题目示例一致）
const int kNoEdge = 0;                       // 邻接矩阵里的 0 表示两个小区之间没有线路

// ============================== 哈希表 ==============================
// 通用哈希表：开放定址 + 线性探测，装载因子超过 0.7 就扩容。
// 约定：K() 被当作"空槽"标记，所以键不能等于 K()
//（考号都 >= 1、报考类别都是非空字符串，都满足这个条件）。
// 查找与插入平均 O(1)：按考号判重、按报考类别分组统计都不会退化成平方级。
template <typename K, typename V>
class HashMap {
 public:
  HashMap() {
    cap_ = 16;
    size_ = 0;
    slots_ = new Entry[cap_]();
  }

  HashMap(const HashMap&) = delete;  // 内部有裸指针，禁止拷贝
  HashMap& operator=(const HashMap&) = delete;
  ~HashMap() { delete[] slots_; }

  void clear() {
    for (unsigned int i = 0; i < cap_; i++) slots_[i] = Entry();
    size_ = 0;
  }

  // 按关键字查找：找到返回值的指针（可以直接改），找不到返回 nullptr
  V* find(const K& key) {
    unsigned int i = hashValue(key) & (cap_ - 1);
    while (!isEmpty(slots_[i].key)) {
      if (slots_[i].key == key) return &slots_[i].value;
      i = (i + 1) & (cap_ - 1);
    }
    return nullptr;
  }

  // 插入新的键值对；键已经存在时什么都不做
  void insert(const K& key, const V& value) {
    if (isEmpty(key) || find(key) != nullptr) return;
    if ((size_ + 1) * 10 >= cap_ * 7) resize(cap_ * 2);
    place(key, value);
  }

  unsigned int size() const { return size_; }

 private:
  struct Entry {
    K key;
    V value;
  };

  static bool isEmpty(const K& key) { return key == K(); }

  // FNV-1a：逐字节混合，无填充字节的可平凡复制类型都能用
  static unsigned int hashValue(const K& key) {
    const unsigned char* bytes = (const unsigned char*)&key;
    unsigned int hash = 2166136261u;
    for (unsigned int i = 0; i < sizeof(K); i++)
      hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
  }

  void place(const K& key, const V& value) {
    unsigned int i = hashValue(key) & (cap_ - 1);
    while (!isEmpty(slots_[i].key)) i = (i + 1) & (cap_ - 1);
    slots_[i].key = key;
    slots_[i].value = value;
    size_++;
  }

  void resize(const unsigned int newCap) {
    Entry* old = slots_;
    const unsigned int oldCap = cap_;
    slots_ = new Entry[newCap]();
    cap_ = newCap;
    size_ = 0;
    for (unsigned int i = 0; i < oldCap; i++)
      if (!isEmpty(old[i].key)) place(old[i].key, old[i].value);
    delete[] old;
  }

  Entry* slots_;
  unsigned int cap_;
  unsigned int size_;
};

// 哈希集合：判重用，内部就是"哈希表 + 一个占位的值"
template <typename T>
class HashSet {
 public:
  bool contains(const T& key) { return map_.find(key) != nullptr; }
  void add(const T& key) { map_.insert(key, 0); }  // insert 会跳过已有的键
  void clear() { map_.clear(); }

 private:
  HashMap<T, unsigned int> map_;
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

// ============================== 电网 ==============================
// 顶点存名称，顶点之间的造价存邻接矩阵（对称矩阵，0 表示没有线路）。
// 构造最小生成树用 Prim 算法：从起始顶点开始，每次挑一条“连接已选顶点和未选顶点、
// 造价最低”的边，把新顶点并入已选集合，n 个顶点共选 n-1 条边。
class PowerGrid {
 public:
  PowerGrid() {
    vertexCount_ = 0;
    edgeCount_ = 0;
    treeWeight_ = 0;
    hasTree_ = false;
    for (unsigned int row = 0; row < kMaxVertexCount; row++) {
      names_[row][0] = '\0';
      for (unsigned int col = 0; col < kMaxVertexCount; col++) cost_[row][col] = kNoEdge;
    }
  }

  // ---- 每个操作一个成员函数，主函数里直接 grid.xxx() 调用 ----

  static void printMenu() {
    const char* const options[] = {
        "A --- 创建电网顶点", "B --- 添加电网的边", "C --- 构造最小生成树",
        "D --- 显示最小生成树", "E --- 退出    程序",
    };
    Menu::printLine("电网造价模拟系统", kMenuWidth);
    Menu::printSeparator(kMenuWidth);
    Menu::printOptions(options, 5, kMenuWidth);
    Menu::printSeparator(kMenuWidth);
  }

  // A：创建电网顶点——先读个数，再在一行里读入各顶点名称
  void createVertices() {
    vertexCount_ = Input::readNumber("请输入顶点的个数：", kMinVertexCount, kMaxVertexCount);
    std::cout << "请依次输入各顶点的名称：" << '\n';
    char line[kVertexLineLen + 1];
    char* fields[kMaxVertexCount + 2];
    while (true) {
      const int count = Input::readFields("", line, fields, kMaxVertexCount + 1, kVertexLineLen);
      if (count < 0) continue;  // 这一行太长
      if (count != (int)vertexCount_) {
        std::cout << "输入不合法，应有 " << vertexCount_ << " 个顶点名称，请重新输入！"
                  << '\n';
        continue;
      }
      bool valid = true;
      for (int i = 0; i < count && valid; i++) {
        if (strlen(fields[i]) > kVertexNameLen) {
          std::cout << "输入不合法，顶点名称最长 " << kVertexNameLen << " 字节，请重新输入！"
                    << '\n';
          valid = false;
          break;
        }
        for (int j = 0; j < i && valid; j++)
          if (strcmp(fields[i], fields[j]) == 0) {
            std::cout << "输入不合法，顶点名称" << fields[i] << "重复，请重新输入！"
                      << '\n';
            valid = false;
          }
      }
      if (!valid) continue;

      index_.clear();
      for (unsigned int i = 0; i < vertexCount_; i++) {
        strcpy(names_[i], fields[i]);
        NameKey key = NameKey();  // 先清零：哈希值是按整个键逐字节算的
        strcpy(key.name, names_[i]);
        index_.insert(key, i);
      }
      for (unsigned int row = 0; row < kMaxVertexCount; row++)
        for (unsigned int col = 0; col < kMaxVertexCount; col++)
          cost_[row][col] = kNoEdge;  // 顶点换了，原来的边全部作废
      hasTree_ = false;
      edgeCount_ = 0;
      treeWeight_ = 0;
      return;
    }
  }

  // B：添加电网的边——反复读“顶点 顶点 造价”，输入 ?? 结束
  void addEdges() {
    if (vertexCount_ == 0) {
      std::cout << "请先创建电网顶点！" << '\n';
      return;
    }
    char line[kEdgeLineLen + 1];
    char* fields[5];
    while (true) {
      const int count =
          Input::readFields("请输入两个顶点及边： ", line, fields, 4, kEdgeLineLen);
      if (count < 0) continue;  // 这一行太长
      if (count == 0) continue;  // 空行：重新输入
      if (strcmp(fields[0], "??") == 0) return;  // 输入 ?? 表示加边结束
      if (count != 3) {
        std::cout << "输入不合法，每行应有 3 个字段：顶点 顶点 造价！" << '\n';
        continue;
      }
      const int from = findVertex(fields[0]);
      const int to = findVertex(fields[1]);
      int weight = 0;
      if (from < 0 || to < 0) {
        std::cout << "输入不合法，顶点不存在，请重新输入！" << '\n';
        continue;
      }
      if (from == to) {
        std::cout << "输入不合法，两个顶点不能相同，请重新输入！" << '\n';
        continue;
      }
      if (!Input::toInt(fields[2], weight) || weight < 1 || weight > (int)kMaxWeight) {
        std::cout << "输入不合法，造价应为 1~" << kMaxWeight << " 之间的整数！" << '\n';
        continue;
      }
      cost_[from][to] = weight;  // 无向图：两个方向都记上
      cost_[to][from] = weight;
    }
  }

  // C：用 Prim 算法构造最小生成树
  void buildTree() {
    if (vertexCount_ == 0) {
      std::cout << "请先创建电网顶点！" << '\n';
      return;
    }
    const unsigned int start = readVertex("请输入起始顶点： ");
    bool visited[kMaxVertexCount];
    int lowCost[kMaxVertexCount];   // 未选顶点到已选集合的最小造价
    int parent[kMaxVertexCount];    // 取得最小造价时，已选集合里是哪个顶点
    for (unsigned int i = 0; i < vertexCount_; i++) {
      visited[i] = false;
      lowCost[i] = kNoEdge;
      parent[i] = -1;
    }
    visited[start] = true;
    for (unsigned int i = 0; i < vertexCount_; i++)
      if (cost_[start][i] != kNoEdge) {
        lowCost[i] = cost_[start][i];
        parent[i] = (int)start;
      }

    edgeCount_ = 0;
    treeWeight_ = 0;
    hasTree_ = false;
    for (unsigned int step = 1; step < vertexCount_; step++) {
      int best = -1;
      for (unsigned int i = 0; i < vertexCount_; i++)
        if (!visited[i] && lowCost[i] != kNoEdge &&
            (best < 0 || lowCost[i] < lowCost[best]))
          best = (int)i;
      if (best < 0) {  // 还有顶点接不上：电网不连通
        edgeCount_ = 0;   // 之前挑出来的边凑不成生成树，全部作废
        treeWeight_ = 0;
        std::cout << "电网不连通，无法生成最小生成树！" << '\n';
        return;
      }
      visited[best] = true;
      mstFrom_[edgeCount_] = (unsigned int)parent[best];
      mstTo_[edgeCount_] = (unsigned int)best;
      mstWeight_[edgeCount_] = lowCost[best];
      treeWeight_ += lowCost[best];
      edgeCount_++;
      for (unsigned int i = 0; i < vertexCount_; i++)
        if (!visited[i] && cost_[best][i] != kNoEdge &&
            (lowCost[i] == kNoEdge || cost_[best][i] < lowCost[i])) {
          lowCost[i] = cost_[best][i];
          parent[i] = best;
        }
    }
    hasTree_ = true;
    std::cout << "生成Prim最小生成树!" << '\n';
  }

  // D：显示最小生成树的各条边
  void printTree() const {
    if (!hasTree_) {
      std::cout << "请先构造最小生成树！" << '\n';
      return;
    }
    std::cout << "最小生成树的顶点及边为：" << '\n';
    for (unsigned int i = 0; i < edgeCount_; i++) {
      if (i > 0) std::cout << '\t';
      std::cout << names_[mstFrom_[i]] << "-(" << mstWeight_[i] << ")->"
                << names_[mstTo_[i]];
    }
    std::cout << '\n';
  }

 private:
  // 顶点名称做哈希表的键：整个键先清零再写字符串，保证哈希值稳定
  struct NameKey {
    char name[kVertexNameLen + 1];
    bool operator==(const NameKey& other) const {
      return strcmp(name, other.name) == 0;
    }
  };

  // 读一个已存在的顶点名称，返回它的下标
  unsigned int readVertex(const char* prompt) {
    char name[kVertexNameLen + 1];
    while (true) {
      Input::readWord(prompt, "顶点名称", kVertexNameLen, name);
      const int index = findVertex(name);
      if (index >= 0) return (unsigned int)index;
      std::cout << "输入不合法，顶点" << name << "不存在，请重新输入！" << '\n';
    }
  }

  // 按名称查下标：查哈希表，平均 O(1)；找不到返回 -1
  int findVertex(const char* name) {
    NameKey key = NameKey();  // 先清零：哈希值是按整个键逐字节算的
    strcpy(key.name, name);
    unsigned int* found = index_.find(key);
    return found == nullptr ? -1 : (int)*found;
  }

  char names_[kMaxVertexCount][kVertexNameLen + 1];  // 顶点名称
  int cost_[kMaxVertexCount][kMaxVertexCount];       // 邻接矩阵：0 表示没有线路
  unsigned int vertexCount_;                         // 顶点个数
  unsigned int mstFrom_[kMaxVertexCount];            // 最小生成树的边：起点
  unsigned int mstTo_[kMaxVertexCount];              // 最小生成树的边：终点
  int mstWeight_[kMaxVertexCount];                   // 最小生成树的边：造价
  unsigned int edgeCount_;                           // 最小生成树的边数
  long long treeWeight_;                             // 最小生成树的总造价
  bool hasTree_;                                     // 是否已经构造过最小生成树
  HashMap<NameKey, unsigned int> index_;             // 顶点名称 → 下标
};
