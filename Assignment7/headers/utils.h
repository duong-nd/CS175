#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

/**
 * The string splitting functions below are adpated from http://stackoverflow.com/a/236803.
 */

static vector<string> &split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) elems.push_back(item);
  return elems;
}

static vector<string> split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

#endif
