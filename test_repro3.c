#define LUA_NUMBER_FRMLEN ""

#define SIZELENMOD	(sizeof(LUA_NUMBER_FRMLEN)/sizeof(char))

static void addlenmod(char *form, const char *lenmod);

int main() {
    char form[50] = "g";
    addlenmod(form, LUA_NUMBER_FRMLEN);
    return 0;
}
