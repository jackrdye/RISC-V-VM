char volatile *const ConsoleWriteChar = (char *)0x0800;
int main() {
    *ConsoleWriteChar = 'H';
    return 0;
}