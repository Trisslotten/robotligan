#include "modelconfig.hpp"

#include <fstream>
#include <iostream>

class Tokens {
 public:
  Tokens(const std::vector<std::string>& tokens) { tokens_ = tokens; }
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
      char current = line[i];
      if (std::isspace(current)) {
        if (curr_token.size() > 0) {
          tokens_vec.push_back(curr_token);
          curr_token.clear();
        }
      } else {
        curr_token += current;
      }
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
    } else {
      std::cout << "WARNING: Unknown type in model config: '" << config_path
                << "'\n";
    }
  }
}

std::optional<int> ModelConfig::GetInt(const std::string& name) {
  auto iter = ints_.find(name);
  if (iter != ints_.end()) {
    return iter->second;
  }
  return {};
}
