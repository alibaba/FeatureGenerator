#include "gmock/gmock.h"

using namespace std;
using namespace ::testing;

int main(int argc, char **argv) {
    InitGoogleMock(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
