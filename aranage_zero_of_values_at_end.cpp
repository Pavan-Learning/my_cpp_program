#include <iostream>
#include <string>
#include <array>
#include <vector>

using namespace std;

int countVowels(const string& str) {
    int count = 0;
    for (char ch : str) {
        ch = tolower(ch);
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
            count++;
        }
    }
    return count;
}

vector<int> placing_zeros_at_end_of_array(vector<int> arr) {
    int insertPos = 0;
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] != 0) {
            auto temp = arr[i];
            arr[insertPos] = temp;
            arr[i] = 0;
            // swap(arr[insertPos], arr[i]);
            insertPos++;
        }
    }
    return arr;
}

void printVector(const vector<int>& arr) {
    for (int num : arr) {
        cout << num << " ";
    }
    cout << endl;
}

int main(){

    string str1;
    getline(cin, str1);

    cout << "Total Vowels: " << countVowels(str1) << endl;

    vector<int> arr = {0, 1, 2, 0, 3, 4, 0, 5};
    vector<int> result = placing_zeros_at_end_of_array(arr);
    printVector(result);

    return 0;
}