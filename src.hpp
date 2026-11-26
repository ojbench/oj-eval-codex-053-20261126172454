#pragma once
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

namespace Grammar {
enum class TransitionType { Epsilon, a, b };
struct Transition {
  TransitionType type;
  int to;
  Transition(TransitionType type, int to) : type(type), to(to) {}
};

class NFA {
private:
  int start = 0;
  std::unordered_set<int> ends;
  std::vector<std::vector<Transition>> transitions{1};

  // Copy a disjoint graph and translate all destination indices.
  int Append(const NFA &other) {
    const int offset = static_cast<int>(transitions.size());
    for (const auto &edges : other.transitions) {
      transitions.emplace_back();
      for (const auto &edge : edges)
        transitions.back().emplace_back(edge.type, edge.to + offset);
    }
    return offset;
  }

public:
  NFA() = default;
  ~NFA() = default;

  std::unordered_set<int>
  GetEpsilonClosure(std::unordered_set<int> states) const {
    std::queue<int> pending;
    for (int state : states) pending.push(state);
    while (!pending.empty()) {
      const int state = pending.front();
      pending.pop();
      for (const auto &edge : transitions[state])
        if (edge.type == TransitionType::Epsilon && states.insert(edge.to).second)
          pending.push(edge.to);
    }
    return states;
  }

  std::unordered_set<int> Advance(std::unordered_set<int> current_states,
                                  char character) const {
    if (character != 'a' && character != 'b') return {};
    const auto type = character == 'a' ? TransitionType::a : TransitionType::b;
    std::unordered_set<int> next;
    for (int state : GetEpsilonClosure(current_states))
      for (const auto &edge : transitions[state])
        if (edge.type == type) next.insert(edge.to);
    return GetEpsilonClosure(next);
  }

  bool IsAccepted(int state) const { return ends.find(state) != ends.end(); }
  int GetStart() const { return start; }

  friend NFA MakeSimple(const char &character);
  friend NFA MakeStar(const char &character);
  friend NFA MakePlus(const char &character);
  friend NFA MakeQuestion(const char &character);
  friend NFA Concatenate(const NFA &first, const NFA &second);
  friend NFA Union(const NFA &first, const NFA &second);
  friend class RegexChecker;
};

inline NFA MakeSimple(const char &character) {
  NFA nfa;
  nfa.transitions.resize(2);
  nfa.transitions[0].emplace_back(
      character == 'a' ? TransitionType::a : TransitionType::b, 1);
  nfa.ends.insert(1);
  return nfa;
}

inline NFA MakeStar(const char &character) {
  NFA nfa;
  nfa.transitions[0].emplace_back(
      character == 'a' ? TransitionType::a : TransitionType::b, 0);
  nfa.ends.insert(0);
  return nfa;
}

inline NFA MakePlus(const char &character) {
  NFA nfa = MakeSimple(character);
  nfa.transitions[1].emplace_back(nfa.transitions[0][0].type, 1);
  return nfa;
}

inline NFA MakeQuestion(const char &character) {
  NFA nfa = MakeSimple(character);
  nfa.ends.insert(0);
  return nfa;
}

inline NFA Concatenate(const NFA &first, const NFA &second) {
  NFA result = first;
  const int offset = result.Append(second);
  for (int state : first.ends)
    result.transitions[state].emplace_back(TransitionType::Epsilon,
                                           second.start + offset);
  result.ends.clear();
  for (int state : second.ends) result.ends.insert(state + offset);
  return result;
}

inline NFA Union(const NFA &first, const NFA &second) {
  NFA result;
  const int left = result.Append(first);
  const int right = result.Append(second);
  result.transitions[0].emplace_back(TransitionType::Epsilon, first.start + left);
  result.transitions[0].emplace_back(TransitionType::Epsilon, second.start + right);
  for (int state : first.ends) result.ends.insert(state + left);
  for (int state : second.ends) result.ends.insert(state + right);
  return result;
}

class RegexChecker {
private:
  NFA nfa;

public:
  RegexChecker(const std::string &regex) {
    // Each alternative has its own chain, reached from the shared start.
    int tail = 1;
    nfa.transitions.emplace_back();
    nfa.transitions[0].emplace_back(TransitionType::Epsilon, tail);
    for (std::string::size_type i = 0; i < regex.size(); ++i) {
      const char character = regex[i];
      if (character == '|') {
        nfa.ends.insert(tail);
        tail = static_cast<int>(nfa.transitions.size());
        nfa.transitions.emplace_back();
        nfa.transitions[0].emplace_back(TransitionType::Epsilon, tail);
        continue;
      }
      char quantifier = '\0';
      if (i + 1 < regex.size() &&
          (regex[i + 1] == '*' || regex[i + 1] == '+' || regex[i + 1] == '?'))
        quantifier = regex[++i];
      const auto type = character == 'a' ? TransitionType::a : TransitionType::b;
      const int next = static_cast<int>(nfa.transitions.size());
      nfa.transitions.emplace_back();
      nfa.transitions[tail].emplace_back(type, next);
      if (quantifier == '*' || quantifier == '?')
        nfa.transitions[tail].emplace_back(TransitionType::Epsilon, next);
      if (quantifier == '*' || quantifier == '+')
        nfa.transitions[next].emplace_back(type, next);
      tail = next;
    }
    nfa.ends.insert(tail);
  }

  bool Check(const std::string &str) const {
    auto states = nfa.GetEpsilonClosure({nfa.GetStart()});
    for (char character : str) {
      states = nfa.Advance(states, character);
      if (states.empty()) return false;
    }
    for (int state : states)
      if (nfa.IsAccepted(state)) return true;
    return false;
  }
};
} // namespace Grammar
