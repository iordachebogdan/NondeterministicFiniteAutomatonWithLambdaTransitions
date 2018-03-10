#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <queue>

using namespace std;

class Test {
  public:
    Test(string path = "") {
        //loads test from path (if path is empty then load from terminal)
        if (path == "") {
            load(cin);
        } else {
            ifstream fin(path.data());
            load(fin);
            fin.close();
        }
    }
    Test(const Test& rhs) = default;
    Test& operator = (const Test& rhs) = default;

    size_t state_count() const { return state_count_; }
    const vector<int>& states() const { return states_; }
    size_t symbol_count() const { return symbol_count_; }
    const vector<char>& symbols() const { return symbols_; }
    int initial_state() const { return initial_state_; }
    size_t final_state_count() const { return final_state_count_; }
    const vector<int>& final_states() const {return final_states_; }
    size_t transition_count() const { return transition_count_; }
    const vector< tuple<int, char, int> >& transitions() const { return transitions_; };
    size_t query_count() const { return query_count_; }
    const vector<string>& queries() const { return queries_; }

  private:
    void load(istream& in) {
        in >> state_count_;
        states_.resize(state_count_);
        for (auto& state : states_)
            in >> state;
        in >> symbol_count_;
        symbols_.resize(symbol_count_);
        for (auto& symbol : symbols_)
            in >> symbol;
        in >> initial_state_;
        in >> final_state_count_;
        final_states_.resize(final_state_count_);
        for (auto& final_state : final_states_)
            in >> final_state;
        in >> transition_count_;
        transitions_.resize(transition_count_);
        for (auto& transition : transitions_) {
            int from, to; char symbol;
            in >> from >> symbol >> to;
            transition = make_tuple(from, symbol, to);
        }

        in >> query_count_;
        queries_.resize(query_count_);
        for (auto& query : queries_)
            in >> query;
    }

    size_t state_count_, symbol_count_, final_state_count_, transition_count_;
    int initial_state_;
    vector< int > states_, final_states_;
    vector< char > symbols_;
    vector< tuple<int, char, int> > transitions_;

    size_t query_count_;
    vector< string > queries_;

  public:
    //lambda transitions are represented by transitions where the symbol is '.'
    static constexpr char lambda = '.';
};

class LNFA {
  private:
    typedef unordered_set< int > StateSet;

  public:
    LNFA(const Test& test) {
        init_state_ = test.initial_state();
        current_state_.insert(test.initial_state());
        total_states_ = test.states();
        final_states_ = test.final_states();
        auto transitions = test.transitions();
        for (auto& transition : transitions) {
            int from, to; char symbol;
            tie(from, symbol, to) = transition;
            if (!transitions_.count({from, symbol}))
                transitions_[{from, symbol}] = StateSet();
            transitions_[{from, symbol}].insert(to);
        }
    }
    LNFA(const LNFA& rhs) = default;
    LNFA& operator = (const LNFA& rhs) = default;

    bool check_word(string word) {
        reset();
        for (auto& symbol : word) {
            input(symbol);
            lambda_closure();
        }
        return check_for_final_states();
    }

  private:
    struct TransitionInput;

    int init_state_;
    StateSet current_state_;
    vector< int > total_states_;
    vector< int > final_states_;
    map< TransitionInput, StateSet > transitions_;

    void reset() {
        current_state_.clear();
        current_state_.insert(init_state_);
        lambda_closure();
    }

    void input(char symbol) {
        StateSet update_current_state;
        for (auto&& state : current_state_) {
            TransitionInput transition_input = {state, symbol};
            if (!transitions_.count(transition_input))
                continue;
            for (auto&& next : transitions_[transition_input])
                update_current_state.insert(next);
        }
        current_state_ = update_current_state;
    }

    void lambda_closure() {
        //resolve lambda-closure using breadth-first search
        queue< int > state_queue;
        StateSet closure = current_state_;
        for (auto&& state : current_state_)
            state_queue.push(state);

        while (!state_queue.empty()) {
            int current = state_queue.front();
            state_queue.pop();
            TransitionInput transition_input = {current, Test::lambda};
            for (auto&& next : transitions_[transition_input])
                if (closure.find(next) == closure.end()) {
                    closure.insert(next);
                    state_queue.push(next);
                }
        }

        current_state_ = closure;
    }

    bool check_for_final_states() {
        for (auto&& it : final_states_)
            if (current_state_.find(it) != current_state_.end())
                return true;
        return false;
    }

    struct TransitionInput {
        int source;
        char symbol;
        bool operator<(const TransitionInput& rhs) const {
            return (source == rhs.source ? symbol < rhs.symbol : source < rhs.source);
        }
    };
};

class Solver {
  public:
    void operator()(string input_path = "", string output_path = "") {
        Test test(input_path);
        LNFA lnfa(test);
        vector< string > output;
        for (auto&& query : test.queries())
            if (lnfa.check_word(query))
                output.push_back("DA");
            else
                output.push_back("NU");

        if (output_path == "") {
            for (auto&& it : output)
                cout << it << '\n';
        }
        else {
            ofstream fout(output_path.data());
            for (auto&& it : output)
                fout << it << '\n';
            fout.close();
        }
    }
};

int main() {
    Solver solver;
    solver("test.in", "test.out");
    return 0;
}