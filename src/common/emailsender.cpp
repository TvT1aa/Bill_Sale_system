#include "emailsender.h"
#include <QDebug>
#include <QByteArray>
#include <QDateTime>

// 构造函数：初始化网络 Socket、定时器及默认状态
EmailSender::EmailSender(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_timeout(new QTimer(this))
    , m_smtpPort(465)
    , m_useSSL(true)
    , m_state(Idle)
{
    // 连接信号槽：监听连接、数据接收、错误发生
    connect(m_socket, &QSslSocket::connected, this, &EmailSender::onConnected);
    connect(m_socket, &QSslSocket::readyRead, this, &EmailSender::onReadyRead);
    connect(m_socket, &QSslSocket::errorOccurred, this, &EmailSender::onErrorOccurred);

    // 超时处理：如果 30 秒内流程没走完，则强制断开，防止阻塞
    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, &EmailSender::onTimeout);
}

// 析构函数：确保退出前断开网络连接
EmailSender::~EmailSender()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

// 设置 SMTP 服务器参数
void EmailSender::setServer(const QString &host, int port, bool useSSL)
{
    m_smtpHost = host;
    m_smtpPort = port;
    m_useSSL = useSSL;
}

// 设置发件人身份认证信息
void EmailSender::setCredentials(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;
}

// 公开接口：发起邮件发送流程
bool EmailSender::sendVerificationCode(const QString &to, const QString &code, const QString &account)
{
    // 基础配置检查
    if (m_smtpHost.isEmpty() || m_username.isEmpty() || m_password.isEmpty()) {
        emit emailSent(false, "Configuration missing");
        return false;
    }

    m_toEmail = to;
    m_state = Connecting; // 状态机起点
    m_responseBuffer.clear();

    // 构建邮件正文（支持中文）
    m_body = QStringLiteral("【%2】").arg(code);

    // 发起连接：根据是否使用SSL选择加密或明文连接
    if (m_useSSL) {
        m_socket->connectToHostEncrypted(m_smtpHost, m_smtpPort);
    } else {
        m_socket->connectToHost(m_smtpHost, m_smtpPort);
    }

    m_timeout->start(30000); // 启动超时监控
    return true;
}

// ==================== 私有槽实现：状态机驱动 ====================

// 仅仅表示网络物理连接已建立，等待服务器发送 220 欢迎语
void EmailSender::onConnected()
{
    qDebug() << "[EmailSender] 连接建立，等待服务器应答...";
}

// 核心逻辑：处理从服务器接收到的每一行数据
void EmailSender::onReadyRead()
{
    m_responseBuffer += m_socket->readAll();

    // 循环处理所有以 \r\n 结尾的完整响应行
    while (m_responseBuffer.contains("\r\n")) {
        int pos = m_responseBuffer.indexOf("\r\n");
        QString line = m_responseBuffer.left(pos);
        m_responseBuffer = m_responseBuffer.mid(pos + 2);

        qDebug() << "[SMTP 响应]:" << line;

        // 获取 SMTP 响应状态码（前三位数字）
        int respCode = line.left(3).toInt();
        if (line.length() > 3 && line[3] == '-') continue; // 多行响应处理

        // 基于当前 m_state 状态决定下一步发送什么
        switch (m_state) {
        case Connecting:
            if (respCode == 220) { // 收到服务器欢迎
                m_state = EhloSent;
                sendNextCommand(); // -> 发送 EHLO
            } else handleError("Connection refused");
            break;

        case EhloSent:
            if (respCode == 250) { // EHLO 成功
                m_state = AuthLoginSent;
                sendNextCommand(); // -> 发送 AUTH LOGIN
            } else handleError("EHLO 失败");
            break;

        case AuthLoginSent:
            if (respCode == 334) { // 准备输入用户名
                m_state = AuthUserSent;
                sendNextCommand(); // -> 发送 Base64 用户名
            } else handleError("AUTH LOGIN 失败");
            break;

        case AuthUserSent:
            if (respCode == 334) { // 准备输入密码
                m_state = AuthPassSent;
                sendNextCommand(); // -> 发送 Base64 密码
            } else handleError("Username authentication failed");
            break;

        case AuthPassSent:
            if (respCode == 235) { // 认证成功
                m_state = MailFromSent;
                sendNextCommand(); // -> 发送 MAIL FROM
            } else handleError("Incorrect password");
            break;

        case MailFromSent:
            if (respCode == 250) { // 发件人验证通过
                m_state = RcptToSent;
                sendNextCommand(); // -> 发送 RCPT TO (收件人)
            } else handleError("MAIL FROM 失败");
            break;

        case RcptToSent:
            if (respCode == 250) { // 收件人验证通过
                m_state = DataSent;
                sendNextCommand(); // -> 发送 DATA 命令
            } else handleError("RCPT TO 失败");
            break;

        case DataSent:
            if (respCode == 354) { // 服务器已准备好接收邮件内容
                m_state = DataContentSent;
                sendNextCommand(); // -> 发送真正的邮件内容
            } else handleError("DATA 准备失败");
            break;

        case DataContentSent:
            if (respCode == 250) { // 发送成功
                m_state = Done;
                emit emailSent(true, "Sent successfully");
                cleanup();
            } else handleError("Failed to send content");
            break;
        default: break;
        }
    }
}

// 统一发送指令的方法，根据 m_state 决定发送什么字符串
void EmailSender::sendNextCommand()
{
    QByteArray cmd;
    switch (m_state) {
        case EhloSent: cmd = "EHLO localhost\r\n"; break;
        case AuthLoginSent: cmd = "AUTH LOGIN\r\n"; break;
        case AuthUserSent: cmd = m_username.toUtf8().toBase64() + "\r\n"; break;
        case AuthPassSent: cmd = m_password.toUtf8().toBase64() + "\r\n"; break;
        case MailFromSent: cmd = "MAIL FROM:<" + m_username.toUtf8() + ">\r\n"; break;
        case RcptToSent: cmd = "RCPT TO:<" + m_toEmail.toUtf8() + ">\r\n"; break;
        case DataSent: cmd = "DATA\r\n"; break;
        case DataContentSent: cmd = buildMailContent().toUtf8(); break;
        default: break;
    }
    m_socket->write(cmd);
}

// 清理函数：发生错误或成功后复位状态
void EmailSender::cleanup()
{
    m_state = Idle;
    m_timeout->stop();
    m_socket->disconnectFromHost();
}
// 1. 补齐错误发生的槽函数
void EmailSender::onErrorOccurred(QAbstractSocket::SocketError error)
{
    qDebug() << "[EmailSender] Socket发生错误，错误码:" << error << m_socket->errorString();
    handleError(QString("网络错误: %1").arg(m_socket->errorString()));
}

// 2. 补齐超时处理的槽函数
void EmailSender::onTimeout()
{
    qDebug() << "[EmailSender] 发送超时，强制断开连接";
    handleError("Email sending timeout");
}

// 3. 补齐内部错误处理函数
void EmailSender::handleError(const QString &errorMsg)
{
    emit emailSent(false, errorMsg);
    cleanup();
}

// 4. 补齐构建邮件正文的函数（注意末尾的 const 必须严格对应头文件）
QString EmailSender::buildMailContent() const
{
    QString content;
    content += "From: " + m_username + "\r\n";
    content += "To: " + m_toEmail + "\r\n";
    content += "Subject: =?UTF-8?B?" + QString("Verification Code").toUtf8().toBase64() + "?=\r\n";
    content += "MIME-Version: 1.0\r\n";
    content += "Content-Type: text/plain; charset=\"UTF-8\"\r\n";
    content += "Content-Transfer-Encoding: 8bit\r\n";
    content += "\r\n"; // 邮件头和邮件体之间必须有一个空行
    content += m_body + "\r\n";
    content += ".\r\n"; // SMTP 协议规定，内容以单独的一行 "." 结束
    return content;
}