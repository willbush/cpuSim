#include "cute.h"
#include "Memory.h"
#include "Cpu.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"
#include "CPUsimTest.h"
#include "sys/wait.h"

struct CpuSimTest {
    int forkResult;
    int cpuToMem[2];
    int memToCpu[2];

    // serves as the test setup
    CpuSimTest() {
        pipe(cpuToMem);
        pipe(memToCpu);
        forkResult = fork();
    }

    // serves as the test tear down.
    ~CpuSimTest() {
    }

    void canLoadValueIntoAC() {
        const int len = 3;
        int instructions[len] = { 1, 50, 50 }; // load 50 into AC and exit
        assertCpuEventsEqual("w LV50 e", createVector(instructions, len));
    }

    void canLoadValueIntoAcFromAddress() {
        const int len = 7;
        int instructions[len] = { 1, 77, 7, 20, 3, 20, 50 };
        assertCpuEventsEqual("w LV77 SA20,77 LVFA20,77 e", createVector(instructions, len));
    }

    void canStoreIntoAddressAndLoadFromAddress() {
        const int len = 7;
        // load 50 into AC, store AC into address 100, load from 100 into the AC and exit
        int instructions[len] = { 1, 50, 7, 100, 2, 100, 50 };
        assertCpuEventsEqual("w LV50 SA100,50 LA100,50 e", createVector(instructions, len));
    }

    void canIncrementandDecrementX() {
        const int len = 4;
        // increment registers x twice and y once
        int instructions[len] = { 25, 25, 26, 50 };
        assertCpuEventsEqual("w x++ x++ x-- e", createVector(instructions, len));
    }

    void canLoadValueFromAddressPlusXintoAC() {
        const int len = 8;
        int instructions[len] = { 1, 666, 7, 1000, 25, 4, 999, 50 };
        assertCpuEventsEqual("w LV666 SA1000,666 x++ LVFAPX1000,666 e", createVector(instructions, len));
    }

    void canLoadValueFromAddressPlusYintoAC() {
        const int len = 8;
        int instructions[len] = { 1, 10, 16, 7, 20, 5, 10, 50 };
        assertCpuEventsEqual("w LV10 CY10 SA20,10 LVFAPY20,10 e", createVector(instructions, len));
    }

    void canCopyToX() {
        const int len = 4;
        int instructions[len] = { 1, 7, 14, 50 };
        assertCpuEventsEqual("w LV7 CX7 e", createVector(instructions, len));
    }

    void canCopyToY() {
        const int len = 4;
        int instructions[len] = { 1, 7, 16, 50 };
        assertCpuEventsEqual("w LV7 CY7 e", createVector(instructions, len));
    }

    void canCopyXtoAC() {
        const int len = 7;
        int instructions[len] = { 1, 7, 14, 1, 0, 15, 50 };
        assertCpuEventsEqual("w LV7 CX7 LV0 CX2AC7 e", createVector(instructions, len));
    }

    void canCopyYtoAC() {
        const int len = 7;
        int instructions[len] = { 1, 7, 16, 1, 0, 17, 50 };
        assertCpuEventsEqual("w LV7 CY7 LV0 CY2AC7 e", createVector(instructions, len));
    }

    void canJumpToAddress() {
        const int len = 6;
        // jump pass garbage -1 to the end (50)
        int instructions[len] = { 20, 5, -1, -1, -1, 50 };
        assertCpuEventsEqual("w JA5 e", createVector(instructions, len));
    }

    void canJumpToAddressIfAcIsZero() {
        const int len = 12;
        int instructions[len] = { 1, 1, 21, 5, 1, 0, 21, 11, -1, -1, -1, 50 };
        assertCpuEventsEqual("w LV1 LV0 JA11 e", createVector(instructions, len));
    }

    void canJumpToAddressIfAcIsNotZero() {
        const int len = 12;
        int instructions[len] = { 1, 0, 22, 5, 1, 1, 22, 11, -1, -1, -1, 50 };
        assertCpuEventsEqual("w LV0 LV1 JA11 e", createVector(instructions, len));
    }

    void canPutPortOne() {
        const int len = 5;
        int instructions[len] = { 1, 10, 9, 1, 50 };
        assertCpuEventsEqual("w LV10 P1,10 e", createVector(instructions, len));
    }

    void canPutPortTwo() {
        const int len = 5;
        int instructions[len] = { 1, 1101, 9, 2, 50 };
        assertCpuEventsEqual("w LV1101 P2,M e", createVector(instructions, len));
    }

    void canPush() {
        const int len = 7;
        int instructions[len] = { 1, 9, 27, 1, 10, 27, 50 };
        assertCpuEventsEqual("w LV9 PU998,9 LV10 PU997,10 e", createVector(instructions, len));
    }

    void canPushAndPop() {
        const int len = 9;
        int instructions[len] = { 1, 9, 27, 1, 10, 27, 28, 28, 50 };
        assertCpuEventsEqual("w LV9 PU998,9 LV10 PU997,10 PO997,10 PO998,9 e", createVector(instructions, len));
    }

    void canPutRandInAC() {
        const int len = 2;
        int instructions[len] = { 8, 50 };
        assertCpuEventsEqual("w R e", createVector(instructions, len));
    }

    void canAddXtoAC() {
        const int len = 5;
        int instructions[len] = { 1, 4, 25, 10, 50 };
        assertCpuEventsEqual("w LV4 x++ AX2AC1,4 e", createVector(instructions, len));
    }

    void canAddYtoAC() {
        const int len = 5;
        int instructions[len] = { 1, 4, 16, 11, 50 };
        assertCpuEventsEqual("w LV4 CY4 AY2AC4,4 e", createVector(instructions, len));
    }

    void canAddTwoRandAndPrint() {
        const int len = 10;
        int instructions[len] = { 8, 14, 8, 16, 8, 10, 11, 9, 1, 50 };
        assertCpuEventsEqual("w R CX87 R CY78 R AX2AC87,16 AY2AC78,103 P1,181 e", createVector(instructions, len));
    }

    void canCallAddress() {
        const int len = 10;
        int instructions[len] = { 23, 2, 50 };
        assertCpuEventsEqual("w PU998,2 JA2 CA2 e", createVector(instructions, len));
    }

    void canReturnFromStack() {
        const int len = 6;
        int instructions[len] = { 23, 3, 50, 25, 25, 24 };
        assertCpuEventsEqual("w PU998,2 JA3 CA3 x++ x++ PO998,2 JA2 RET2 e", createVector(instructions, len));
    }

    void canPrintHi() {
        const int len = 16;
        int instructions[len] = { 1, 72, 9, 2, 1, 73, 9, 2, 23, 11, 50, 1, 10, 9, 2, 24 };
        assertCpuEventsEqual("w LV72 P2,H LV73 P2,I PU998,10 JA11 CA11 LV10 P2,\n PO998,10 JA10 RET10 e",
                createVector(instructions, len));
    }

private:
    std::vector<int> createVector(int array[], const int len) {
        std::vector<int> v(&array[0], &array[0] + len);
        return v;
    }

    void assertCpuEventsEqual(const std::string& expected, const std::vector<int>& program) {
        if (forkResult == 0) {
            Memory m(cpuToMem[0], memToCpu[1], program);
            _exit(EXIT_SUCCESS);
        } else {
            Cpu c(memToCpu[0], cpuToMem[1]);
            ASSERT_EQUAL(expected, c.getEvents());
            waitpid(-1, NULL, 0); // wait for child
        }
    }
};

void runSuite(int argc, char const *argv[]) {
    cute::xml_file_opener xmlfile(argc, argv);
    cute::xml_listener<cute::ide_listener<> > lis(xmlfile.out);
    cute::suite s;
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canLoadValueIntoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canLoadValueIntoAcFromAddress));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canStoreIntoAddressAndLoadFromAddress));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canIncrementandDecrementX));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canLoadValueFromAddressPlusXintoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canLoadValueFromAddressPlusYintoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canCopyToX));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canCopyToY));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canCopyXtoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canCopyYtoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canJumpToAddress));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canJumpToAddressIfAcIsZero));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canJumpToAddressIfAcIsNotZero));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPutPortOne));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPutPortTwo));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPush));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPushAndPop));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPutRandInAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canAddXtoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canAddYtoAC));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canAddTwoRandAndPrint));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canCallAddress));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canReturnFromStack));
    s.push_back(CUTE_SMEMFUN(CpuSimTest, canPrintHi));

    cute::makeRunner(lis, argc, argv)(s, "CPUsimTest");
}

int main(int argc, char const *argv[]) {
    runSuite(argc, argv);
}
