#ifndef POINTS_COMPONENT_HPP_
#define POINTS_COMPONENT_HPP_

struct PointsComponent {
  int points = 0;
  int goals = 0;
  int assists = 0;
  int blocks = 0;
  bool changed = true;

  void AddPoints(int p) {
    points += p;
    // changed = true;
  }
  int GetPoints() {
    // changed = false;
    return points;
  }
  void AddGoals(int g) {
    goals += g;
    // changed = true;
  }
  int GetGoals() {
    // changed = false;
    return goals;
  }
  void AddAssists(int a) { assists += a; }
  int GetAssists() { return assists; }
  void AddBlock(int b) { blocks++; }
  int GetBlocks() { return blocks; }
};

#endif  // POINTS_COMPONENT_HPP_