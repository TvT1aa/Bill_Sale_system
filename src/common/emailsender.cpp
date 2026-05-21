#include "emailsender.h"
#include <QDebug>
#include <QByteArray>
#include <QDateTime>

EmailSender::EmailSender(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_timeout(new QTimer(this))
    , m_smtpPort(465)
    , m_useSSL(true)
    , m_state(Idle)
{
    // 连接信号
    connect(m_socket, &QSslSocket::connected, this, &EmailSender::onConnected);
    connect(m_socket, &QSslSocket::readyRead, this, &EmailSender::onReadyRead);
    connect(m_socket, &QSslSocket::errorOccurred, this, &EmailSender::onErrorOccurred);

    // 超时处理
    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, &EmailSender::onTimeout);
}

EmailSender::~EmailSender()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void EmailSender::setServer(const QString &host, int port, bool useSSL)
{
    m_smtpHost = host;
    m_smtpPort = port;
    m_useSSL = useSSL;
}

void EmailSender::setCredentials(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
}

bool EmailSender::sendVerificationCode(const QString &to,
                                       const QString &code,
                                       const QString &account)
{
    if (m_smtpHost.isEmpty()) {
        m_lastError = "未配置 SMTP 服务器地址";
        emit emailSent(false, m_lastError);
        return false;
    }

    if (m_username.isEmpty() || m_password.isEmpty()) {
        m_lastError = "未配置邮箱账号或授权码";
        emit emailSent(false, m_lastError);
        return false;
    }

    m_toEmail = to;
    m_state = Connecting;
    m_responseBuffer.clear();
    m_lastError.clear();

    // 构建邮件主题和正文
    m_subject = QStringLiteral("密码找回 - 验证码");

    QString greeting = account.isEmpty() ? "尊敬的用户" : account;
    m_body = QStringLiteral(
        "%1，您好！\n\n"
        "您正在进行密码找回操作，您的验证码为：\n\n"
        "【%2】\n\n"
        "验证码有效期为 5 分钟，请尽快操作。\n"
        "如非本人操作，请忽略此邮件。\n\n"
        "（本邮件由系统自动发送，请勿回复）"
    ).arg(greeting, code);

    qDebug() << "[EmailSender] 开始发送邮件到:" << to;

    // 发起连接
    if (m_useSSL) {
        m_socket->connectToHostEncrypted(m_smtpHost, m_smtpPort);
    } else {
        m_socket->connectToHost(m_smtpHost, m_smtpPort);
    }

    m_timeout->start(TIMEOUT_MS);
    return true;
}

// ==================== 私有槽实现 ====================

void EmailSender::onConnected()
{
    qDebug() << "[EmailSender] 已连接到 SMTP 服务器";
    m_state = EhloSent;
    // 等待服务器发送欢迎消息（在 onReadyRead 中处理）
}

void EmailSender::onReadyRead()
{
    m_responseBuffer += m_socket->readAll();

    // SMTP 响应以 \\r\\n 结尾，多行响应最后一行以空格+数字开头
    // 简单判断：如果包含换行符，就认为收到了完整响应
    while (m_responseBuffer.contains("\r\n")) {
        int pos = m_responseBuffer.indexOf("\r\n");
        QString line = m_responseBuffer.left(pos);
        m_responseBuffer = m_responseBuffer.mid(pos + 2);

        qDebug() << "[EmailSender] 收到响应:" << line;

        // 获取响应码（前三位）
        if (line.length() < 3) continue;
        int respCode = line.left(3).toInt();

        // 如果是多行响应（第4位是 '-'），继续等待
        if (line.length() > 3 && line[3] == '-') continue;

        // 根据当前状态发送下一命令
        switch (m_state) {
        case Connecting:
            // 收到服务器的欢迎消息，发送 EHLO
            if (respCode == 220) {
                sendNextCommand();
            } else {
                m_lastError = QString("SMTP 连接被拒绝: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case EhloSent:
            if (respCode == 250) {
                m_state = AuthLoginSent;
                sendNextCommand();
            } else {
                m_lastError = QString("EHLO 失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case AuthLoginSent:
            if (respCode == 334) {
                m_state = AuthUserSent;
                sendNextCommand();
            } else {
                m_lastError = QString("AUTH LOGIN 失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case AuthUserSent:
            if (respCode == 334) {
                m_state = AuthPassSent;
                sendNextCommand();
            } else {
                m_lastError = QString("用户名认证失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case AuthPassSent:
            if (respCode == 235) {
                m_state = MailFromSent;
                sendNextCommand();
            } else {
                m_lastError = QString("密码/授权码认证失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case MailFromSent:
            if (respCode == 250) {
                m_state = RcptToSent;
                sendNextCommand();
            } else {
                m_lastError = QString("MAIL FROM 失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case RcptToSent:
            if (respCode == 250) {
                m_state = DataSent;
                sendNextCommand();
            } else {
                m_lastError = QString("RCPT TO 失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case DataSent:
            if (respCode == 354) {
                m_state = DataContentSent;
                sendNextCommand();
            } else {
                m_lastError = QString("DATA 命令失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        case DataContentSent:
            if (respCode == 250) {
                m_state = Done;
                qDebug() << "[EmailSender] 邮件发送成功!";
                m_timeout->stop();
                emit emailSent(true, "验证码邮件发送成功");
                cleanup();
            } else {
                m_lastError = QString("邮件内容发送失败: %1").arg(line);
                emit emailSent(false, m_lastError);
                cleanup();
            }
            break;

        default:
            break;
        }
    }
}

void EmailSender::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_lastError = m_socket->errorString();
    qDebug() << "[EmailSender] Socket 错误:" << m_lastError;
    m_timeout->stop();
    emit emailSent(false, QString("网络错误: %1").arg(m_lastError));
    cleanup();
}

void EmailSender::onTimeout()
{
    m_lastError = "操作超时（30秒）";
    qDebug() << "[EmailSender] 超时";
    emit emailSent(false, m_lastError);
    cleanup();
}

// ==================== 私有方法 ====================

void EmailSender::sendNextCommand()
{
    QByteArray cmd;

    switch (m_state) {
    case Connecting:
        // 等待服务器 220 欢迎消息，什么都不发送
        break;

    case EhloSent: {
        // 发送 EHLO
        QString domain = m_smtpHost.section('.', -2, -1);  // 提取域名后缀
        cmd = QString("EHLO %1\r\n").arg(domain).toUtf8();
        break;
    }

    case AuthLoginSent: {
        // 发送 AUTH LOGIN
        cmd = "AUTH LOGIN\r\n";
        break;
    }

    case AuthUserSent: {
        // 发送 Base64 编码的用户名
        cmd = m_username.toUtf8().toBase64() + "\r\n";
        break;
    }

    case AuthPassSent: {
        // 发送 Base64 编码的密码/授权码
        cmd = m_password.toUtf8().toBase64() + "\r\n";
        break;
    }

    case MailFromSent: {
        // MAIL FROM
        cmd = QString("MAIL FROM:<%1>\r\n").arg(m_username).toUtf8();
        break;
    }

    case RcptToSent: {
        // RCPT TO
        cmd = QString("RCPT TO:<%1>\r\n").arg(m_toEmail).toUtf8();
        break;
    }

    case DataSent: {
        // DATA
        cmd = "DATA\r\n";
        break;
    }

    case DataContentSent: {
        // 发送邮件内容
        QString mailContent = buildMailContent();
        cmd = mailContent.toUtf8();
        break;
    }

    default:
        break;
    }

    if (!cmd.isEmpty()) {
        qDebug() << "[EmailSender] 发送命令:" << cmd.left(64);
        m_socket->write(cmd);
        m_socket->flush();
    }
}

QString EmailSender::buildMailContent() const
{
    QString content;
    content += "From: \"" + m_username.section('@', 0, 0) + "\" <" + m_username + ">\r\n";
    content += "To: " + m_toEmail + "\r\n";
    content += "Subject: =?UTF-8?B?" + m_subject.toUtf8().toBase64() + "?=\r\n";
    content += "MIME-Version: 1.0\r\n";
    content += "Content-Type: text/plain; charset=UTF-8\r\n";
    content += "Content-Transfer-Encoding: base64\r\n";
    content += "Date: " + QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss +0800") + "\r\n";
    content += "\r\n";

    // 正文用 Base64 编码（支持中文）
    content += m_body.toUtf8().toBase64() + "\r\n";
    content += ".\r\n";

    return content;
}

void EmailSender::cleanup()
{
    m_state = Idle;
    m_timeout->stop();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void EmailSender::appendLog(const QString &log)
{
    qDebug() << "[EmailSender]" << log;
}
