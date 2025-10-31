// #include <cstddef>
// #include <iostream>
// #include <map>
// #include <string>
// #include <utility>
// #include <vector>

// // ---------------------------------------------
// // 基础类型别名
// // ---------------------------------------------

// using TokenID = int;

// // 表示一对连续 token (a,b)
// // BPE 的基本想法就是“把出现最频繁的 (a,b) 合成一个新的 token c”
// using IntPair = std::pair<TokenID, TokenID>;

// // 表示一个 token 在词表里的展开（扩展）形式
// // - 对基础 token 0..255：Expansion = {自身}，比如 {97}
// // - 对新学到的 token：Expansion = {a,b}，表示它由两个子 token 合成
// using Expansion = std::vector<TokenID>;

// // 一条合并规则：(pair.first, pair.second) -> idx
// // 例如：("l","l") -> 256
// class Merge {
// public:
//     IntPair pair;  // 要合并的那对 token
//     TokenID idx;   // 新产生的 token id
// };


// // ---------------------------------------------
// // BPE Tokenizer
// // ---------------------------------------------
// class BasicTokenizer {
// public:
//     static constexpr std::size_t INITIAL_VOCAB_SIZE = 256;

//     BasicTokenizer() {
//         std::cout << "Tokenizer is initialized.." << std::endl;

//         // 初始化词表 vocab：
//         // vocab[id] = {id} 对于 0..255
//         // 也就是最初每个 tokenID 只是单个字节自己本身
//         vocab.reserve(INITIAL_VOCAB_SIZE);
//         for (std::size_t t = 0; t < INITIAL_VOCAB_SIZE; ++t) {
//             vocab.push_back(Expansion{ static_cast<TokenID>(t) });
//         }
//     }

//     ~BasicTokenizer() {
//         std::cout << "BPE Tokenizer is cleaned" << std::endl;
//     }

//     /*
//      * train():
//      *
//      * 输入: 原始文本 text, 目标词表大小 target_vocab_size
//      *
//      * 工作流程（标准 BPE 训练）：
//      *   1. 把 text 按字节切成初始 token 序列 ids
//      *   2. 重复:
//      *        - 统计 ids 里所有相邻 pair 的出现次数
//      *        - 找出现次数最多的 pair = (a,b)
//      *        - 给它分配一个新的 token id = 当前 vocab.size()
//      *        - 在 ids 中把所有 (a,b) 合并成这个新 token
//      *        - 把这条规则 (a,b)->new_id 存到 merges
//      *        - 把 new_id 的展开 {a,b} 存到 vocab
//      *      直到 vocab.size() 达到目标，或者没有高频 pair 可以合并
//      */
//     void train(const std::string& text, std::size_t target_vocab_size) {
//         std::cout << "Training.." << std::endl;

//         // Step 1: 文本 -> 初始 token 序列（每个字节一个 TokenID）
//         // 例如 "hello" -> [104,101,108,108,111]
//         std::vector<TokenID> ids;
//         ids.reserve(text.size());
//         for (unsigned char c : text) {
//             ids.push_back(static_cast<TokenID>(c));
//         }

//         // 我们最多还能创建多少新的 token？
//         // budget = 想要的最终词表大小 - 当前已有大小
//         int budget = static_cast<int>(target_vocab_size) - static_cast<int>(vocab.size());
//         if (budget < 0) budget = 0;

//         // 我们循环 budget 次，每次学习一个新 token
//         for (int iter = 0; iter < budget; ++iter) {
//             // 2a. 数一数当前 ids 里所有相邻 pair 的频率
//             // counts[(a,b)] = 出现次数
//             auto counts = token_counts(ids);

//             // 2b. 找到出现频率最高的 pair
//             IntPair best_pair{-1, -1};
//             int best_freq = 0;
//             for (auto& [pair, freq] : counts) {
//                 if (freq > best_freq) {
//                     best_freq = freq;
//                     best_pair = pair;
//                 }
//             }

//             // 如果没有任何 pair 出现（或所有 pair 频率为 0）
//             // 那说明没法再继续合并了
//             if (best_freq == 0) {
//                 break;
//             }

//             // 2c. 给这个最常见的 pair 分配一个新的 token id
//             // 新 token 的 ID 就是 vocab 的当前大小
//             TokenID new_id = static_cast<TokenID>(vocab.size());

//             // 2d. 把 ids 中所有出现的 best_pair 全部合并成 new_id
//             merge_pairs(ids, best_pair, new_id);

//             // 2e. 把这个规则记录下来
//             merges.push_back(Merge{ best_pair, new_id });

//             // 2f. 在词表里登记新 token 的展开
//             // 例如：vocab[256] = {108,108} 表示 256 代表 "ll"
//             vocab.push_back(Expansion{ best_pair.first, best_pair.second });

//             // 继续下一轮：现在 ids 更短了，vocab 更大了
//         }
//     }

//     /*
//      * encode():
//      *
//      * 输入: 一段新文本
//      * 输出: 这段文本的 tokenID 序列（按已经学到的 merges 规则尽量合并）
//      *
//      * 流程：
//      *   1. 把文本按字节变成初始 token 列表
//      *   2. 循环：
//      *        - 数所有出现的相邻 pair
//      *        - 对每个 pair，看它是不是我们训练过的某条规则
//      *          （即在 merges[] 里有没有）
//      *        - 如果多个 pair 都是合法规则，选优先级最高的那条
//      *          （即 merges 的 index 最小的那条规则）
//      *        - 把这个 pair 全部合并成那个规则对应的新 token
//      *        - 重复直到没有任何可用规则
//      */
//     std::vector<TokenID> encode(const std::string& text) const {
//         // Step 1: 文本 -> 最原始 token 序列
//         std::vector<TokenID> ids;
//         ids.reserve(text.size());
//         for (unsigned char c : text) {
//             ids.push_back(static_cast<TokenID>(c));
//         }

//         // Step 2: 不断尝试应用已知的合并规则
//         while (ids.size() >= 2) {
//             auto counts = token_counts(ids);

//             // chosen_rule_idx 用来表示哪个规则最“优先”
//             // merges[0] 是第一条学到的规则，优先级最高
//             std::size_t chosen_rule_idx = merges.size(); // 相当于 "无穷大"
//             IntPair chosen_pair{-1, -1};

//             // 我们遍历当前文本中的每个相邻 pair
//             for (auto& [pair, _freq] : counts) {
//                 // 看这个 pair 有没有在 merges 中出现过
//                 std::size_t rule_idx = find_pair_index(pair);
//                 if (rule_idx < chosen_rule_idx) {
//                     chosen_rule_idx = rule_idx;
//                     chosen_pair = pair;
//                 }
//             }

//             // 如果没有任何 pair 能匹配我们学过的 merge 规则，那就停止
//             if (chosen_rule_idx == merges.size()) {
//                 break;
//             }

//             // 把该规则对应的新 token id 取出来
//             TokenID new_id = merges[chosen_rule_idx].idx;
//             // 把所有出现的 chosen_pair 合并成 new_id
//             merge_pairs(ids, chosen_pair, new_id);
//         }

//         return ids;
//     }

//     /*
//      * decode():
//      *
//      * 输入: 一个 tokenID 序列（一般来自 encode 的输出）
//      * 输出: 还原后的原始字符串
//      *
//      * 关键点：
//      *   - 对于基础 token (0~255)，它本身就是一个字节
//      *   - 对于新 token，比如 256，它在 vocab[256] = {97,108} 里存着“分解回两个子 token”
//      *     这些子 token 可能又是组合，所以我们递归展开，直到全都变回基础字节(<256)
//      */
//     std::string decode(const std::vector<TokenID>& ids) const {
//         // 我们先把所有 token 递归展开成字节序列
//         std::vector<unsigned char> bytes;
//         for (TokenID t : ids) {
//             expand_recursive(t, bytes);
//         }

//         // 然后把这些字节拼成 std::string 输出
//         return std::string(bytes.begin(), bytes.end());
//     }

// private:
//     // 查找某个 pair 是否是我们学过的 merge 规则
//     // 返回它在 merges 向量里的下标；如果没找到，返回 merges.size()
//     // 下标越小，优先级越高
//     std::size_t find_pair_index(const IntPair& pair) const {
//         for (std::size_t i = 0; i < merges.size(); ++i) {
//             if (merges[i].pair == pair) {
//                 return i;
//             }
//         }
//         return merges.size();
//     }

//     // 对当前 token 序列 ids 统计所有相邻 pair 的出现次数
//     // 返回一个 map: (a,b) -> freq
//     //
//     // 例子：
//     //   ids = [104,101,108,108,111]
//     //   pair: (104,101), (101,108), (108,108), (108,111)
//     //   counts[(108,108)] = 1, ...
//     std::map<IntPair, int> token_counts(const std::vector<TokenID>& ids) const {
//         std::map<IntPair, int> counts;
//         for (std::size_t i = 0; i + 1 < ids.size(); ++i) {
//             IntPair p{ ids[i], ids[i+1] };
//             counts[p] += 1;
//         }
//         return counts;
//     }

//     // 把整个序列 ids 里所有出现的 (pair.first, pair.second)
//     // 合并成一个新的 token new_id
//     //
//     // 例子：
//     //   ids = [104,101,108,108,111]
//     //   pair = (108,108), new_id = 256
//     //   结果: ids = [104,101,256,111]
//     void merge_pairs(std::vector<TokenID>& ids,
//                      const IntPair& pair,
//                      TokenID new_id) const {
//         std::vector<TokenID> out;
//         out.reserve(ids.size());

//         for (std::size_t i = 0; i < ids.size(); ++i) {
//             // 如果当前位置正好匹配 pair，就用 new_id 替换这俩元素
//             if (i + 1 < ids.size()
//              && ids[i]   == pair.first
//              && ids[i+1] == pair.second)
//             {
//                 out.push_back(new_id);
//                 ++i; // 跳过第二个
//             } else {
//                 // 否则照抄
//                 out.push_back(ids[i]);
//             }
//         }

//         ids.swap(out);
//     }

//     // 把一个 token 递归展开为基础字节，并追加到 bytes 里
//     // - 如果 t < 256，说明它就是原始字节，直接 push_back
//     // - 否则 vocab[t] = {a,b}，我们要继续展开 a 和 b
//     void expand_recursive(TokenID t, std::vector<unsigned char>& bytes) const {
//         if (t < static_cast<TokenID>(INITIAL_VOCAB_SIZE)) {
//             // base token: 它直接就是一个字节
//             bytes.push_back(static_cast<unsigned char>(t));
//             return;
//         }

//         // 否则它是个合成 token。查它的展开：
//         // vocab[t] 就是 {sub1, sub2}（两个子 token）
//         // 递归展开每个子 token，直到全都变回原始字节
//         const Expansion& exp = vocab[t];
//         for (TokenID child : exp) {
//             expand_recursive(child, bytes);
//         }
//     }

// private:
//     // merges 保存所有学到的合并规则
//     // merges[0] 是第一条学到的规则，也是优先级最高的
//     std::vector<Merge> merges;

//     // vocab[id] = Expansion
//     // - 对 0..255: Expansion{ id }
//     // - 对新 token: Expansion{ left_token, right_token }
//     //
//     // 注意：新 token 可能的组成部分也可能是别的新 token
//     // 所以 decode 的时候我们要递归地展开
//     std::vector<Expansion> vocab;
// };
#pragma once
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

// ---------------------------------------------
// 基础类型别名
// ---------------------------------------------

using TokenID = int;

// 一对连续的 token (a,b)
using IntPair   = std::pair<TokenID, TokenID>;

// 一个 token 的展开：
// - base token (<256): {自身}
// - 合成 token: {left,right} (也可能继续展开)
using Expansion = std::vector<TokenID>;

// 一条合并规则: (pair.first, pair.second) -> idx
class Merge {
public:
    IntPair pair;
    TokenID idx;
};


// ---------------------------------------------
// BPE Tokenizer
// ---------------------------------------------
class BasicTokenizer {
public:
    static constexpr std::size_t INITIAL_VOCAB_SIZE = 256;

    BasicTokenizer() {
        std::cout << "Tokenizer is initialized.." << std::endl;

        vocab.reserve(INITIAL_VOCAB_SIZE);
        for (std::size_t t = 0; t < INITIAL_VOCAB_SIZE; ++t) {
            vocab.push_back(Expansion{ static_cast<TokenID>(t) });
        }
    }

    ~BasicTokenizer() {
        std::cout << "BPE Tokenizer is cleaned" << std::endl;
    }

    /*
     * train():
     *   1. text -> byte-level token 序列 ids
     *   2. 重复:
     *        - 数相邻 pair 频率
     *        - 取频率最高的 pair = best_pair
     *        - new_id = vocab.size()
     *        - 把 best_pair 全部合并成 new_id
     *        - merges.push_back({best_pair, new_id})
     *        - vocab.push_back({best_pair.first, best_pair.second})
     *      直到 budget 用完或没有可合并 pair
     */
    void train(const std::string& text, std::size_t target_vocab_size) {
        std::cout << "Training..\n";

        // 1. 文本 -> 初始 ids（逐字节）
        std::vector<TokenID> ids;
        ids.reserve(text.size());
        for (unsigned char c : text) {
            ids.push_back(static_cast<TokenID>(c));
        }

        // 2. 我们还能造多少新 token？
        int budget = static_cast<int>(target_vocab_size) - static_cast<int>(vocab.size());
        if (budget < 0) budget = 0;

        for (int iter = 0; iter < budget; ++iter) {
            // 2a. 统计相邻 pair 频率
            auto counts = token_counts(ids);

            // 2b. 找最常见的 pair
            IntPair best_pair{-1, -1};
            int best_freq = 0;
            for (auto& [pair, freq] : counts) {
                if (freq > best_freq) {
                    best_freq = freq;
                    best_pair = pair;
                }
            }

            // 没有出现过的 pair，就停
            if (best_freq == 0) {
                break;
            }

            // 2c. 给这对 pair 分配新 token id
            TokenID new_id = static_cast<TokenID>(vocab.size());

            // 2d. 在 ids 里把所有 best_pair 合并成 new_id
            merge_pairs(ids, best_pair, new_id);

            // 2e. 记录合并规则
            merges.push_back(Merge{ best_pair, new_id });

            // 2f. 记录词表展开
            vocab.push_back(Expansion{ best_pair.first, best_pair.second });
        }
    }

    /*
     * encode():
     *   把任意新文本转成 token 序列，尽量合并到最粗粒度
     *   （优先使用训练时最早学到的规则）
     */
    std::vector<TokenID> encode(const std::string& text) const {
        std::vector<TokenID> ids;
        ids.reserve(text.size());
        for (unsigned char c : text) {
            ids.push_back(static_cast<TokenID>(c));
        }

        apply_merges_until_stable(ids, /*trace=*/false);
        return ids;
    }

    /*
     * encode_with_trace():
     *   和 encode() 一样，但会打印出每一轮合并细节：
     *     - 当前序列
     *     - 选择了哪条 merge 规则
     *     - 合并后序列
     */
    std::vector<TokenID> encode_with_trace(const std::string& text) const {
        std::vector<TokenID> ids;
        ids.reserve(text.size());
        for (unsigned char c : text) {
            ids.push_back(static_cast<TokenID>(c));
        }

        apply_merges_until_stable(ids, /*trace=*/true);
        return ids;
    }

    /*
     * decode():
     *   把 token 序列还原成字符串
     *   做法：递归展开每个 token，直到全是基础 byte，再拼成 string
     */
    std::string decode(const std::vector<TokenID>& ids) const {
        std::vector<unsigned char> bytes;
        for (TokenID t : ids) {
            expand_recursive(t, bytes);
        }
        return std::string(bytes.begin(), bytes.end());
    }

    /*
     * print_merges():
     *   打印所有学到的 merge 规则以及对应 token 的展开
     *   merges[i] 越小，表示越早学到 -> 优先级越高
     */
    void print_merges() const {
        std::cout << "=== Learned merges (" << merges.size() << ") ===\n";
        for (std::size_t i = 0; i < merges.size(); ++i) {
            const auto& m = merges[i];
            std::cout << "#" << i
                      << "  (" << m.pair.first
                      << ","  << m.pair.second
                      << ") -> " << m.idx;

            // 打印它在 vocab 里的展开
            if (m.idx < vocab.size()) {
                std::cout << "   vocab[" << m.idx << "] = {";
                const auto& exp = vocab[m.idx];
                for (std::size_t k = 0; k < exp.size(); ++k) {
                    std::cout << exp[k];
                    if (k + 1 < exp.size()) std::cout << ",";
                }
                std::cout << "}";
            }

            std::cout << "\n";
        }
        std::cout << "===============================\n";
    }

private:
    // 对当前 ids 做一轮一轮的合并，直到没有可用规则
    void apply_merges_until_stable(std::vector<TokenID>& ids, bool trace) const {
        // trace == true 时，我们会打印每一轮状态
        int step = 0;
        while (ids.size() >= 2) {
            // 统计当前 pair
            auto counts = token_counts(ids);

            // 找“我们能用的规则”里，优先级最高的那一个
            std::size_t chosen_rule_idx = merges.size(); // 初始为"无"
            IntPair chosen_pair{-1, -1};

            for (auto& [pair, _freq] : counts) {
                std::size_t rule_idx = find_pair_index(pair);
                if (rule_idx < chosen_rule_idx) {
                    chosen_rule_idx = rule_idx;
                    chosen_pair = pair;
                }
            }

            // 如果没有任何 pair 匹配我们学过的 merges，就停
            if (chosen_rule_idx == merges.size()) {
                if (trace) {
                    std::cout << "[encode step " << step << "] no more merges.\n";
                }
                break;
            }

            // 合并之前，打印当前 ids
            if (trace) {
                std::cout << "[encode step " << step << "] ids before: ";
                print_ids(ids);
            }

            // 取到对应的新 token id
            TokenID new_id = merges[chosen_rule_idx].idx;

            if (trace) {
                std::cout << "  applying rule #" << chosen_rule_idx
                          << ": (" << chosen_pair.first
                          << ","  << chosen_pair.second
                          << ") -> " << new_id
                          << "\n";
            }

            // 应用这条合并：把 (a,b) 全部替换成 new_id
            merge_pairs(ids, chosen_pair, new_id);

            // 打印合并后的 ids
            if (trace) {
                std::cout << "  ids after : ";
                print_ids(ids);
                std::cout << "\n";
            }

            ++step;
        }

        if (trace) {
            std::cout << "[encode done] final ids: ";
            print_ids(ids);
            std::cout << "\n";
        }
    }

    // 打印一个 token 序列（调试用）
    void print_ids(const std::vector<TokenID>& ids) const {
        std::cout << "[";
        for (std::size_t i = 0; i < ids.size(); ++i) {
            std::cout << ids[i];
            if (i + 1 < ids.size()) std::cout << " ";
        }
        std::cout << "]";
    }

    // 查找 pair 在 merges 里的位置；找不到则返回 merges.size()
    // 下标越小，优先级越高
    std::size_t find_pair_index(const IntPair& pair) const {
        for (std::size_t i = 0; i < merges.size(); ++i) {
            if (merges[i].pair == pair) {
                return i;
            }
        }
        return merges.size();
    }

    // 统计 ids 中所有相邻 pair 的出现次数: (ids[i], ids[i+1]) → 频率
    std::map<IntPair, int> token_counts(const std::vector<TokenID>& ids) const {
        std::map<IntPair, int> counts;
        for (std::size_t i = 0; i + 1 < ids.size(); ++i) {
            counts[{ ids[i], ids[i+1] }] += 1;
        }
        return counts;
    }

    // 把 ids 里所有出现的 (pair.first, pair.second) 合并成 new_id
    void merge_pairs(std::vector<TokenID>& ids,
                     const IntPair& pair,
                     TokenID new_id) const {
        std::vector<TokenID> out;
        out.reserve(ids.size());

        for (std::size_t i = 0; i < ids.size(); ++i) {
            if (i + 1 < ids.size()
             && ids[i]   == pair.first
             && ids[i+1] == pair.second)
            {
                out.push_back(new_id);
                ++i; // 跳过下一个
            } else {
                out.push_back(ids[i]);
            }
        }

        ids.swap(out);
    }

    // 把 token t 递归展开成基础字节，追加进 bytes
    void expand_recursive(TokenID t,
                          std::vector<unsigned char>& bytes) const {
        if (t < static_cast<TokenID>(INITIAL_VOCAB_SIZE)) {
            // 基础 token：就是一个字节
            bytes.push_back(static_cast<unsigned char>(t));
            return;
        }

        // 合成 token: vocab[t] = {sub_a, sub_b}
        const Expansion& exp = vocab[t];
        for (TokenID child : exp) {
            expand_recursive(child, bytes);
        }
    }

private:
    // 学到的合并规则（顺序 = 优先级）
    // merges[0] 最早学到，优先级最高
    std::vector<Merge> merges;

    // 词表
    // vocab[id] = 展开
    //   - 对 0..255: {id}
    //   - 对新 token: {left, right}
    std::vector<Expansion> vocab;
};
