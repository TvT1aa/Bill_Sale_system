#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QString>

class PasswordManager
{
public:
    // 修改密码（需要验证旧密码）
    // 参数：用户名、旧密码、新密码
    // 返回：true=成功，false=失败
    static bool changePassword(const QString& username,
                               const QString& oldPassword,
                               const QString& newPassword);

    // 忘记密码（通过安全问题验证）
    // 参数：用户名、安全答案、新密码
    // 返回：true=成功，false=失败
    static bool resetPasswordBySecurity(const QString& username,
                                        const QString& securityAnswer,
                                        const QString& newPassword);

    // 忘记密码（通过邮箱验证）
    // 参数：邮箱、验证码、新密码
    // 返回：true=成功，false=失败
    static bool resetPasswordByEmail(const QString& email,
                                     const QString& verifyCode,
                                     const QString& newPassword);

    // 管理员重置用户密码（无需旧密码）
    // 参数：管理员用户名、目标用户ID、新密码
    // 返回：true=成功，false=失败
    static bool adminResetPassword(const QString& adminUsername,
                                   int targetUserId,
                                   const QString& newPassword);

    // 验证安全答案
    // 参数：用户名、安全答案
    // 返回：true=验证通过，false=验证失败
    static bool verifySecurityAnswer(const QString& username,
                                     const QString& securityAnswer);

private:
    // 禁止实例化，所有方法都是静态的
    PasswordManager() = delete;
};

#endif // PASSWORDMANAGER_H