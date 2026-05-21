#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QString>
#include <QSslSocket>
#include <QTimer>

/**
 * @class EmailSender
 * @brief 邮件发送类 - 通过 SMTP 协议发送验证码邮件
 *
 * 使用示例：
 * @code
 * EmailSender *sender = new EmailSender(this);
 * sender->setServer("smtp.qq.com", 465, true);
 * sender->setCredentials("your@qq.com", "授权码");
 * sender->sendVerificationCode("user@example.com", "123456");
 * @endcode
 */
class EmailSender : public QObject
{
    Q_OBJECT

public:
    explicit EmailSender(QObject *parent = nullptr);
    ~EmailSender();

    /// 配置 SMTP 服务器
    void setServer(const QString &host, int port, bool useSSL = true);

    /// 配置账号密码（需填写邮箱的 SMTP 授权码）
    void setCredentials(const QString &username, const QString &password);

    /**
     * @brief 发送验证码邮件
     * @param to       收件人邮箱
     * @param code     验证码
     * @param account  用户账号（用于在邮件中称呼）
     * @return 是否成功发起连接
     */
    bool sendVerificationCode(const QString &to,
                              const QString &code,
                              const QString &account = QString());

    /// 获取最后错误信息
    QString lastError() const { return m_lastError; }

signals:
    /// 邮件发送结果 signal
    void emailSent(bool success, const QString &message);

private slots:
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onTimeout();

private:
    // SMTP 协议状态机
    enum SmtpState {
        Idle,
        Connecting,
        EhloSent,
        AuthLoginSent,
        AuthUserSent,
        AuthPassSent,
        MailFromSent,
        RcptToSent,
        DataSent,
        DataContentSent,
        Done
    };

    void sendNextCommand();
    QString buildMailContent() const;
    void cleanup();
    void appendLog(const QString &log);

    QSslSocket *m_socket;
    QTimer     *m_timeout;

    // SMTP 服务器配置
    QString m_smtpHost;
    int     m_smtpPort;
    bool    m_useSSL;

    // 认证信息
    QString m_username;
    QString m_password;

    // 邮件内容
    QString m_toEmail;
    QString m_subject;
    QString m_body;

    // 状态
    SmtpState m_state;
    QString   m_responseBuffer;
    QString   m_lastError;

    static constexpr int TIMEOUT_MS = 30000;   // 30秒超时
};

#endif // EMAILSENDER_H
