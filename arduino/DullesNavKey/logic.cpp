void logical_and(int n, bool* mask, bool* wipe, bool* out){
  int i;
  for(i = 0; i < n; i++){
    out[i] = mask[i] && wipe[i];
  }
}

void logical_or(int n, bool* mask, bool* wipe, bool* out){
  int i;
  for(i = 0; i < n; i++){
    out[i] = mask[i] || wipe[i];
  }
}

void logical_copy(int n, bool* mask, bool* out){
  int i;
  for(i = 0; i < n; i++){
    out[i] = mask[i];
  }
}
  
void logical_not(int n, bool* mask, bool* out){
  int i;
  for(i = 0; i < n; i++){
    out[i] = !mask[i];
  }
}

bool logical_equal(int n, bool* mask, bool* wipe){
  bool out;
  int i = 0;
  while(i < n && mask[i] == wipe[i]){
    i++;
  }
  return i == n;
}
bool all_true(int n, bool* mask){
  int i = 0;
  
  while(i < n && mask[i]){
    i++;
  }
  return i == n;
}

bool any_false(int n, bool* mask){
  return !all_true(n, mask);
}

bool all_false(int n, bool* mask){
  int i = 0;
  
  while(i < n && !mask[i]){
    i++;
  }
  return i == n;
}

bool any_true(int n, bool* mask){
  return !all_false(n, mask);
}

