#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(int role = 0, QWidget *parent = nullptr);
    ~RegisterWidget();

signals:
    void backToLogin();
    void registerSubmitted(const QString &username,
                           const QString &email,
                           const QString &phone,
                           const QString &password,
                           const QString &confirmPassword,
                           int role);

public slots:
    void showRegisterError(const QString &message);
    void showRegisterSuccess(const QString &message);

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_btn_BackToLogin_clicked();
    void on_btn_RegisterSubmit_clicked();
    void on_btn_close_clicked();

private:
    Ui::RegisterWidget *ui;
    int m_role;
    QPoint m_dragPosition;
};

#endif // REGISTERWIDGET_H