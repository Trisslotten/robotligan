#include "Chat.hpp"
#include "util/input.hpp"
#include <GLFW\glfw3.h>
#include <sstream>
#include <iostream>

void Chat::AddMessage(std::string text) {
  if (text.size() == 0) return;
  std::vector<std::string> result;
  std::istringstream iss(text);
  for (std::string s; iss >> s;) result.push_back(s);

  int number_of_characters = 0;
  std::string row;
  for (int i = 0; i < result.size(); ++i) {
    if (number_of_characters + result[i].size() < row_length_) {
        number_of_characters += result[i].size() + 1;
        row += result[i] + " ";
    } else {
      messages_.push_back(row);
      row = result[i] + " ";
      number_of_characters = result[i].length() + 1;
    }
  }
  messages_.push_back(row);
}
//void Chat::AddToCurrentMessage(std::string text) { current_message_ += text; }

void Chat::Update(float dt) {
  if (send_message_ == true) {
    AddMessage(current_message_);
    current_message_ = "";
    send_message_ = false;
  }
  if (close_chat_ == true) {
    close_timer_ -= dt;
    if (close_timer_ < 0) {
      show_chat_ = false;
      close_chat_ = false;
    }
  } else {
    current_message_ += Input::GetCharacters();
    // std::cout << messages_.size() << std::endl;
    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE)) {
      if (current_message_.size() > 0) current_message_.pop_back();
    }
  }
 }

void Chat::SubmitText(glob::Font2DHandle font) {
   int index = messages_.size() - 5;
  if (index < 0) index = 0;
  int counter = 0;
  for (index; index < messages_.size(); ++index) {
	glob::Submit(font, glm::vec2(50.f, 700.f - 20.f * counter), 20,
                 messages_[index], glm::vec4(0, 1, 1, 1));
    counter++;
  
  }
  glob::Submit(font, glm::vec2(50.f, 700.f - 20.f * 5), 20,
               current_message_, glm::vec4(0, 1, 1, 1));
 }

 void Chat::SetShowChat() {
   show_chat_ = true;
   close_chat_ = false;
   close_timer_ = 1000.f;
 }

void Chat::SetSendMessage(bool send) { send_message_ = send; }

void Chat::CloseChat() {
  close_timer_ = 2.0f;
  close_chat_ = true;
}

std::string Chat::GetCurrentMessage() { return current_message_; }

bool Chat::ShowChat() { return show_chat_; }

bool Chat::IsClosing() { return close_chat_;}