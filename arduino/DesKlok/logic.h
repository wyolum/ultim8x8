void logical_and(int n, bool* mask, bool* wipe, bool* out);
void logical_or(int n, bool* mask, bool* wipe, bool* out);
void logical_copy(int n, bool* mask, bool* out);
void logical_not(int n, bool* mask, bool* out);
bool logical_equal(int n, bool* mask, bool* wipe);
bool all_true(int n, bool* mask);
bool any_false(int n, bool* mask);
bool all_false(int n, bool* mask);
bool any_true(int n, bool* mask);

