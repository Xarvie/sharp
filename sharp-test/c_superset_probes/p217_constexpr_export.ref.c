constexpr int BUFFER_SIZE = 1024;

constexpr int PAGE_SIZE = 4096;

constexpr int max(int a, int b) {

    return a > b ? a : b;
}

constexpr int min(int a, int b) {

    return a < b ? a : b;
}

constexpr int array_elements(int total_bytes) {

    return total_bytes / sizeof(int);
}

int compute_buffer_pages(void) {

    return BUFFER_SIZE * sizeof(int) / PAGE_SIZE;
}

int main(void) {

    if (BUFFER_SIZE != 1024) 
        return 1;

    if (PAGE_SIZE != 4096) 
        return 2;

    if (max(3, 7) != 7) 
        return 3;

    if (min(3, 7) != 3) 
        return 4;

    int buf[BUFFER_SIZE];

    if (sizeof(buf) != 4096) 
        return 5;

    int pages = compute_buffer_pages();

    if (pages != 1) 
        return 6;

    if (max(min(5, 10), 3) != 5) 
        return 7;

    if (min(max(5, 10), 8) != 8) 
        return 8;

    return 0;
}
