#include "LoadingProtocol.h"


namespace Ui {
class LoadingProtocol;
}

class LoadingProtocol : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingProtocol(QWidget *parent = nullptr);
    ~LoadingProtocol();

private:
    Ui::LoadingProtocol *ui;
};
