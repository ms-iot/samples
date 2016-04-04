//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


extern "C" {
    #include "logger.h"
}


#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <stdint.h>
using namespace std;


//-----------------------------------------------------------------------------
// file_exist citation -
// http://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform/230070#230070
//-----------------------------------------------------------------------------
bool file_exist(const char *filename) {
  struct stat   buffer;
  return (stat(filename, &buffer) == 0);
}

//-----------------------------------------------------------------------------
// stdio redirection citation - http://www.cplusplus.com/forum/general/94879/
//-----------------------------------------------------------------------------
static int fd;
static int defout;

bool directStdOutToFile(const char *filename) {
    if (!filename) {
        return false;
    }

    if ((defout = dup(1)) < 0) {
        fprintf(stderr, "Can't dup(2) - (%s)\n", strerror(errno));
        return false;
    }
    if ((fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stderr, "Can't open(2) - (%s)\n", strerror(errno));
        return false;
    }
    // redirect output to the file
    if (dup2(fd, 1) < 0)  {
        fprintf(stderr, "Can't dup2(2) - (%s)\n", strerror(errno));
        return false;
    }
    close(fd);  // Descriptor no longer needed

    fflush(stdout);          // FLUSH ALL OUTPUT TO file

    return true;
}

bool directStdOutToConsole() {
    fflush(stdout);          // FLUSH ALL OUTPUT TO file

    // redirect output back to stdout
    if (dup2(defout, 1) < 0) {
        fprintf(stderr, "Can't dup2(2) - (%s)\n", strerror(errno));
        return false;
    }
    close(defout);  // Copy of stdout no longer needed

    return true;
}

//-----------------------------------------------------------------------------
// CalcFileMD5 citation - http://stackoverflow.com/questions/3395690/md5sum-of-file-in-linux-c
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <ctype.h>

#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)

#define PATH_LEN 256
#define MD5_LEN 32

bool CalcFileMD5(const char *file_name, char *md5_sum) {
    #define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
    char cmd[PATH_LEN + sizeof (MD5SUM_CMD_FMT)];
    snprintf(cmd, sizeof(cmd), MD5SUM_CMD_FMT, file_name);
    #undef MD5SUM_CMD_FMT

    FILE *p = popen(cmd, "r");
    if (p == NULL) return 0;

    int i, ch;
    for (i = 0; i < MD5_LEN && isxdigit(ch = fgetc(p)); i++) {
        *md5_sum++ = ch;
    }

    *md5_sum = '\0';
    pclose(p);
    return (i == MD5_LEN);
}


//-----------------------------------------------------------------------------
//  Tests
//-----------------------------------------------------------------------------
TEST(LoggerTest, StringArg) {
    char testFile[] = "tst_stringarg.txt";
    char stdFile[]  = "std_stringarg.txt";

    // Try deleting test file
    remove(testFile);

    directStdOutToFile(testFile);
    const char *tag = "StringArg";
	OC_LOG(INFO, tag, "This is a fixed string call");
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, StringArgNoTag) {
    char testFile[] = "tst_stringargnotag.txt";
    char stdFile[]  = "std_stringargnotag.txt";

    directStdOutToFile(testFile);
    OC_LOG(INFO, 0, "This is a fixed string call");
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, StringArgNoLogStr) {
    char testFile[] = "tst_stringargnologstr.txt";
    char stdFile[]  = "std_stringargnologstr.txt";

    directStdOutToFile(testFile);
    const char *tag = "StringArgNoLogStr";
    OC_LOG(INFO, tag, 0);
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, StringArgNoTagNoLogStr) {
    char testFile[] = "tst_stringargnotagnologstr.txt";
    char stdFile[]  = "std_stringargnotagnologstr.txt";

    directStdOutToFile(testFile);
    OC_LOG(INFO, 0, 0);
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, StringArgLevels) {
    char testFile[] = "tst_stringarglevels.txt";
    char stdFile[]  = "std_stringarglevels.txt";

    directStdOutToFile(testFile);
    const char *tag = "StringArgLevels";
    // DEBUG, INFO, WARNING, ERROR, FATAL
    OC_LOG(DEBUG, tag, "this is a DEBUG message");
    OC_LOG(INFO, tag, "this is a INFO message");
    OC_LOG(WARNING, tag, "this is a WARNING message");
    OC_LOG(ERROR, tag, "this is a ERROR message");
    OC_LOG(FATAL, tag, "this is a FATAL message");
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, StringArgMultiline) {
    char testFile[] = "tst_stringargmultiline.txt";
    char stdFile[]  = "std_stringargmultiline.txt";

    directStdOutToFile(testFile);
    const char *tag = "StringArgMultiline";
    OC_LOG(DEBUG, tag, "this is a DEBUG message\non multiple\nlines");
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, VariableArg) {
    char testFile[] = "tst_variablearg.txt";
    char stdFile[]  = "std_variablearg.txt";

    directStdOutToFile(testFile);
    const char *tag = "VariableArg";
    // DEBUG, INFO, WARNING, ERROR, FATAL
    OC_LOG_V(DEBUG, tag, "this is a char: %c", 'A');
    OC_LOG_V(DEBUG, tag, "this is an integer: %d", 123);
    OC_LOG_V(DEBUG, tag, "this is a string: %s", "hello");
    OC_LOG_V(DEBUG, tag, "this is a float: %5.2f", 123.45);
    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}

TEST(LoggerTest, LogBuffer) {
    char testFile[] = "tst_logbuffer.txt";
    char stdFile[]  = "std_logbuffer.txt";

    directStdOutToFile(testFile);
    const char *tag = "LogBuffer";

    // Log buffer
    uint8_t buffer[50];
    for (int i = 0; i < (int)(sizeof buffer); i++) {
        buffer[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer, sizeof buffer);

    // Log buffer, 128 bytes is a good boundary (8 rows of 16 values)
    uint8_t buffer1[128];
    for (int i = 0; i < (int)(sizeof buffer1); i++) {
        buffer1[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer1, sizeof buffer1);

    // 1 below 128 byte boundary
    uint8_t buffer2[127];
    for (int i = 0; i < (int)(sizeof buffer2); i++) {
        buffer2[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer2, sizeof buffer2);

    // 1 above 128 byte boundary
    uint8_t buffer3[129];
    for (int i = 0; i < (int)(sizeof buffer3); i++) {
        buffer3[i] = i;
    }
    OC_LOG_BUFFER(DEBUG, tag, buffer3, sizeof buffer3);

    directStdOutToConsole();

    bool testFileExists = file_exist(testFile);
    EXPECT_TRUE(testFileExists);
    bool stdFileExists = file_exist(stdFile);
    EXPECT_TRUE(stdFileExists);

    if (testFileExists && stdFileExists) {
        char testFileMD5[MD5_LEN + 1] = {0};
        char stdFileMD5[MD5_LEN + 1] = {0};

        EXPECT_TRUE(CalcFileMD5(testFile, testFileMD5));
        EXPECT_TRUE(CalcFileMD5(stdFile, stdFileMD5));

        EXPECT_STREQ(stdFileMD5, testFileMD5);
    }
}
