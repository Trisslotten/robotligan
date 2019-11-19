#ifndef MODEL_CONFIG_HPP_
#define MODEL_CONFIG_HPP_

#include <optional>
#include <string>
#include <unordered_map>

class ModelConfig {
 public:
  ModelConfig(const std::string& config_path);

  std::optional<int> GetInt(const std::string& name);
  std::optional<std::string> GetWord(const std::string& name);
  std::optional<float> GetFloat(const std::string& name);
  std::optional<bool> GetBool(const std::string& name);

  bool isLoaded() { return is_loaded_; }

 private:
  std::unordered_map<std::string, int> ints_;
  std::unordered_map<std::string, std::string> words_;
  std::unordered_map<std::string, float> floats_;
  std::unordered_map<std::string, bool> bools_;
  bool is_loaded_ = false;
};

#endif  // MODEL_CONFIG_HPP_