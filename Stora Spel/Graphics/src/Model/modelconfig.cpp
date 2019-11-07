#include "modelconfig.hpp"

#include <fstream>
#include <iostream>

namespace {

template <class T>
std::optional<T> GetItem(const std::unordered_map<std::string, T>& items,
                         const std::string name) {
  auto iter = items.find(name);
  if (iter != items.end()) {
    return iter->second;
  }
  return {};
}

}  // namespace

class Tokens {
 public:
  Tokens(const std::vector<std::string>& tokens) {
    tokens_ = tokens;
    /*
    std::cout << "tokens: ";
    for (int i = 0; i < tokens.size(); i++) {
      std::cout << "'" << tokens[i] << "', ";
    }
    std::cout << "\n";
    */
  }
  std::string Consume() {
    std::string result;
    if (index < tokens_.size()) {
      result = tokens_[index];
    }
    index++;
    return result;
  }
  bool Empty() { return index >= tokens_.size(); }

 private:
  int index = 0;
  std::vector<std::string> tokens_;
};

ModelConfig::ModelConfig(const std::string& config_path) {
  std::ifstream settings_file(config_path);

  if (!settings_file.is_open()) {
    // std::cout << "ERROR: Could not open model config: '" << config_path <<
    // "'\n";
    is_loaded_ = false;
    return;
  }
  is_loaded_ = true;

  std::string line;
  std::string key_str;
  std::string val_str;

  std::vector<std::string> tokens_vec;

  int lines = 0;
  std::string curr_token;
  while (settings_file) {
    lines++;
    // Read up to the end of the line. Save as the current line
    std::getline(settings_file, line, '\n');
    for (int i = 0; i < line.size(); i++) {
      unsigned char current = static_cast<unsigned char>(line[i]);
      if (std::isspace(current)) {
        if (curr_token.size() > 0) {
          tokens_vec.push_back(curr_token);
          curr_token.clear();
        }
      } else if (current == '#') {
        break;
      } else {
        curr_token += current;
      }
    }
    if (curr_token.size() > 0) {
      tokens_vec.push_back(curr_token);
      curr_token.clear();
    }
  }
  if (curr_token.size() > 0) {
    tokens_vec.push_back(curr_token);
  }

  Tokens tokens{tokens_vec};

  while (!tokens.Empty()) {
    std::string type = tokens.Consume();
    std::string key = tokens.Consume();
    std::string value = tokens.Consume();

    if (type.empty() || key.empty() || value.empty()) {
      break;
    }

    if (type == "i") {
      ints_[key] = std::stoi(value);
    } else if (type == "w") {
      words_[key] = value;
    } else if (type == "f") {
      floats_[key] = std::stof(value);
    } else {
      std::cout << "WARNING: Unknown type '" << type << "' in model config: '"
                << config_path << "'\n";
    }
  }
}

std::optional<int> ModelConfig::GetInt(const std::string& name) {
  return GetItem(ints_, name);
}

std::optional<std::string> ModelConfig::GetWord(const std::string& name) {
  return GetItem(words_, name);
}

std::optional<float> ModelConfig::GetFloat(const std::string& name) {
  return GetItem(floats_, name);
}
