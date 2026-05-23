#ifndef DESUTIL_H
#define DESUTIL_H

#include <QString>
#include <vector>

typedef uint64_t uint64;
typedef uint32_t uint32;

class DESutil {
public:
    DESutil();
    // 任务 1.1 & 1.2: 核心加解密接口
    static QString encrypt(const QString& plainText, const QString& key);
    static QString decrypt(const QString& cipherText, const QString& key);
    // 任务 1.4: 验证输入
    static bool validateInput(const QString& input);
    static QString encryptWithDefaultKey(const QString& plainText);
    static QString decryptWithDefaultKey(const QString& plainText);

private:
    static std::vector<uint64> generateSubkeys(uint64 key);
    static uint32 roundFunction(uint32 rpt, uint64 subkey);
    static uint64 initialPermutation(uint64 input);
    static uint64 finalPermutation(uint64 input);
    static uint64 stringTo64Bit(const QString& str);
    static QString sixtyFourBitToString(uint64 val);
    //====DES加密密钥====//
    static const QString s_desKey;
    static QByteArray u64ToBytes(uint64 val);
    static uint64 bytesTo64(const QByteArray &ba);
};

#endif