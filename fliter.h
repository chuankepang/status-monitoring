#ifndef FLITER_H
#define FLITER_H

#include "QVector"

class Fliter
{
public:
    Fliter();
};

// 定义滤波器类
class MovingAverageFilter {
public:
  explicit MovingAverageFilter();
  explicit MovingAverageFilter(int windowSize);

  double filter(double value);

private:
  int windowSize_;
  QVector<double> buffer_;
  double sum_;
  int index_;
};

#endif // FLITER_H
