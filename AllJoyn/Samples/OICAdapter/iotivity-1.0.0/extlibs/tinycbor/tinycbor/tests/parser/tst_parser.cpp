/****************************************************************************
**
** Copyright (C) 2015 Intel Corporation
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#define _XOPEN_SOURCE 700
#include <QtTest>
#include "cbor.h"
#include <locale.h>
#include <stdio.h>

extern "C" FILE *open_memstream(char **bufptr, size_t *sizeptr);

Q_DECLARE_METATYPE(CborError)

class tst_Parser : public QObject
{
    Q_OBJECT
private slots:
    void initParserEmpty();

    // parsing API
    void fixed_data();
    void fixed();
    void strings_data();
    void strings() { fixed(); }
    void tags_data();
    void tags() { fixed(); }
    void tagTags_data() { tags_data(); }
    void tagTags();
    void emptyContainers_data();
    void emptyContainers();
    void arrays_data();
    void arrays();
    void undefLengthArrays_data() { arrays_data(); }
    void undefLengthArrays();
    void nestedArrays_data() { arrays_data(); }
    void nestedArrays();
    void maps_data();
    void maps();
    void undefLengthMaps_data() { maps_data(); }
    void undefLengthMaps();
    void nestedMaps_data() { maps_data(); }
    void nestedMaps();
    void mapMixed_data();
    void mapMixed() { arrays(); }
    void mapsAndArrays_data() { arrays_data(); }
    void mapsAndArrays();

    // convenience API
    void stringLength_data();
    void stringLength();
    void stringCompare_data();
    void stringCompare();
    void mapFind_data();
    void mapFind();

    // validation & errors
    void validation_data();
    void validation();
    void resumeParsing_data();
    void resumeParsing();
    void endPointer_data();
    void endPointer();
    void recursionLimit_data();
    void recursionLimit();
};

CborError parseOne(CborValue *it, QString *parsed)
{
    CborError err;
    char *buffer;
    size_t size;

    setlocale(LC_ALL, "C");
    FILE *f = open_memstream(&buffer, &size);
    err = cbor_value_to_pretty_advance(f, it);
    fclose(f);

    *parsed = QString::fromLatin1(buffer, size);
    free(buffer);
    return err;
}

template <size_t N> QByteArray raw(const char (&data)[N])
{
    return QByteArray::fromRawData(data, N - 1);
}

void tst_Parser::initParserEmpty()
{
    CborParser parser;
    CborValue first;
    CborError err = cbor_parser_init((const quint8 *)"", 0, 0, &parser, &first);
    QCOMPARE(err, CborErrorUnexpectedEOF);
}

void addColumns()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("n");         // some aux integer, not added in all columns
}

bool compareFailed = true;
void compareOne_real(const QByteArray &data, const QString &expected, int line, int n = -1)
{
    compareFailed = true;
    CborParser parser;
    CborValue first;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &first);
    QVERIFY2(!err, QByteArray::number(line) + ": Got error \"" + cbor_error_string(err) + "\"");

    if (cbor_value_get_type(&first) == CborArrayType) {
        size_t len;
        if (n >= 0) {
            QCOMPARE(cbor_value_get_array_length(&first, &len), CborNoError);
            QCOMPARE(len, size_t(len));
        } else {
            QCOMPARE(cbor_value_get_array_length(&first, &len), CborErrorUnknownLength);
        }
    } else if (cbor_value_get_type(&first) == CborMapType) {
        size_t len;
        if (n >= 0) {
            QCOMPARE(cbor_value_get_map_length(&first, &len), CborNoError);
            QCOMPARE(len, size_t(len));
        } else {
            QCOMPARE(cbor_value_get_map_length(&first, &len), CborErrorUnknownLength);
        }
    }

    QString decoded;
    err = parseOne(&first, &decoded);
    QVERIFY2(!err, QByteArray::number(line) + ": Got error \"" + cbor_error_string(err) +
                   "\"; decoded stream:\n" + decoded.toLatin1());
    QCOMPARE(decoded, expected);

    // check that we consumed everything
    QCOMPARE((void*)first.ptr, (void*)data.constEnd());

    compareFailed = false;
}
#define compareOne(data, expected) compareOne_real(data, expected, __LINE__)
#define compareOneSize(n, data, expected) compareOne_real(data, expected, __LINE__, n)

void addFixedData()
{
    // unsigned integers
    QTest::newRow("0") << raw("\x00") << "0";
    QTest::newRow("1") << raw("\x01") << "1";
    QTest::newRow("10") << raw("\x0a") << "10";
    QTest::newRow("23") << raw("\x17") << "23";
    QTest::newRow("24") << raw("\x18\x18") << "24";
    QTest::newRow("UINT8_MAX") << raw("\x18\xff") << "255";
    QTest::newRow("UINT8_MAX+1") << raw("\x19\x01\x00") << "256";
    QTest::newRow("UINT16_MAX") << raw("\x19\xff\xff") << "65535";
    QTest::newRow("UINT16_MAX+1") << raw("\x1a\0\1\x00\x00") << "65536";
    QTest::newRow("UINT32_MAX") << raw("\x1a\xff\xff\xff\xff") << "4294967295";
    QTest::newRow("UINT32_MAX+1") << raw("\x1b\0\0\0\1\0\0\0\0") << "4294967296";
    QTest::newRow("UINT64_MAX") << raw("\x1b" "\xff\xff\xff\xff" "\xff\xff\xff\xff")
                                << QString::number(std::numeric_limits<uint64_t>::max());

    // negative integers
    QTest::newRow("-1") << raw("\x20") << "-1";
    QTest::newRow("-2") << raw("\x21") << "-2";
    QTest::newRow("-24") << raw("\x37") << "-24";
    QTest::newRow("-25") << raw("\x38\x18") << "-25";
    QTest::newRow("-UINT8_MAX") << raw("\x38\xff") << "-256";
    QTest::newRow("-UINT8_MAX-1") << raw("\x39\x01\x00") << "-257";
    QTest::newRow("-UINT16_MAX") << raw("\x39\xff\xff") << "-65536";
    QTest::newRow("-UINT16_MAX-1") << raw("\x3a\0\1\x00\x00") << "-65537";
    QTest::newRow("-UINT32_MAX") << raw("\x3a\xff\xff\xff\xff") << "-4294967296";
    QTest::newRow("-UINT32_MAX-1") << raw("\x3b\0\0\0\1\0\0\0\0") << "-4294967297";
    QTest::newRow("INT64_MIN+1") << raw("\x3b\x7f\xff\xff\xff""\xff\xff\xff\xfe")
                               << QString::number(std::numeric_limits<int64_t>::min() + 1);
    QTest::newRow("INT64_MIN") << raw("\x3b\x7f\xff\xff\xff""\xff\xff\xff\xff")
                               << QString::number(std::numeric_limits<int64_t>::min());
    QTest::newRow("INT64_MIN-1") << raw("\x3b\x80\0\0\0""\0\0\0\0") << "-9223372036854775809";
    QTest::newRow("-UINT64_MAX") << raw("\x3b" "\xff\xff\xff\xff" "\xff\xff\xff\xfe")
                                   << '-' + QString::number(std::numeric_limits<uint64_t>::max());
    QTest::newRow("-UINT64_MAX+1") << raw("\x3b" "\xff\xff\xff\xff" "\xff\xff\xff\xff")
                                   << "-18446744073709551616";

    // overlongs
    QTest::newRow("0*1") << raw("\x18\x00") << "0";
    QTest::newRow("0*2") << raw("\x19\x00\x00") << "0";
    QTest::newRow("0*4") << raw("\x1a\0\0\0\0") << "0";
    QTest::newRow("0*8") << raw("\x1b\0\0\0\0\0\0\0\0") << "0";
    QTest::newRow("-1*1") << raw("\x38\x00") << "-1";
    QTest::newRow("-1*2") << raw("\x39\x00\x00") << "-1";
    QTest::newRow("-1*4") << raw("\x3a\0\0\0\0") << "-1";
    QTest::newRow("-1*8") << raw("\x3b\0\0\0\0\0\0\0\0") << "-1";

    QTest::newRow("simple0") << raw("\xe0") << "simple(0)";
    QTest::newRow("simple19") << raw("\xf3") << "simple(19)";
    QTest::newRow("false") << raw("\xf4") << "false";
    QTest::newRow("true") << raw("\xf5") << "true";
    QTest::newRow("null") << raw("\xf6") << "null";
    QTest::newRow("undefined") << raw("\xf7") << "undefined";
    QTest::newRow("simple32") << raw("\xf8\x20") << "simple(32)";
    QTest::newRow("simple255") << raw("\xf8\xff") << "simple(255)";

    // floating point

    QTest::newRow("0.f16") << raw("\xf9\0\0") << "0.f16";
    QTest::newRow("0.f") << raw("\xfa\0\0\0\0") << "0.f";
    QTest::newRow("0.")  << raw("\xfb\0\0\0\0\0\0\0\0") << "0.";
    QTest::newRow("-1.f16") << raw("\xf9\xbc\x00") << "-1.f16";
    QTest::newRow("-1.f") << raw("\xfa\xbf\x80\0\0") << "-1.f";
    QTest::newRow("-1.") << raw("\xfb\xbf\xf0\0\0\0\0\0\0") << "-1.";
    QTest::newRow("65504.f16") << raw("\xf9\x7b\xff") << "65504.f16";
    QTest::newRow("16777215.f") << raw("\xfa\x4b\x7f\xff\xff") << "16777215.f";
    QTest::newRow("16777215.") << raw("\xfb\x41\x6f\xff\xff\xe0\0\0\0") << "16777215.";
    QTest::newRow("-16777215.f") << raw("\xfa\xcb\x7f\xff\xff") << "-16777215.f";
    QTest::newRow("-16777215.") << raw("\xfb\xc1\x6f\xff\xff\xe0\0\0\0") << "-16777215.";

    QTest::newRow("0.5f16") << raw("\xf9\x38\0") << "0.5f16";
    QTest::newRow("0.5f") << raw("\xfa\x3f\0\0\0") << "0.5f";
    QTest::newRow("0.5") << raw("\xfb\x3f\xe0\0\0\0\0\0\0") << "0.5";
    QTest::newRow("2.f16^11-1") << raw("\xf9\x67\xff") << "2047.f16";
    QTest::newRow("2.f^24-1") << raw("\xfa\x4b\x7f\xff\xff") << "16777215.f";
    QTest::newRow("2.^53-1") << raw("\xfb\x43\x3f\xff\xff""\xff\xff\xff\xff") << "9007199254740991.";
    QTest::newRow("2.f^64-epsilon") << raw("\xfa\x5f\x7f\xff\xff") << "18446742974197923840.f";
    QTest::newRow("2.^64-epsilon") << raw("\xfb\x43\xef\xff\xff""\xff\xff\xff\xff") << "18446744073709549568.";
    QTest::newRow("2.f^64") << raw("\xfa\x5f\x80\0\0") << "1.8446744073709552e+19f";
    QTest::newRow("2.^64") << raw("\xfb\x43\xf0\0\0\0\0\0\0") << "1.8446744073709552e+19";

    QTest::newRow("nan_f16") << raw("\xf9\x7e\x00") << "nan";
    QTest::newRow("nan_f") << raw("\xfa\x7f\xc0\0\0") << "nan";
    QTest::newRow("nan") << raw("\xfb\x7f\xf8\0\0\0\0\0\0") << "nan";
    QTest::newRow("-inf_f16") << raw("\xf9\xfc\x00") << "-inf";
    QTest::newRow("-inf_f") << raw("\xfa\xff\x80\0\0") << "-inf";
    QTest::newRow("-inf") << raw("\xfb\xff\xf0\0\0\0\0\0\0") << "-inf";
    QTest::newRow("+inf_f16") << raw("\xf9\x7c\x00") << "inf";
    QTest::newRow("+inf_f") << raw("\xfa\x7f\x80\0\0") << "inf";
    QTest::newRow("+inf") << raw("\xfb\x7f\xf0\0\0\0\0\0\0") << "inf";

}

void tst_Parser::fixed_data()
{
    addColumns();
    addFixedData();
}

void tst_Parser::fixed()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    compareOne(data, expected);
}

void addStringsData()
{
    // byte strings
    QTest::newRow("emptybytestring") << raw("\x40") << "h''";
    QTest::newRow("bytestring1") << raw("\x41 ") << "h'20'";
    QTest::newRow("bytestring1-nul") << raw("\x41\0") << "h'00'";
    QTest::newRow("bytestring5") << raw("\x45Hello") << "h'48656c6c6f'";
    QTest::newRow("bytestring24") << raw("\x58\x18""123456789012345678901234")
                                  << "h'313233343536373839303132333435363738393031323334'";
    QTest::newRow("bytestring256") << raw("\x59\1\0") + QByteArray(256, '3')
                                   << "h'" + QString(256 * 2, '3') + '\'';

    // text strings
    QTest::newRow("emptytextstring") << raw("\x60") << "\"\"";
    QTest::newRow("textstring1") << raw("\x61 ") << "\" \"";
    QTest::newRow("textstring1-nul") << raw("\x61\0") << "\"\\u0000\"";
    QTest::newRow("textstring5") << raw("\x65Hello") << "\"Hello\"";
    QTest::newRow("textstring24") << raw("\x78\x18""123456789012345678901234")
                                  << "\"123456789012345678901234\"";
    QTest::newRow("textstring256") << raw("\x79\1\0") + QByteArray(256, '3')
                                   << '"' + QString(256, '3') + '"';

    // strings with overlong length
    QTest::newRow("emptybytestring*1") << raw("\x58\x00") << "h''";
    QTest::newRow("emptytextstring*1") << raw("\x78\x00") << "\"\"";
    QTest::newRow("emptybytestring*2") << raw("\x59\x00\x00") << "h''";
    QTest::newRow("emptytextstring*2") << raw("\x79\x00\x00") << "\"\"";
    QTest::newRow("emptybytestring*4") << raw("\x5a\0\0\0\0") << "h''";
    QTest::newRow("emptytextstring*4") << raw("\x7a\0\0\0\0") << "\"\"";
    QTest::newRow("emptybytestring*8") << raw("\x5b\0\0\0\0\0\0\0\0") << "h''";
    QTest::newRow("emptytextstring*8") << raw("\x7b\0\0\0\0\0\0\0\0") << "\"\"";
    QTest::newRow("bytestring5*1") << raw("\x58\x05Hello") << "h'48656c6c6f'";
    QTest::newRow("textstring5*1") << raw("\x78\x05Hello") << "\"Hello\"";
    QTest::newRow("bytestring5*2") << raw("\x59\0\5Hello") << "h'48656c6c6f'";
    QTest::newRow("textstring5*2") << raw("\x79\0\x05Hello") << "\"Hello\"";
    QTest::newRow("bytestring5*4") << raw("\x5a\0\0\0\5Hello") << "h'48656c6c6f'";
    QTest::newRow("textstring5*4") << raw("\x7a\0\0\0\x05Hello") << "\"Hello\"";
    QTest::newRow("bytestring5*8") << raw("\x5b\0\0\0\0\0\0\0\5Hello") << "h'48656c6c6f'";
    QTest::newRow("textstring5*8") << raw("\x7b\0\0\0\0\0\0\0\x05Hello") << "\"Hello\"";

    // strings with undefined length
    QTest::newRow("_emptybytestring") << raw("\x5f\xff") << "h''";
    QTest::newRow("_emptytextstring") << raw("\x7f\xff") << "\"\"";
    QTest::newRow("_emptybytestring2") << raw("\x5f\x40\xff") << "h''";
    QTest::newRow("_emptytextstring2") << raw("\x7f\x60\xff") << "\"\"";
    QTest::newRow("_emptybytestring3") << raw("\x5f\x40\x40\xff") << "h''";
    QTest::newRow("_emptytextstring3") << raw("\x7f\x60\x60\xff") << "\"\"";
    QTest::newRow("_bytestring5*2") << raw("\x5f\x43Hel\x42lo\xff") << "h'48656c6c6f'";
    QTest::newRow("_textstring5*2") << raw("\x7f\x63Hel\x62lo\xff") << "\"Hello\"";
    QTest::newRow("_bytestring5*5") << raw("\x5f\x41H\x41""e\x41l\x41l\x41o\xff") << "h'48656c6c6f'";
    QTest::newRow("_textstring5*5") << raw("\x7f\x61H\x61""e\x61l\x61l\x61o\xff") << "\"Hello\"";
    QTest::newRow("_bytestring5*6") << raw("\x5f\x41H\x41""e\x40\x41l\x41l\x41o\xff") << "h'48656c6c6f'";
    QTest::newRow("_textstring5*6") << raw("\x7f\x61H\x61""e\x61l\x60\x61l\x61o\xff") << "\"Hello\"";
}

void tst_Parser::strings_data()
{
    addColumns();
    addStringsData();
}

void addTagsData()
{
    // since parseOne() works recursively for tags, we can't test lone tags
    QTest::newRow("tag0") << raw("\xc0\x00") << "0(0)";
    QTest::newRow("tag1") << raw("\xc1\x00") << "1(0)";
    QTest::newRow("tag24") << raw("\xd8\x18\x00") << "24(0)";
    QTest::newRow("tag255") << raw("\xd8\xff\x00") << "255(0)";
    QTest::newRow("tag256") << raw("\xd9\1\0\x00") << "256(0)";
    QTest::newRow("tag65535") << raw("\xd9\xff\xff\x00") << "65535(0)";
    QTest::newRow("tag65536") << raw("\xda\0\1\0\0\x00") << "65536(0)";
    QTest::newRow("tagUINT32_MAX-1") << raw("\xda\xff\xff\xff\xff\x00") << "4294967295(0)";
    QTest::newRow("tagUINT32_MAX") << raw("\xdb\0\0\0\1\0\0\0\0\x00") << "4294967296(0)";
    QTest::newRow("tagUINT64_MAX") << raw("\xdb" "\xff\xff\xff\xff" "\xff\xff\xff\xff" "\x00")
                                << QString::number(std::numeric_limits<uint64_t>::max()) + "(0)";

    // overlong tags
    QTest::newRow("tag0*1") << raw("\xd8\0\x00") << "0(0)";
    QTest::newRow("tag0*2") << raw("\xd9\0\0\x00") << "0(0)";
    QTest::newRow("tag0*4") << raw("\xda\0\0\0\0\x00") << "0(0)";
    QTest::newRow("tag0*8") << raw("\xdb\0\0\0\0\0\0\0\0\x00") << "0(0)";

    // tag other things
    QTest::newRow("unixtime") << raw("\xc1\x1a\x55\x4b\xbf\xd3") << "1(1431027667)";
    QTest::newRow("rfc3339date") << raw("\xc0\x78\x19" "2015-05-07 12:41:07-07:00")
                                 << "0(\"2015-05-07 12:41:07-07:00\")";
    QTest::newRow("tag6+false") << raw("\xc6\xf4") << "6(false)";
    QTest::newRow("tag25+true") << raw("\xd8\x19\xf5") << "25(true)";
    QTest::newRow("tag256+null") << raw("\xd9\1\0\xf6") << "256(null)";
    QTest::newRow("tag65536+simple32") << raw("\xda\0\1\0\0\xf8\x20") << "65536(simple(32))";
    QTest::newRow("float+unixtime") << raw("\xc1\xfa\x4e\xaa\x97\x80") << "1(1431027712.f)";
    QTest::newRow("double+unixtime") << raw("\xc1\xfb" "\x41\xd5\x52\xef" "\xf4\xc7\xce\xfe")
                                     << "1(1431027667.1220088)";
}

void tst_Parser::tags_data()
{
    addColumns();
    addTagsData();
}

void tst_Parser::tagTags()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    compareOne("\xd9\xd9\xf7" + data, "55799(" + expected + ')');
    if (!compareFailed)
        compareOne("\xd9\xd9\xf7" "\xd9\xd9\xf7" + data, "55799(55799(" + expected + "))");
}

void addEmptyContainersData()
{
    QTest::newRow("emptyarray") << raw("\x80") << "[]" << 0;
    QTest::newRow("emptymap") << raw("\xa0") << "{}" << 0;
    QTest::newRow("_emptyarray") << raw("\x9f\xff") << "[_ ]" << -1;
    QTest::newRow("_emptymap") << raw("\xbf\xff") << "{_ }" << -1;
}

void tst_Parser::emptyContainers_data()
{
    addColumns();
    addEmptyContainersData();
}

void tst_Parser::emptyContainers()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);
    QFETCH(int, n);

    compareOneSize(n, data, expected);
}

void tst_Parser::arrays_data()
{
    addColumns();
    addFixedData();
    addStringsData();
    addTagsData();
}

void tst_Parser::arrays()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    compareOneSize(1, "\x81" + data, '[' + expected + ']');
    if (compareFailed) return;

    compareOneSize(2, "\x82" + data + data, '[' + expected + ", " + expected + ']');
    if (compareFailed) return;

    // overlong length
    compareOneSize(1, "\x98\1" + data, '[' + expected + ']');
    if (compareFailed) return;
    compareOneSize(1, raw("\x99\0\1") + data, '[' + expected + ']');
    if (compareFailed) return;
    compareOneSize(1, raw("\x9a\0\0\0\1") + data, '[' + expected + ']');
    if (compareFailed) return;
    compareOneSize(1, raw("\x9b\0\0\0\0\0\0\0\1") + data, '[' + expected + ']');
    if (compareFailed) return;

    // medium-sized array: 32 elements (1 << 5)
    expected += ", ";
    for (int i = 0; i < 5; ++i) {
        data += data;
        expected += expected;
    }
    expected.chop(2);   // remove the last ", "
    compareOneSize(32, "\x98\x20" + data, '[' + expected + ']');
    if (compareFailed) return;

    // large array: 256 elements (32 << 3)
    expected += ", ";
    for (int i = 0; i < 3; ++i) {
        data += data;
        expected += expected;
    }
    expected.chop(2);   // remove the last ", "
    compareOneSize(256, raw("\x99\1\0") + data, '[' + expected + ']');
    if (compareFailed) return;
}

void tst_Parser::undefLengthArrays()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    compareOne("\x9f" + data + "\xff", "[_ " + expected + ']');
    if (compareFailed) return;

    compareOne("\x9f" + data + data + "\xff", "[_ " + expected + ", " + expected + ']');
}

void tst_Parser::nestedArrays()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    compareOneSize(1, "\x81\x81" + data, "[[" + expected + "]]");
    if (compareFailed) return;

    compareOneSize(1, "\x81\x81\x81" + data, "[[[" + expected + "]]]");
    if (compareFailed) return;

    compareOneSize(1, "\x81\x82" + data + data, "[[" + expected + ", " + expected + "]]");
    if (compareFailed) return;

    compareOneSize(2, "\x82\x81" + data + data, "[[" + expected + "], " + expected + "]");
    if (compareFailed) return;

    compareOneSize(2, "\x82\x81" + data + '\x81' + data, "[[" + expected + "], [" + expected + "]]");
    if (compareFailed) return;

    // undefined length
    compareOneSize(-1, "\x9f\x9f" + data + data + "\xff\xff", "[_ [_ " + expected + ", " + expected + "]]");
    if (compareFailed) return;

    compareOneSize(-1, "\x9f\x9f" + data + "\xff\x9f" + data + "\xff\xff", "[_ [_ " + expected + "], [_ " + expected + "]]");
    if (compareFailed) return;

    compareOneSize(-1, "\x9f\x9f" + data + data + "\xff\x9f" + data + "\xff\xff",
               "[_ [_ " + expected + ", " + expected + "], [_ " + expected + "]]");
    if (compareFailed) return;

    // mix them
    compareOneSize(1, "\x81\x9f" + data + "\xff", "[[_ " + expected + "]]");
    if (compareFailed) return;

    compareOneSize(-1, "\x9f\x81" + data + "\xff", "[_ [" + expected + "]]");
}

void tst_Parser::maps_data()
{
    arrays_data();
}

void tst_Parser::maps()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    // integer key
    compareOneSize(1, "\xa1\1" + data, "{1: " + expected + '}');
    if (compareFailed) return;

    // string key
    compareOneSize(1, "\xa1\x65" "Hello" + data, "{\"Hello\": " + expected + '}');
    if (compareFailed) return;

    // map to self
    compareOneSize(1, "\xa1" + data + data, '{' + expected + ": " + expected + '}');
    if (compareFailed) return;

    // two integer keys
    compareOneSize(2, "\xa2\1" + data + "\2" + data, "{1: " + expected + ", 2: " + expected + '}');
    if (compareFailed) return;

    // OneSize integer and OneSize string key
    compareOneSize(2, "\xa2\1" + data + "\x65" "Hello" + data, "{1: " + expected + ", \"Hello\": " + expected + '}');
    if (compareFailed) return;
}

void tst_Parser::undefLengthMaps()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    // integer key
    compareOne("\xbf\1" + data + '\xff', "{_ 1: " + expected + '}');
    if (compareFailed) return;

    compareOne("\xbf\1" + data + '\2' + data + '\xff', "{_ 1: " + expected + ", 2: " + expected + '}');
    if (compareFailed) return;

    compareOne("\xbf\1" + data + "\x65Hello" + data + '\xff', "{_ 1: " + expected + ", \"Hello\": " + expected + '}');
    if (compareFailed) return;

    compareOne("\xbf\x65Hello" + data + '\1' + data + '\xff', "{_ \"Hello\": " + expected + ", 1: " + expected + '}');
}

void tst_Parser::nestedMaps()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    // nested maps as values
    compareOneSize(1, "\xa1\1\xa1\2" + data, "{1: {2: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\x65Hello\xa1\2" + data, "{\"Hello\": {2: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\1\xa2\2" + data + '\x20' + data, "{1: {2: " + expected + ", -1: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(2, "\xa2\1\xa1\2" + data + "\2\xa1\x20" + data, "{1: {2: " + expected + "}, 2: {-1: " + expected + "}}");
    if (compareFailed) return;

    // nested maps as keys
    compareOneSize(1, "\xa1\xa1\xf4" + data + "\xf5", "{{false: " + expected + "}: true}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\xa1" + data + data + "\xa1" + data + data,
               "{{" + expected + ": " + expected + "}: {" + expected + ": " + expected + "}}");
    if (compareFailed) return;

    // undefined length
    compareOneSize(-1, "\xbf\1\xbf\2" + data + "\xff\xff", "{_ 1: {_ 2: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\xbf\2" + data + '\x20' + data + "\xff\xff", "{_ 1: {_ 2: " + expected + ", -1: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\xbf\2" + data + "\xff\2\xbf\x20" + data + "\xff\xff",
               "{_ 1: {_ 2: " + expected + "}, 2: {_ -1: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\xbf" + data + data + "\xff\xbf" + data + data + "\xff\xff",
               "{_ {_ " + expected + ": " + expected + "}: {_ " + expected + ": " + expected + "}}");
    if (compareFailed) return;

    // mix them
    compareOneSize(1, "\xa1\1\xbf\2" + data + "\xff", "{1: {_ 2: " + expected + "}}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\xa1\2" + data + "\xff", "{_ 1: {2: " + expected + "}}");
    if (compareFailed) return;
}

void addMapMixedData()
{
    QTest::newRow("map-0-24") << raw("\xa1\0\x18\x18") << "{0: 24}" << 1;
    QTest::newRow("map-0*1-24") << raw("\xa1\x18\0\x18\x18") << "{0: 24}" << 1;
    QTest::newRow("map-0*1-24*2") << raw("\xa1\x18\0\x19\0\x18") << "{0: 24}" << 1;
    QTest::newRow("map-0*4-24*2") << raw("\xa1\x1a\0\0\0\0\x19\0\x18") << "{0: 24}" << 1;
    QTest::newRow("map-24-0") << raw("\xa1\x18\x18\0") << "{24: 0}" << 1;
    QTest::newRow("map-24-0*1") << raw("\xa1\x18\x18\0") << "{24: 0}" << 1;
    QTest::newRow("map-255-65535") << raw("\xa1\x18\xff\x19\xff\xff") << "{255: 65535}" << 1;

    QTest::newRow("_map-0-24") << raw("\xbf\0\x18\x18\xff") << "{_ 0: 24}" << 1;
    QTest::newRow("_map-0*1-24") << raw("\xbf\x18\0\x18\x18\xff") << "{_ 0: 24}" << 1;
    QTest::newRow("_map-0*1-24*2") << raw("\xbf\x18\0\x19\0\x18\xff") << "{_ 0: 24}" << 1;
    QTest::newRow("_map-0*4-24*2") << raw("\xbf\x1a\0\0\0\0\x19\0\x18\xff") << "{_ 0: 24}" << 1;
    QTest::newRow("_map-24-0") << raw("\xbf\x18\x18\0\xff") << "{_ 24: 0}" << 1;
    QTest::newRow("_map-24-0*1") << raw("\xbf\x18\x18\0\xff") << "{_ 24: 0}" << 1;
    QTest::newRow("_map-255-65535") << raw("\xbf\x18\xff\x19\xff\xff\xff") << "{_ 255: 65535}" << 1;
}

void tst_Parser::mapMixed_data()
{
    addColumns();
    addMapMixedData();
}

void tst_Parser::mapsAndArrays()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    // arrays of maps
    compareOneSize(1, "\x81\xa1\1" + data, "[{1: " + expected + "}]");
    if (compareFailed) return;

    compareOneSize(2, "\x82\xa1\1" + data + "\xa1\2" + data, "[{1: " + expected + "}, {2: " + expected + "}]");
    if (compareFailed) return;

    compareOneSize(1, "\x81\xa2\1" + data + "\2" + data, "[{1: " + expected + ", 2: " + expected + "}]");
    if (compareFailed) return;

    compareOneSize(-1, "\x9f\xa1\1" + data + "\xff", "[_ {1: " + expected + "}]");
    if (compareFailed) return;

    compareOneSize(1, "\x81\xbf\1" + data + "\xff", "[{_ 1: " + expected + "}]");
    if (compareFailed) return;

    compareOneSize(-1, "\x9f\xbf\1" + data + "\xff\xff", "[_ {_ 1: " + expected + "}]");
    if (compareFailed) return;

    // maps of arrays
    compareOneSize(1, "\xa1\1\x81" + data, "{1: [" + expected + "]}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\1\x82" + data + data, "{1: [" + expected + ", " + expected + "]}");
    if (compareFailed) return;

    compareOneSize(2, "\xa2\1\x81" + data + "\x65Hello\x81" + data, "{1: [" + expected + "], \"Hello\": [" + expected + "]}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\1\x9f" + data + "\xff", "{1: [_ " + expected + "]}");
    if (compareFailed) return;

    compareOneSize(1, "\xa1\1\x9f" + data + data + "\xff", "{1: [_ " + expected + ", " + expected + "]}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\x81" + data + "\xff", "{_ 1: [" + expected + "]}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\x9f" + data + "\xff\xff", "{_ 1: [_ " + expected + "]}");
    if (compareFailed) return;

    compareOneSize(-1, "\xbf\1\x9f" + data + data + "\xff\xff", "{_ 1: [_ " + expected + ", " + expected + "]}");
    if (compareFailed) return;

    // mixed with indeterminate length strings
    compareOneSize(-1, "\xbf\1\x9f" + data + "\xff\x65Hello\xbf" + data + "\x7f\xff\xff\xff",
                   "{_ 1: [_ " + expected + "], \"Hello\": {_ " + expected + ": \"\"}}");
}

void tst_Parser::stringLength_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("expected");

    QTest::newRow("emptybytestring") << raw("\x40") << 0;
    QTest::newRow("bytestring1") << raw("\x41 ") << 1;
    QTest::newRow("bytestring1-nul") << raw("\x41\0") << 1;
    QTest::newRow("bytestring5") << raw("\x45Hello") << 5;
    QTest::newRow("bytestring24") << raw("\x58\x18""123456789012345678901234") << 24;
    QTest::newRow("bytestring256") << raw("\x59\1\0") + QByteArray(256, '3') << 256;

    // text strings
    QTest::newRow("emptytextstring") << raw("\x60") << 0;
    QTest::newRow("textstring1") << raw("\x61 ") << 1;
    QTest::newRow("textstring1-nul") << raw("\x61\0") << 1;
    QTest::newRow("textstring5") << raw("\x65Hello") << 5;
    QTest::newRow("textstring24") << raw("\x78\x18""123456789012345678901234") << 24;
    QTest::newRow("textstring256") << raw("\x79\1\0") + QByteArray(256, '3') << 256;

    // strings with overlong length
    QTest::newRow("emptybytestring*1") << raw("\x58\x00") << 0;
    QTest::newRow("emptytextstring*1") << raw("\x78\x00") << 0;
    QTest::newRow("emptybytestring*2") << raw("\x59\x00\x00") << 0;
    QTest::newRow("emptytextstring*2") << raw("\x79\x00\x00") << 0;
    QTest::newRow("emptybytestring*4") << raw("\x5a\0\0\0\0") << 0;
    QTest::newRow("emptytextstring*4") << raw("\x7a\0\0\0\0") << 0;
    QTest::newRow("emptybytestring*8") << raw("\x5b\0\0\0\0\0\0\0\0") << 0;
    QTest::newRow("emptytextstring*8") << raw("\x7b\0\0\0\0\0\0\0\0") << 0;
    QTest::newRow("bytestring5*1") << raw("\x58\x05Hello") << 5;
    QTest::newRow("textstring5*1") << raw("\x78\x05Hello") << 5;
    QTest::newRow("bytestring5*2") << raw("\x59\0\5Hello") << 5;
    QTest::newRow("textstring5*2") << raw("\x79\0\x05Hello") << 5;
    QTest::newRow("bytestring5*4") << raw("\x5a\0\0\0\5Hello") << 5;
    QTest::newRow("textstring5*4") << raw("\x7a\0\0\0\x05Hello") << 5;
    QTest::newRow("bytestring5*8") << raw("\x5b\0\0\0\0\0\0\0\5Hello") << 5;
    QTest::newRow("textstring5*8") << raw("\x7b\0\0\0\0\0\0\0\x05Hello") << 5;

    // strings with undefined length
    QTest::newRow("_emptybytestring") << raw("\x5f\xff") << 0;
    QTest::newRow("_emptytextstring") << raw("\x7f\xff") << 0;
    QTest::newRow("_emptybytestring2") << raw("\x5f\x40\xff") << 0;
    QTest::newRow("_emptytextstring2") << raw("\x7f\x60\xff") << 0;
    QTest::newRow("_emptybytestring3") << raw("\x5f\x40\x40\xff") << 0;
    QTest::newRow("_emptytextstring3") << raw("\x7f\x60\x60\xff") << 0;
    QTest::newRow("_bytestring5*2") << raw("\x5f\x43Hel\x42lo\xff") << 5;
    QTest::newRow("_textstring5*2") << raw("\x7f\x63Hel\x62lo\xff") << 5;
    QTest::newRow("_bytestring5*5") << raw("\x5f\x41H\x41""e\x41l\x41l\x41o\xff") << 5;
    QTest::newRow("_textstring5*5") << raw("\x7f\x61H\x61""e\x61l\x61l\x61o\xff") << 5;
    QTest::newRow("_bytestring5*6") << raw("\x5f\x41H\x41""e\x40\x41l\x41l\x41o\xff") << 5;
    QTest::newRow("_textstring5*6") << raw("\x7f\x61H\x61""e\x61l\x60\x61l\x61o\xff") << 5;
}

void tst_Parser::stringLength()
{
    QFETCH(QByteArray, data);
    QFETCH(int, expected);

    CborParser parser;
    CborValue value;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &value);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    size_t result;
    err = cbor_value_calculate_string_length(&value, &result);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");
    QCOMPARE(result, size_t(expected));
}

void tst_Parser::stringCompare_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QString>("string");
    QTest::addColumn<bool>("expected");

    // compare empty to empty
    QTest::newRow("empty-empty") << raw("\x60") << QString() << true;
    QTest::newRow("_empty-empty") << raw("\x7f\xff") << QString() << true;
    QTest::newRow("_empty*1-empty") << raw("\x7f\x60\xff") << QString() << true;
    QTest::newRow("_empty*2-empty") << raw("\x7f\x60\x60\xff") << QString() << true;

    // compare empty to non-empty
    QTest::newRow("empty-nonempty") << raw("\x60") << "Hello" << false;
    QTest::newRow("_empty-nonempty") << raw("\x7f\xff") << "Hello" << false;
    QTest::newRow("_empty*1-nonempty") << raw("\x7f\x60\xff") << "Hello" << false;
    QTest::newRow("_empty*2-nonempty") << raw("\x7f\x60\x60\xff") << "Hello" << false;

    // compare same strings
    QTest::newRow("same-short-short") << raw("\x65Hello") << "Hello" << true;
    QTest::newRow("same-_short*1-short") << raw("\x7f\x65Hello\xff") << "Hello" << true;
    QTest::newRow("same-_short*2-short") << raw("\x7f\x63Hel\x62lo\xff") << "Hello" << true;
    QTest::newRow("same-_short*5-short") << raw("\x7f\x61H\x61""e\x61l\x61l\x61o\xff") << "Hello" << true;
    QTest::newRow("same-_short*8-short") << raw("\x7f\x61H\x60\x61""e\x60\x61l\x61l\x60\x61o\xff") << "Hello" << true;
    QTest::newRow("same-long-long") << raw("\x78\x2aGood morning, good afternoon and goodnight")
                                    << "Good morning, good afternoon and goodnight" << true;
    QTest::newRow("same-_long*1-long") << raw("\x7f\x78\x2aGood morning, good afternoon and goodnight\xff")
                                       << "Good morning, good afternoon and goodnight" << true;
    QTest::newRow("same-_long*2-long") << raw("\x7f\x78\x1cGood morning, good afternoon\x6e and goodnight\xff")
                                       << "Good morning, good afternoon and goodnight" << true;

    // compare different strings (same length)
    QTest::newRow("diff-same-length-short-short") << raw("\x65Hello") << "World" << false;
    QTest::newRow("diff-same-length-_short*1-short") << raw("\x7f\x65Hello\xff") << "World" << false;
    QTest::newRow("diff-same-length-_short*2-short") << raw("\x7f\x63Hel\x62lo\xff") << "World" << false;
    QTest::newRow("diff-same-length-_short*5-short") << raw("\x7f\x61H\x61""e\x61l\x61l\x61o\xff") << "World" << false;
    QTest::newRow("diff-same-length-_short*8-short") << raw("\x7f\x61H\x60\x61""e\x60\x61l\x61l\x60\x61o\xff") << "World" << false;
    QTest::newRow("diff-same-length-long-long") << raw("\x78\x2aGood morning, good afternoon and goodnight")
                                                << "Good morning, good afternoon and goodnight, world" << false;
    QTest::newRow("diff-same-length-_long*1-long") << raw("\x7f\x78\x2aGood morning, good afternoon and goodnight\xff")
                                                   << "Good morning, good afternoon and goodnight, world" << false;
    QTest::newRow("diff-same-length-_long*2-long") << raw("\x7f\x78\x1cGood morning, good afternoon\x6e and goodnight\xff")
                                                   << "Good morning, good afternoon and goodnight, world" << false;

    // compare different strings (different length)
    QTest::newRow("diff-diff-length-short-short") << raw("\x65Hello") << "Hello World" << false;
    QTest::newRow("diff-diff-length-_short*1-short") << raw("\x7f\x65Hello\xff") << "Hello World" << false;
    QTest::newRow("diff-diff-length-_short*2-short") << raw("\x7f\x63Hel\x62lo\xff") << "Hello World" << false;
    QTest::newRow("diff-diff-length-_short*5-short") << raw("\x7f\x61H\x61""e\x61l\x61l\x61o\xff") << "Hello World" << false;
    QTest::newRow("diff-diff-length-_short*8-short") << raw("\x7f\x61H\x60\x61""e\x60\x61l\x61l\x60\x61o\xff") << "Hello World" << false;
    QTest::newRow("diff-diff-length-long-long") << raw("\x78\x2aGood morning, good afternoon and goodnight")
                                                << "Good morning, good afternoon and goodnight World" << false;
    QTest::newRow("diff-diff-length-_long*1-long") << raw("\x7f\x78\x2aGood morning, good afternoon and goodnight\xff")
                                                   << "Good morning, good afternoon and goodnight World" << false;
    QTest::newRow("diff-diff-length-_long*2-long") << raw("\x7f\x78\x1cGood morning, good afternoon\x6e and goodnight\xff")
                                                   << "Good morning, good afternoon and goodnight World" << false;

    // compare against non-strings
    QTest::newRow("unsigned") << raw("\0") << "0" << false;
    QTest::newRow("negative") << raw("\x20") << "-1" << false;
    QTest::newRow("emptybytestring") << raw("\x40") << "" << false;
    QTest::newRow("_emptybytestring") << raw("\x5f\xff") << "" << false;
    QTest::newRow("shortbytestring") << raw("\x45Hello") << "Hello" << false;
    QTest::newRow("longbytestring") << raw("\x58\x2aGood morning, good afternoon and goodnight")
                                    << "Good morning, good afternoon and goodnight" << false;
    QTest::newRow("emptyarray") << raw("\x80") << "" << false;
    QTest::newRow("emptymap") << raw("\xa0") << "" << false;
    QTest::newRow("array") << raw("\x81\x65Hello") << "Hello" << false;
    QTest::newRow("map") << raw("\xa1\x65Hello\x65World") << "Hello World" << false;
    QTest::newRow("false") << raw("\xf4") << "false" << false;
    QTest::newRow("true") << raw("\xf5") << "true" << false;
    QTest::newRow("null") << raw("\xf6") << "null" << false;
}

void compareOneString(const QByteArray &data, const QString &string, bool expected, int line)
{
    compareFailed = true;

    CborParser parser;
    CborValue value;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &value);
    QVERIFY2(!err, QByteArray::number(line) + ": Got error \"" + cbor_error_string(err) + "\"");

    bool result;
    err = cbor_value_text_string_equals(&value, string.toUtf8().constData(), &result);
    QVERIFY2(!err, QByteArray::number(line) + ": Got error \"" + cbor_error_string(err) + "\"");
    QCOMPARE(result, expected);

    compareFailed = false;
}
#define compareOneString(data, string, expected) compareOneString(data, string, expected, __LINE__)

void tst_Parser::stringCompare()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, string);
    QFETCH(bool, expected);

    compareOneString(data, string, expected);
    if (compareFailed) return;

    // tag it
    compareOneString("\xc1" + data, string, expected);
    if (compareFailed) return;

    compareOneString("\xc1\xc2" + data, string, expected);
}

void tst_Parser::mapFind_data()
{
    // Rules:
    //  we are searching for string "needle"
    //  if present, the value should be the string "haystack" (with tag 42)

    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("expected");

    QTest::newRow("emptymap") << raw("\xa0") << false;
    QTest::newRow("_emptymap") << raw("\xbf\xff") << false;

    // maps not containing our items
    QTest::newRow("absent-unsigned-unsigned") << raw("\xa1\0\0") << false;
    QTest::newRow("absent-taggedunsigned-unsigned") << raw("\xa1\xc0\0\0") << false;
    QTest::newRow("absent-unsigned-taggedunsigned") << raw("\xa1\0\xc0\0") << false;
    QTest::newRow("absent-taggedunsigned-taggedunsigned") << raw("\xa1\xc0\0\xc0\0") << false;
    QTest::newRow("absent-string-unsigned") << raw("\xa1\x68haystack\0") << false;
    QTest::newRow("absent-taggedstring-unsigned") << raw("\xa1\xc0\x68haystack\0") << false;
    QTest::newRow("absent-string-taggedunsigned") << raw("\xa1\x68haystack\xc0\0") << false;
    QTest::newRow("absent-taggedstring-taggedunsigned") << raw("\xa1\xc0\x68haystack\xc0\0") << false;
    QTest::newRow("absent-string-string") << raw("\xa1\x68haystack\x66needle") << false;
    QTest::newRow("absent-string-taggedstring") << raw("\xa1\x68haystack\xc0\x66needle") << false;
    QTest::newRow("absent-taggedstring-string") << raw("\xa1\xc0\x68haystack\x66needle") << false;
    QTest::newRow("absent-string-taggedstring") << raw("\xa1\xc0\x68haystack\xc0\x66needle") << false;

    QTest::newRow("absent-string-emptyarray") << raw("\xa1\x68haystack\x80") << false;
    QTest::newRow("absent-string-_emptyarray") << raw("\xa1\x68haystack\x9f\xff") << false;
    QTest::newRow("absent-string-array1") << raw("\xa1\x68haystack\x81\0") << false;
    QTest::newRow("absent-string-array2") << raw("\xa1\x68haystack\x85\0\1\2\3\4") << false;
    QTest::newRow("absent-string-array3") << raw("\xa1\x68haystack\x85\x63one\x63two\x65three\x64""four\x64""five") << false;

    QTest::newRow("absent-string-emptymap") << raw("\xa1\x68haystack\xa0") << false;
    QTest::newRow("absent-string-_emptymap") << raw("\xa1\x68haystack\xbf\xff") << false;
    QTest::newRow("absent-string-map") << raw("\xa1\x68haystack\xa1\x68haystack\x66needle") << false;
    QTest::newRow("absent-string-map2") << raw("\xa1\x68haystack\xa1\x68haystack\x66needle\61z\62yx") << false;

    // maps containing our items
    QTest::newRow("alone") << raw("\xa1\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("tagged") << raw("\xa1\xc1\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("doubletagged") << raw("\xa1\xc1\xc2\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("chunked") << raw("\xa1\x7f\x66needle\xff\xd8\x2a\x68haystack") << true;
    QTest::newRow("chunked*2") << raw("\xa1\x7f\x60\x66needle\xff\xd8\x2a\x68haystack") << true;
    QTest::newRow("chunked*2bis") << raw("\xa1\x7f\x66needle\x60\xff\xd8\x2a\x68haystack") << true;
    QTest::newRow("chunked*3") << raw("\xa1\x7f\x62ne\x62""ed\x62le\xff\xd8\x2a\x68haystack") << true;
    QTest::newRow("chunked*8") << raw("\xa1\x7f\x61n\x61""e\x60\x61""e\x61""d\x60\x62le\x60\xff\xd8\x2a\x68haystack") << true;

    QTest::newRow("1before") << raw("\xa2\x68haystack\x66needle\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("tagged-1before") << raw("\xa2\xc1\x68haystack\x66needle\xc1\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("doubletagged-1before2") << raw("\xa2\xc1\xc2\x68haystack\x66needle\xc1\xc2\x66needle\xd8\x2a\x68haystack") << true;

    QTest::newRow("arraybefore") << raw("\xa2\x61z\x80\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("nestedarraybefore") << raw("\xa2\x61z\x81\x81\0\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("arrayarraybefore") << raw("\xa2\x82\1\2\x80\x66needle\xd8\x2a\x68haystack") << true;

    QTest::newRow("mapbefore") << raw("\xa2\x61z\xa0\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("nestedmapbefore") << raw("\xa2\x61z\xa1\0\x81\0\x66needle\xd8\x2a\x68haystack") << true;
    QTest::newRow("mapmapbefore") << raw("\xa2\xa1\1\2\xa0\x66needle\xd8\x2a\x68haystack") << true;
}

void tst_Parser::mapFind()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, expected);

    CborParser parser;
    CborValue value;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &value);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    CborValue element;
    err = cbor_value_map_find_value(&value, "needle", &element);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    if (expected) {
        QCOMPARE(int(element.type), int(CborTagType));

        CborTag tag;
        err = cbor_value_get_tag(&element, &tag);
        QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");
        QCOMPARE(int(tag), 42);

        bool equals;
        err = cbor_value_text_string_equals(&element, "haystack", &equals);
        QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");
        QVERIFY(equals);
    } else {
        QCOMPARE(int(element.type), int(CborInvalidType));
    }
}

void tst_Parser::validation_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("flags");     // future
    QTest::addColumn<CborError>("expectedError");
    QTest::addColumn<int>("offset");

    // illegal numbers are future extension points
    QTest::newRow("illegal-number-in-unsigned-1") << raw("\x81\x1c") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-unsigned-2") << raw("\x81\x1d") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-unsigned-3") << raw("\x81\x1e") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-unsigned-4") << raw("\x81\x1f") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-negative-1") << raw("\x81\x3c") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-negative-2") << raw("\x81\x3d") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-negative-3") << raw("\x81\x3e") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-negative-4") << raw("\x81\x3f") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-bytearray-length-1") << raw("\x81\x5c") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-bytearray-length-2") << raw("\x81\x5d") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-bytearray-length-3") << raw("\x81\x5e") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-string-length-1") << raw("\x81\x7c") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-string-length-2") << raw("\x81\x7d") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-string-length-3") << raw("\x81\x7e") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-array-length-1") << raw("\x81\x9c") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-array-length-2") << raw("\x81\x9d") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-array-length-3") << raw("\x81\x9e") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-map-length-1") << raw("\x81\xbc") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-map-length-2") << raw("\x81\xbd") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("illegal-number-in-map-length-3") << raw("\x81\xbe") << 0 << CborErrorIllegalNumber << 1;

    QTest::newRow("number-too-short-1-0") << raw("\x81\x18") << 0 << CborErrorUnexpectedEOF << 1;   // requires 1 byte, 0 given
    QTest::newRow("number-too-short-2-0") << raw("\x81\x19") << 0 << CborErrorUnexpectedEOF << 1;   // requires 2 bytes, 0 given
    QTest::newRow("number-too-short-2-1") << raw("\x81\x19\x01") << 0 << CborErrorUnexpectedEOF << 1; // etc
    QTest::newRow("number-too-short-4-0") << raw("\x81\x1a") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("number-too-short-4-3") << raw("\x81\x1a\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("number-too-short-8-0") << raw("\x81\x1b") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("number-too-short-8-7") << raw("\x81\x1b\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-1-0") << raw("\x81\x38") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-2-0") << raw("\x81\x39") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-2-1") << raw("\x81\x39\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-4-0") << raw("\x81\x3a") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-4-3") << raw("\x81\x3a\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-8-0") << raw("\x81\x3b") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-length-too-short-8-7") << raw("\x81\x3b\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-1-0") << raw("\x81\x58") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-2-0") << raw("\x81\x59") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-2-1") << raw("\x81\x59\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-4-0") << raw("\x81\x5a") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-4-3") << raw("\x81\x5a\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-8-0") << raw("\x81\x5b") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-length-too-short-8-7") << raw("\x81\x5b\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-1-0") << raw("\x81\x98") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-2-0") << raw("\x81\x99") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-2-1") << raw("\x81\x99\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-4-0") << raw("\x81\x9a") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-4-3") << raw("\x81\x9a\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-8-0") << raw("\x81\x9b") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("array-length-too-short-8-7") << raw("\x81\x9b\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-1-0") << raw("\x81\xb8") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-2-0") << raw("\x81\xb9") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-2-1") << raw("\x81\xb9\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-4-0") << raw("\x81\xba") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-4-3") << raw("\x81\xba\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-8-0") << raw("\x81\xbb") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("map-length-too-short-8-7") << raw("\x81\xbb\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-1-0") << raw("\x81\xd8") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-2-0") << raw("\x81\xd9") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-2-1") << raw("\x81\xd9\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-4-0") << raw("\x81\xda") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-4-3") << raw("\x81\xda\x01\x02\x03") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-8-0") << raw("\x81\xdb") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("tag-too-short-8-7") << raw("\x81\xdb\1\2\3\4\5\6\7") << 0 << CborErrorUnexpectedEOF << 1;

    QTest::newRow("bytearray-too-short1") << raw("\x81\x41") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short2") << raw("\x81\x58") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short3") << raw("\x81\x58\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short4") << raw("\x81\x59") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short5") << raw("\x81\x5a\0\0\0\1") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short6") << raw("\x81\x5a\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("bytearray-too-short7") << raw("\x81\x5b\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short1") << raw("\x81\x61") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short2") << raw("\x81\x78") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short3") << raw("\x81\x78\x01") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short4") << raw("\x81\x79") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short5") << raw("\x81\x7a\0\0\0\1") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short6") << raw("\x81\x7a\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("string-too-short7") << raw("\x81\x7b\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("fp16-too-short1") << raw("\x81\xf9") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("fp16-too-short2") << raw("\x81\xf9\x00") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("float-too-short1") << raw("\x81\xfa") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("float-too-short2") << raw("\x81\xfa\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("double-too-short1") << raw("\x81\xfb") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("double-too-short2") << raw("\x81\xfb\0\0\0\0\0\0\0") << 0 << CborErrorUnexpectedEOF << 1;

    QTest::newRow("eof-after-array") << raw("\x81") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("eof-after-array2") << raw("\x81\x78\x20") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("eof-after-array-element") << raw("\x81\x82\x01") << 0 << CborErrorUnexpectedEOF << 3;
    QTest::newRow("eof-after-object") << raw("\x81\xa1") << 0 << CborErrorUnexpectedEOF << 2;
    QTest::newRow("eof-after-object2") << raw("\x81\xb8\x20") << 0 << CborErrorUnexpectedEOF << 3;
    QTest::newRow("eof-after-object-key") << raw("\x81\xa1\x01") << 0 << CborErrorUnexpectedEOF << 3;
    QTest::newRow("eof-after-object-value") << raw("\x81\xa2\x01\x01") << 0 << CborErrorUnexpectedEOF << 4;
    QTest::newRow("eof-after-tag") << raw("\x81\xc0") << 0 << CborErrorUnexpectedEOF << 2;
    QTest::newRow("eof-after-tag2") << raw("\x81\xd8\x20") << 0 << CborErrorUnexpectedEOF << 3;

    // major type 7 has future types
    QTest::newRow("future-type-28") << raw("\x81\xfc") << 0 << CborErrorUnknownType << 1;
    QTest::newRow("future-type-29") << raw("\x81\xfd") << 0 << CborErrorUnknownType << 1;
    QTest::newRow("future-type-30") << raw("\x81\xfe") << 0 << CborErrorUnknownType << 1;
    QTest::newRow("unexpected-break") << raw("\x81\xff") << 0 << CborErrorUnexpectedBreak << 1;
    QTest::newRow("illegal-simple-0") << raw("\x81\xf8\0") << 0 << CborErrorIllegalSimpleType << 1;
    QTest::newRow("illegal-simple-31") << raw("\x81\xf8\x1f") << 0 << CborErrorIllegalSimpleType << 1;

    // not only too big (UINT_MAX or UINT_MAX+1 in size), but also incomplete
    if (sizeof(size_t) < sizeof(uint64_t)) {
        QTest::newRow("bytearray-too-big1") << raw("\x81\x5b\0\0\0\1\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;
        QTest::newRow("string-too-big1") << raw("\x81\x7b\0\0\0\1\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;
    }
    QTest::newRow("array-too-big1") << raw("\x81\x9a\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;
    QTest::newRow("array-too-big2") << raw("\x81\x9b\0\0\0\1\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;
    QTest::newRow("object-too-big1") << raw("\x81\xba\xff\xff\xff\xff\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;
    QTest::newRow("object-too-big2") << raw("\x81\xbb\0\0\0\1\0\0\0\0") << 0 << CborErrorDataTooLarge << 1;

    QTest::newRow("no-break-for-array0") << raw("\x81\x9f") << 0 << CborErrorUnexpectedEOF << 2;
    QTest::newRow("no-break-for-array1") << raw("\x81\x9f\x01") << 0 << CborErrorUnexpectedEOF << 3;
    QTest::newRow("no-break-string0") << raw("\x81\x7f") << 0 << CborErrorUnexpectedEOF << 1;
    QTest::newRow("no-break-string1") << raw("\x81\x7f\x61Z") << 0 << CborErrorUnexpectedEOF << 1;

    QTest::newRow("nested-indefinite-length-bytearrays") << raw("\x81\x5f\x5f\xff\xff") << 0 << CborErrorIllegalNumber << 1;
    QTest::newRow("nested-indefinite-length-strings") << raw("\x81\x5f\x5f\xff\xff") << 0 << CborErrorIllegalNumber << 1;

    QTest::newRow("string-chunk-unsigned") << raw("\x81\x7f\0\xff") << 0 << CborErrorIllegalType << 1;
    QTest::newRow("string-chunk-bytearray") << raw("\x81\x7f\x40\xff") << 0 << CborErrorIllegalType << 1;
    QTest::newRow("string-chunk-array") << raw("\x81\x7f\x80\xff") << 0 << CborErrorIllegalType << 1;
    QTest::newRow("bytearray-chunk-unsigned") << raw("\x81\x5f\0\xff") << 0 << CborErrorIllegalType << 1;
    QTest::newRow("bytearray-chunk-string") << raw("\x81\x5f\x60\xff") << 0 << CborErrorIllegalType << 1;
    QTest::newRow("bytearray-chunk-array") << raw("\x81\x5f\x80\xff") << 0 << CborErrorIllegalType << 1;
}

void tst_Parser::validation()
{
    QFETCH(QByteArray, data);
    QFETCH(int, flags);
    QFETCH(CborError, expectedError);
    QFETCH(int, offset);

    QString decoded;
    CborParser parser;
    CborValue first;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), flags, &parser, &first);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    err = parseOne(&first, &decoded);
    QCOMPARE(int(err), int(expectedError));
    QCOMPARE(int(first.ptr - reinterpret_cast<const quint8 *>(data.constBegin())), offset);
}

void tst_Parser::resumeParsing_data()
{
    addColumns();
    addFixedData();
    addStringsData();
    addTagsData();
    addMapMixedData();
}

void tst_Parser::resumeParsing()
{
    QFETCH(QByteArray, data);
    QFETCH(QString, expected);

    for (int len = 0; len < data.length() - 1; ++len) {
        CborParser parser;
        CborValue first;
        CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), len, 0, &parser, &first);
        if (!err) {
            QString decoded;
            err = parseOne(&first, &decoded);
        }
        if (err != CborErrorUnexpectedEOF)
            qDebug() << "Length is" << len;
        QCOMPARE(int(err), int(CborErrorUnexpectedEOF));
    }
}

void tst_Parser::endPointer_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("offset");

    QTest::newRow("number1") << raw("\x81\x01\x01") << 2;
    QTest::newRow("number24") << raw("\x81\x18\x18\x01") << 3;
    QTest::newRow("string") << raw("\x81\x61Z\x01") << 3;
    QTest::newRow("indefinite-string") << raw("\x81\x7f\x61Z\xff\x01") << 5;
    QTest::newRow("array") << raw("\x81\x02\x01") << 2;
    QTest::newRow("indefinite-array") << raw("\x81\x9f\x02\xff\x01") << 4;
    QTest::newRow("object") << raw("\x81\xa1\x03\x02\x01") << 4;
    QTest::newRow("indefinite-object") << raw("\x81\xbf\x03\x02\xff\x01") << 5;
}

void tst_Parser::endPointer()
{
    QFETCH(QByteArray, data);
    QFETCH(int, offset);

    QString decoded;
    CborParser parser;
    CborValue first;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &first);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    err = parseOne(&first, &decoded);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");
    QCOMPARE(int(first.ptr - reinterpret_cast<const quint8 *>(data.constBegin())), offset);
}

void tst_Parser::recursionLimit_data()
{
    static const int recursions = CBOR_PARSER_MAX_RECURSIONS + 2;
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("array") << QByteArray(recursions, '\x81') + '\x20';
    QTest::newRow("_array") << QByteArray(recursions, '\x9f') + '\x20' + QByteArray(recursions, '\xff');

    QByteArray data;
    for (int i = 0; i < recursions; ++i)
        data += "\xa1\x65Hello";
    data += '\2';
    QTest::newRow("map-recursive-values") << data;

    data.clear();
    for (int i = 0; i < recursions; ++i)
        data += "\xbf\x65World";
    data += '\2';
    for (int i = 0; i < recursions; ++i)
        data += "\xff";
    QTest::newRow("_map-recursive-values") << data;

    data = QByteArray(recursions, '\xa1');
    data += '\2';
    for (int i = 0; i < recursions; ++i)
        data += "\x7f\x64quux\xff";
    QTest::newRow("map-recursive-keys") << data;

    data = QByteArray(recursions, '\xbf');
    data += '\2';
    for (int i = 0; i < recursions; ++i)
        data += "\1\xff";
    QTest::newRow("_map-recursive-keys") << data;

    data.clear();
    for (int i = 0; i < recursions / 2; ++i)
        data += "\x81\xa1\1";
    data += '\2';
    QTest::newRow("mixed") << data;
}

void tst_Parser::recursionLimit()
{
    QFETCH(QByteArray, data);

    CborParser parser;
    CborValue first;
    CborError err = cbor_parser_init(reinterpret_cast<const quint8 *>(data.constData()), data.length(), 0, &parser, &first);
    QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");

    // check that it is valid:
    CborValue it = first;
    {
        QString dummy;
        err = parseOne(&it, &dummy);
        QVERIFY2(!err, QByteArray("Got error \"") + cbor_error_string(err) + "\"");
    }

    it = first;
    err = cbor_value_advance(&it);
    QCOMPARE(int(err), int(CborErrorNestingTooDeep));

    it = first;
    if (cbor_value_is_map(&it)) {
        CborValue dummy;
        err = cbor_value_map_find_value(&it, "foo", &dummy);
        QCOMPARE(int(err), int(CborErrorNestingTooDeep));
    }
}

QTEST_MAIN(tst_Parser)
#include "tst_parser.moc"
