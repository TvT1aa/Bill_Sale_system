#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    signals:
        void closed(); // 窗口关闭信号
        void requestVerificationCode(const QString &account);
        void resetPasswordRequested(const QString &account, const QString &code,
                                const QString &newPwd, const QString &confirmPwd);
    // 提交密码重置（账号、验证码、新密码、确认密码）

public slots:
    // 后端调用：显示错误 / 成功
    void showResetError(const QString &message);
    void showResetSuccess(const QString &message);

private slots:
    void on_btn_back_clicked();
    void on_btn_getCode_clicked();
    void on_btn_submit_clicked();

    void on_edit_account_textChanged(const QString &text);
    void on_edit_code_textChanged(const QString &text);
    void on_edit_newPwd_textChanged(const QString &text);
    void on_edit_confirmPwd_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H