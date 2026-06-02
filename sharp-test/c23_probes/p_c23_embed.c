const unsigned char data[] = {
#embed "/tmp/sharp_c23_embed_test.bin"
};
int main(void) {
    return data[0] + data[1] - ('H' + 'e');
}
