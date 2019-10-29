#ifndef MODEL_CONFIG_HPP_
#define MODEL_CONFIG_HPP_

#include <string>
#include <unordered_map>
#include <optional>

class ModelConfig {
 public:
  ModelConfig(const std::string& config_path);

  std::optional<int> GetInt(const std::string& name);
  //float GetFloat(const std::string& name);
  //std::string GetString(const std::string& name);

  bool isLoaded() {
    return is_loaded_;
  }

 private:
  std::unordered_map<std::string, int> ints_;
  bool is_loaded_ = false;
};


#endif // MODEL_CONFIG_HPP_