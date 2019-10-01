#ifndef CHAT_HPP_
#define CHAT_HPP_

#include <glob/graphics.hpp>
#include <string>
#include <vector>

class Chat {
 private:
  std::vector<std::string> messages_;
  std::string current_message_;
  float close_timer_;
  int row_length_ = 50;
  bool show_chat_ = false;
  bool send_message_ = false;
  bool close_chat_ = false;
  public : 
  void AddMessage(std::string text);
  //void AddToCurrentMessage(std::string text);
  void Update(float dt);
  void SubmitText(glob::Font2DHandle font);
  void SetShowChat();
  void SetSendMessage(bool send);
  void CloseChat();
  std::string GetCurrentMessage();
  bool IsVisable();
  bool IsClosing();
};

#endif  // CHAT_HPP_