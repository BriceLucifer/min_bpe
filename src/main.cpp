// #include "minbpe.hpp"
// // ---------------------------------------------
// // Demo main: 训练 + 编码 + 解码
// // ---------------------------------------------
// int main() {
//     BasicTokenizer tokenizer;

//     std::string text = "hello hello hell";  // 训练语料
//     std::size_t target_vocab_size = 300;    // 想要的最终词表大小上限

//     tokenizer.train(text, target_vocab_size);

//     // 编码新句子
//     std::string test_sentence = "hello hell";
//     auto encoded = tokenizer.encode(test_sentence);

//     std::cout << "Encoded IDs: ";
//     for (auto id : encoded) {
//         std::cout << id << " ";
//     }
//     std::cout << "\n";

//     // 解码回来
//     std::string decoded = tokenizer.decode(encoded);
//     std::cout << "Decoded back: " << decoded << "\n";

//     return 0;
// }
#include "minbpe.hpp"
#include <iostream>

int main() {
    BasicTokenizer tokenizer;

    std::string text = "hello hello hell";  // 训练语料
    std::size_t target_vocab_size = 300;    // 想要的最终词表大小上限

    tokenizer.train(text, target_vocab_size);

    // 打印学到的 merges（训练产物）
    tokenizer.print_merges();

    // 编码新句子，带 trace
    std::string test_sentence = "hello hell";

    std::cout << "\n--- ENCODE TRACE ---\n";
    auto encoded_trace = tokenizer.encode_with_trace(test_sentence);

    std::cout << "Encoded IDs (trace run result): ";
    for (auto id : encoded_trace) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    // 再用正常 encode（不打印），拿到同样的结果
    auto encoded = tokenizer.encode(test_sentence);
    std::cout << "Encoded IDs (quiet encode): ";
    for (auto id : encoded) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    // 解码回来
    std::string decoded = tokenizer.decode(encoded);
    std::cout << "Decoded back: " << decoded << "\n";

    return 0;
}
