#ifndef _PROPERTYSHEET_H_
#define _PROPERTYSHEET_H_

#include <qtablewidget.h>
#include <qmap.h>

class PlusMinusWg;

typedef QMap<QString, PlusMinusWg*> CaptionMap;
typedef QMap<QString, int> RowMap;

class PlusMinusWg : public QWidget {
	Q_OBJECT
public:
	PlusMinusWg(const QString & name, QWidget *parent = 0):
	QWidget(parent),
	m_Name(name),
	m_Closed(false) {}
	virtual ~PlusMinusWg(){}

	void setBgColor(const QColor & color);
	QString name() { return m_Name; }

	void open();
	void close();
    void clean();
	bool isClosed() { return m_Closed; }

signals:
	void closed(bool);

protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
	void mousePressEvent(QMouseEvent *);

private:
	QColor m_BgColor;
    QRect m_PMRect;
	QString m_Name;
	bool m_Closed;
};
//-----------------------------------------------------------------------
class QssSheet : public QTableWidget {
	Q_OBJECT
public:
    QssSheet(QWidget *parent = 0);
    ~QssSheet(){}

    void addCaption(const QString & name, const QString & hint, QColor &cap_color, bool closed = true);
	void addPropertyWidget(const QString & name, QWidget *widget);
	QStringList captionList() { return m_Captions.keys(); }
    QString getText(int row, int column) const;
    QString getObj(int row, int column);

    QColor m_CaptionColor;

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:

private slots:
	void changeCaptionState(bool);
    void insertNewRow();

private:
	int addRow();
	RowMap m_Rows;
	CaptionMap m_Captions;
    QAction *addNewRow;
	QString m_LastAddedCaption;
	int m_RowHeight;
};

#endif
