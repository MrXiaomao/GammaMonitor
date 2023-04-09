#pragma once

#include <QLineEdit>
#include <QEvent>

// ר��������ַIP����Ŀؼ�
class QIPLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	QIPLineEdit(QWidget* parent = 0);
	~QIPLineEdit();

	void setText(const QString& strIP);
	QString text() const;
	bool isTextValid(const QString& strIP);

protected:
	void paintEvent(QPaintEvent* event);
	bool eventFilter(QObject* obj, QEvent* ev);

	int getIndex(QLineEdit* pEdit);
private:
	QLineEdit* m_lineEidt[4];
};