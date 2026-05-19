#ifndef ADMIN_VALIDATION_H
#define ADMIN_VALIDATION_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class AdminValidation; }
QT_END_NAMESPACE

class AdminValidation : public QWidget
{
    Q_OBJECT

public:
    explicit AdminValidation(QWidget *parent = nullptr);
    ~AdminValidation();

    // 注册管理员接口
    bool registerAdmin(const QString &username, const QString &password);

private slots:
    void onRegisterButtonClicked();

private:
    void updateStatus(const QString &message);
    void showError(const QString &message);
    void showSuccess(const QString &message);

private:
    Ui::AdminValidation *ui;
};

#endif // ADMIN_VALIDATION_H