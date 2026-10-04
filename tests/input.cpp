# include <iostream>
# include <vector>

using namespace std;

class TestA {
    struct Internal {
        vector<int> c, d;
        int x, y;
    };
    
    int a;
    int b;
    std::vector<int> c, d;
    TestA(int n): a(n), b(n), c(n), d(n) {
        c.push_back(0);
    }

    void out() {
        cout << a << " " << b << endl;

        for (auto i : c) {
            cout << i << " ";
        }
        cout << endl;

        for (auto i : d)
        {
            cout << i << " ";
        }
        cout << endl;
    }
};

void Greet()
{
    cout << "Hello, World!" << endl;
}

template <class T>
concept ArrayLike = requires(const T& x, size_t i, size_t j) {
    { ranges::size(x) } -> convertible_to<size_t>;
    { x[i] == x[j] } -> convertible_to<bool>;
};