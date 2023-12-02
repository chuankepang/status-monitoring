#include "fliter.h"

MovingAverageFilter::MovingAverageFilter(int windowSize): windowSize_(windowSize), buffer_(windowSize), sum_(0), index_(0) {}

double MovingAverageFilter::filter(double value)
{
  sum_ -= buffer_[index_];
  buffer_[index_] = value;
  sum_ += value;
  index_ = (index_ + 1) % windowSize_;
  return sum_ / windowSize_;
}
