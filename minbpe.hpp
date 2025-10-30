
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using TokenID = int;
using IntPair = std::pair<TokenID, TokenID>;
using Expansion = std::vector<TokenID>;

class Merge {
    public:
    IntPair pair;
    TokenID idx;
};

// rule one size_t for size lenth
class BasicTokenizer {
    public:
    // 初始化vocab_size
    static constexpr std::size_t INITIAL_VOCAB_SIZE = 256;
    BasicTokenizer(){
        std::cout << "Tokenizer is initialized.." << std::endl;
        vocab.reserve(INITIAL_VOCAB_SIZE);
        for (int t = 0; t < INITIAL_VOCAB_SIZE; ++t) {
            vocab.push_back(Expansion{t});
        }
    }
    ~BasicTokenizer() {
        std::cout << "BPE Tokenizer is cleaned" << std::endl;
    }
    void train(const std::string &text, std::size_t target_vocab_size){
        std::cout << "Training.." << std::endl;
        auto num_merges = vocab.size() - INITIAL_VOCAB_SIZE;
        // 好像没用
        auto text_size = text.length();

        std::vector<int> ids {};
        for (auto c: text) {
            ids.push_back(static_cast<TokenID>(c));
        }

        // step1: 合并规则
        //
        // step2: 找出最多的pair
    }
    std::vector<TokenID> encode(const std::string& text){
        std::cout << "Encoding..." << std::endl;
        return {1,2,34};
    }
    std::string decode(const std::string& test){
        std::cout << "Decoding..." << std::endl;
        return "hello world";
    }

    private:
    std::size_t find_pair_index(const IntPair& pair) const{
        for (std::size_t i = 0; i < merges.size(); ++i) {
            if (merges[i].pair == pair) {
                return i;
            }
        }

        return merges.size();
    }
    std::map<IntPair, int> token_counts(std::vector<TokenID>& ids) const;
    void merge_pairs(std::vector<TokenID>& ids, const IntPair& pair, TokenID new_id) const ;
    // merger array
    std::vector<Merge> merges;
    // num_merges use the fucntion of merges.size()

    // 2 dimension array with dynamic
    std::vector<Expansion> vocab;
    /*
     * [[0], [1,2]]
    */
};
