int less(int n1, int n2, int n3)
{
    int less_num = n1;
    less_num = less_num < n2? n2: less_num;
    less_num = less_num < n3? n3: less_num;
    return less_num;
}