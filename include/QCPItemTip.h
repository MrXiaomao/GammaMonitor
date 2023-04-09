#pragma once
#include "qcustomplot.h"
/* including file 'src/items/item-text.h'  */
/* modified 2021-03-29T02:30:44, size 5576 */

class QCP_LIB_DECL QCPItemTip : public QCPAbstractItem
{
	Q_OBJECT
		/// \cond INCLUDE_QPROPERTIES
		Q_PROPERTY(QColor color READ color WRITE setColor)
		Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor)
		Q_PROPERTY(QPen pen READ pen WRITE setPen)
		Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
		Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
		Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
		Q_PROPERTY(QFont font READ font WRITE setFont)
		Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
		Q_PROPERTY(QString text READ text WRITE setText)
		Q_PROPERTY(Qt::Alignment positionAlignment READ positionAlignment WRITE setPositionAlignment)
		Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment)
		Q_PROPERTY(double rotation READ rotation WRITE setRotation)
		Q_PROPERTY(QMargins padding READ padding WRITE setPadding)
		/// \endcond
public:
	explicit QCPItemTip(QCustomPlot* parentPlot);
	virtual ~QCPItemTip() Q_DECL_OVERRIDE;

	struct ToolTipData {
		QColor _color;
		QString _text;
		QString _value;
		double _mouseY;
		bool _multiAxis;
	};

	// getters:
	QColor color() const { return mColor; }
	QColor selectedColor() const { return mSelectedColor; }
	QPen pen() const { return mPen; }
	QPen selectedPen() const { return mSelectedPen; }
	QBrush brush() const { return mBrush; }
	QBrush selectedBrush() const { return mSelectedBrush; }
	QFont font() const { return mFont; }
	QFont selectedFont() const { return mSelectedFont; }
	QString text() const { return mText; }
	Qt::Alignment positionAlignment() const { return mPositionAlignment; }
	Qt::Alignment textAlignment() const { return mTextAlignment; }
	double rotation() const { return mRotation; }
	QMargins padding() const { return mPadding; }

	// setters;
	void setRectColor(const QColor& color = QColor("#228b22")); //设置文本框颜色
	void setColor(const QColor& color);
	void setSelectedColor(const QColor& color);
	void setPen(const QPen& pen);
	void setSelectedPen(const QPen& pen);
	void setBrush(const QBrush& brush);
	void setSelectedBrush(const QBrush& brush);
	void setFont(const QFont& font);
	void setSelectedFont(const QFont& font);
	void setData(const QVector<QCPItemTip::ToolTipData>& tData);
	void setText(const QString& text);
	void setPositionAlignment(Qt::Alignment alignment);
	void setTextAlignment(Qt::Alignment alignment);
	void setRotation(double degrees);
	void setPadding(const QMargins& padding);

	// reimplemented virtual methods:
	virtual double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const Q_DECL_OVERRIDE;

	QCPItemPosition* const position;
	QCPItemAnchor* const topLeft;
	QCPItemAnchor* const top;
	QCPItemAnchor* const topRight;
	QCPItemAnchor* const right;
	QCPItemAnchor* const bottomRight;
	QCPItemAnchor* const bottom;
	QCPItemAnchor* const bottomLeft;
	QCPItemAnchor* const left;

protected:
	enum AnchorIndex { aiTopLeft, aiTop, aiTopRight, aiRight, aiBottomRight, aiBottom, aiBottomLeft, aiLeft };

	// property members:
	QColor mColor, mSelectedColor,mRectColor;  //mRectColor文本框颜色
	QPen mPen, mSelectedPen;
	QBrush mBrush, mSelectedBrush;
	QFont mFont, mSelectedFont;
	QString mText;
	Qt::Alignment mPositionAlignment;
	Qt::Alignment mTextAlignment;
	double mRotation;
	QMargins mPadding;

	QVector<ToolTipData> mtData;
	// reimplemented virtual methods:
	virtual void draw(QCPPainter* painter) Q_DECL_OVERRIDE;
	virtual QPointF anchorPixelPosition(int anchorId) const Q_DECL_OVERRIDE;

	// non-virtual methods:
	QPointF getTextDrawPoint(const QPointF& pos, const QRectF& rect, Qt::Alignment positionAlignment) const;
	QFont mainFont() const;
	QColor mainColor() const;
	QPen mainPen() const;
	QBrush mainBrush() const;
};

/* end of 'src/items/item-text.h' */