
int count_digits(char* str) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            count++;
        }
    }
    return count;
}

int main() {
    return 0;
}