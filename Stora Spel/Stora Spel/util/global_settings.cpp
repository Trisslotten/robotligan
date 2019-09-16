#include "global_settings.hpp"

#include <fstream>
#include <iostream>

// Private:--------------------------------------------------------------------

// Public----------------------------------------------------------------------
GlobalSettings* GlobalSettings::Access() {
  // Return a pointer to this GlobalSettings object
  static GlobalSettings instance;
  return &instance;
}

void GlobalSettings::UpdateValuesFromFile() {
  // Create one char* to hold line identifier
  char line_identifier[1];
  // Create two strings to hold key and value
  std::string key_str;
  std::string val_str;

  // Clear the settings map
  this->settings_map_.clear();

  // Open a file stream
  std::ifstream settings_file("util/settings.txt");

  // If the file isn't able to be opened, write error
  if (!settings_file.is_open()) {
    this->WriteError("global_setting.cpp", "UpdateValuesFromFile()",
                     "Settings file cound not be opened");
    return;
  }

  // Read through the settings file
  while (settings_file) {
    // Read the line identifier
    settings_file.read(line_identifier, 1);

    // '>' indicates a line that holds a value
    if (line_identifier == ">") {
      // Read up to the '=' sign. Save as key.
      std::getline(settings_file, key_str, '=');
      // Read up to the end of the line. Save as value.
      std::getline(settings_file, val_str, '\n');
      // Save what has been read into the map
      this->settings_map_[key_str] = std::stof(val_str);
    } else {
      // If a '>' is found the program skips the line
      std::getline(settings_file, val_str, '\n');
    }
  }

  settings_file.close();
}

float GlobalSettings::ValueOf(std::string in_identifier) {
  // Check if the identifier exists
  if (this->settings_map_.find(in_identifier) == settings_map_.end()) {
    // If not, write error message to terminal
    // NTS: Should maybe write to a error log instead?
    std::cout << "ERROR::"
              << "global_settings.cpp::"
              << "ValueOf()::"
              << "Tried to access unavailable value '" << in_identifier << "'";
    return 0.0;
  }

  // If identifier exists return asociated value
  return this->settings_map_[in_identifier];
}

void GlobalSettings::WriteError(std::string in_file_name,
                                std::string in_function_name,
                                std::string in_msg) {
  // TRUE:	Write to console
  // FALSE:	Write to file (/util/errorlog.txt)
  if (true) {
    std::cout << "ERROR::" << in_file_name << "::" << in_function_name << "::"
              << "'" << in_msg << "'\n";
    return;
  }

  unsigned int file_name_size = (unsigned)strlen(in_file_name.c_str());
  unsigned int function_name_size = (unsigned)strlen(in_function_name.c_str());
  unsigned int msg_size = (unsigned)strlen(in_msg.c_str());

  std::ofstream error_file("util/errorlog.txt");

  error_file.write(in_file_name.c_str(), file_name_size);
  error_file.write(in_function_name.c_str(), function_name_size);
  error_file.write(in_msg.c_str(), msg_size);
  error_file.write("\n\n", 2);

  error_file.close();

  return;
}

void GlobalSettings::WriteMapToConsole() {
  std::cout << "GLOBAL SETTINGS:\n";
  for (const auto& m : this->settings_map_) {
    std::cout << m.first << "\t" << m.second << "\n";
  }
  std::cout << "NUMBER OF SETTINGS: " << this->settings_map_.size() << "\n";
  return;
}