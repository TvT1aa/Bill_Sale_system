#include "user.h"
#include "hashsha.h"
#include <QRegularExpression>
#include <utility>

// 直接在定义处初始化，避免权限问题和初始化时机问题
QRegularExpression User::s_usernameRegex("^[a-zA-Z0-9]+$");
QRegularExpression User::s_emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
QRegularExpression User::s_phoneRegex(R"(^1[3-9]\d{9}$)");

// 注意：如果你在 user.h 中保留了 initRegex() 的声明，建议去 .h 里把它删掉。
// 如果一定要保留声明，这里给一个空实现即可：
void User::initRegex() {}

// ===================== 构造、析构与拷贝控制 =====================

User::User()
    : m_id(-1)
    , m_isActive(true)
{
}

User::User(const QString& username, const QString& email,
           const QString& phone, const QString& password)
    : m_id(-1)
    , m_isActive(true)
{
    setUsername(username);
    setEmail(email);
    setPhone(phone);
    setPassword(password);
}

User::User(int id, const QString& username, const QString& email,
           const QString& phone, const QString& passwordHash,
           bool isActive, const QDateTime& createdAt,
           const QDateTime& lastLogin)
    : m_id(id)
    , m_username(username)
    , m_email(email)
    , m_phone(phone)
    , m_passwordHash(passwordHash)
    , m_isActive(isActive)
    , m_createdAt(createdAt)
    , m_lastLogin(lastLogin)
{
}

User::User(const User& other) = default;

User::User(User&& other) noexcept
    : m_id(std::exchange(other.m_id, -1))
    , m_username(std::move(other.m_username))
    , m_email(std::move(other.m_email))
    , m_phone(std::move(other.m_phone))
    , m_passwordHash(std::move(other.m_passwordHash))
    , m_isActive(other.m_isActive)
    , m_createdAt(std::move(other.m_createdAt))
    , m_lastLogin(std::move(other.m_lastLogin))
{
}

User::~User() = default;

User& User::operator=(const User& other)
{
    if (this != &other) {
        m_id = other.m_id;
        m_username = other.m_username;
        m_email = other.m_email;
        m_phone = other.m_phone;
        m_passwordHash = other.m_passwordHash;
        m_isActive = other.m_isActive;
        m_createdAt = other.m_createdAt;
        m_lastLogin = other.m_lastLogin;
    }
    return *this;
}

User& User::operator=(User&& other) noexcept
{
    if (this != &other) {
        m_id = std::exchange(other.m_id, -1);
        m_username = std::move(other.m_username);
        m_email = std::move(other.m_email);
        m_phone = std::move(other.m_phone);
        m_passwordHash = std::move(other.m_passwordHash);
        m_isActive = other.m_isActive;
        m_createdAt = std::move(other.m_createdAt);
        m_lastLogin = std::move(other.m_lastLogin);
    }
    return *this;
}

// ===================== Setter 逻辑 =====================

bool User::setUsername(const QString& username)
{
    QString clean = DESutil::encryptWithDefaultKey(username.trimmed());
    if (validateUsername(clean)) {
        m_username = clean;
        return true;
    }
    return false;
}

bool User::setEmail(const QString& email)
{
    QString clean = DESutil::encryptWithDefaultKey(email.trimmed());
    if (validateEmail(clean)) {
        m_email = clean;
        return true;
    }
    return false;
}

bool User::setPhone(const QString& phone)
{
    QString clean = phone.trimmed();
    if (validatePhone(clean, true)) {
        m_phone = clean;
        return true;
    }
    return false;
}

bool User::setPassword(const QString& plainPassword)
{
    if (checkPasswordStrength(plainPassword) >= PASSWORD_MEDIUM) {
        m_passwordHash = HashSha::hashSha256(plainPassword);
        return true;
    }
    return false;
}

// ===================== 校验逻辑 =====================

bool User::validateUsername(const QString& username, UserValidationError* errorCode)
{
    if (username.isEmpty()) {
        if (errorCode) *errorCode = ERR_USERNAME_EMPTY;
        return false;
    }
    if (username.length() < MIN_USERNAME_LEN || username.length() > MAX_USERNAME_LEN) {
        if (errorCode) *errorCode = ERR_USERNAME_LENGTH;
        return false;
    }
    if (!s_usernameRegex.match(username).hasMatch()) {
        if (errorCode) *errorCode = ERR_USERNAME_INVALID_CHAR;
        return false;
    }
    return true;
}

bool User::validateEmail(const QString& email, UserValidationError* errorCode)
{
    if (email.isEmpty()) {
        if (errorCode) *errorCode = ERR_EMAIL_EMPTY;
        return false;
    }
    if (!s_emailRegex.match(email).hasMatch()) {
        if (errorCode) *errorCode = ERR_EMAIL_INVALID;
        return false;
    }
    return true;
}

bool User::validatePhone(const QString& phone, bool allowEmpty, UserValidationError* errorCode)
{
    if (phone.isEmpty()) {
        if (allowEmpty) return true;
        if (errorCode) *errorCode = ERR_PHONE_INVALID;
        return false;
    }
    if (!s_phoneRegex.match(phone).hasMatch()) {
        if (errorCode) *errorCode = ERR_PHONE_INVALID;
        return false;
    }
    return true;
}

// ===================== 密码强度逻辑 =====================

PasswordStrength User::checkPasswordStrength(const QString& password)
{
    bool hu, hl, hd, hs;
    return checkPasswordStrengthDetailed(password, hu, hl, hd, hs);
}

PasswordStrength User::checkPasswordStrengthDetailed(const QString& password, bool& hasUpper, bool& hasLower, bool& hasDigit, bool& hasSpecial)
{
    hasUpper = hasLower = hasDigit = hasSpecial = false;
    if (password.isEmpty()) return PASSWORD_WEAK;

    for (const QChar& c : password) {
        if (c.isUpper()) hasUpper = true;
        else if (c.isLower()) hasLower = true;
        else if (c.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }

    int score = 0;
    if (password.length() >= MIN_PASSWORD_LEN) score++;
    if (hasUpper && hasLower) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;

    if (score >= 4) return PASSWORD_VERY_STRONG;
    if (score == 3) return PASSWORD_STRONG;
    if (score == 2) return PASSWORD_MEDIUM;
    return PASSWORD_WEAK;
}

bool User::verifyPassword(const QString& plainPassword) const
{
    if (m_passwordHash.isEmpty() || plainPassword.isEmpty()) return false;
    return m_passwordHash == HashSha::hashSha256(plainPassword);
}


QString User::getErrorMessage(UserValidationError errorCode)
{
    switch (errorCode) {
    case ERR_VALID_OK:               return QStringLiteral("验证通过");
    case ERR_USERNAME_EMPTY:         return QStringLiteral("用户名不能为空");
    case ERR_USERNAME_LENGTH:        return QStringLiteral("用户名长度应为3-20个字符");
    case ERR_USERNAME_INVALID_CHAR:  return QStringLiteral("用户名只能包含字母和数字");
    case ERR_EMAIL_EMPTY:            return QStringLiteral("邮箱不能为空");
    case ERR_EMAIL_INVALID:          return QStringLiteral("邮箱格式不正确");
    case ERR_PHONE_INVALID:          return QStringLiteral("手机号格式不正确");
    case ERR_PASSWORD_EMPTY:         return QStringLiteral("密码不能为空");
    case ERR_PASSWORD_WEAK:          return QStringLiteral("密码强度不足");
    default:                         return QStringLiteral("未知错误");
    }
}