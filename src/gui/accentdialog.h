// Choosing the project's colour.
//
// The house spectrum is one system every app draws from, and taking a point on
// it does not use it up. So this is a choice per project rather than per
// product, and the dialog shows what the house says each colour is for, because
// picking one is a decision about what the project's room is rather than a
// preference about which purple is nicest.
#pragma once

#include <QDialog>

class AccentDialog : public QDialog {
    Q_OBJECT
public:
    AccentDialog(const QString& projectName, const QColor& current, QWidget* parent = nullptr);

    QColor chosen() const { return chosen_; }

private:
    QColor chosen_;
};
