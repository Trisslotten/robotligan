#include "src/replay machine/replay_machine.hpp"

#include <entt.hpp>
#include <entity/registry.hpp>

struct TestComponent {
  int a;
  float b;
  char c;
};

int main(void) {
  // HAI FFS

  ReplayMachine::Access()->TestFunctionB();

  
  entt::registry test_reg_A;
  entt::entity test_ent_A = test_reg_A.create();
  entt::entity test_ent_B = test_reg_A.create();
  entt::entity test_ent_C = test_reg_A.create();
  entt::entity test_ent_D = test_reg_A.create();
  test_reg_A.assign<TestComponent>(test_ent_A, 1, 0.25f, 'A');
  test_reg_A.assign<TestComponent>(test_ent_B, 2, 0.5f, 'B');
  test_reg_A.assign<TestComponent>(test_ent_C, 3, 1.0f, 'C');
  test_reg_A.assign<TestComponent>(test_ent_D, 4, 2.0f, 'D');

  entt::registry test_reg_B = ReplayMachine::Access()->TestFunctionC(test_reg_A);

  std::cout << "\n";
  auto test_view = test_reg_B.view<TestComponent>();
  for (auto entity : test_view) {
        
    TestComponent& ref = test_view.get(entity);
    std::cout << ref.a << " : " << ref.b << " : " << ref.c << "\n";
  }
  
}